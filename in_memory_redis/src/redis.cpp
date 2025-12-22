#include "in_memory_redis/redis.hpp"

namespace in_memory_redis {

KVStore::KVStore(Ms sweep_interval)
    : sweep_interval_(sweep_interval),
      stop_(false),
      version_counter_(0)
{
    sweeper_ = std::thread([this] { this->sweepLoop(); });
}

KVStore::~KVStore()
{
    stop_.store(true, std::memory_order_relaxed);
    cv_.notify_one();
    if (sweeper_.joinable())
        sweeper_.join();
}

void KVStore::put(const std::string& key, std::string value, Ms ttl)
{
    const auto now     = Clock::now();
    const bool has_ttl = ttl.count() > 0;
    const auto exp     = has_ttl ? now + ttl : TimePoint{};
    const auto ver     = version_counter_.fetch_add(1, std::memory_order_relaxed) + 1;

    {
        std::unique_lock<std::shared_mutex> lk(mu_);
        auto& e     = store_[key];
        e.value     = std::move(value);
        e.expires   = exp;
        e.version   = ver;
        e.hasExpiry = has_ttl;
    }
    if (has_ttl) {
        std::lock_guard<std::mutex> lk(heap_mu_);
        heap_.push(Node{exp, ver, key});
        cv_.notify_one();
    }
}

std::optional<std::string> KVStore::get(const std::string& key)
{
    const auto now = Clock::now();
    std::shared_lock<std::shared_mutex> lk(mu_);
    auto it = store_.find(key);
    if (it == store_.end())
        return std::nullopt;
    if (isExpired(it->second, now)) {
        // Lazy erase: release read lock, then do conditional erase under write lock.
        lk.unlock();
        eraseIfExpired(key, now);
        return std::nullopt;
    }
    return it->second.value;
}

void KVStore::erase(const std::string& key)
{
    std::unique_lock<std::shared_mutex> lk(mu_);
    store_.erase(key);
}

std::vector<std::pair<std::string, std::string>>
KVStore::prefix_get(const std::string& prefix, std::size_t limit)
{
    const auto now = Clock::now();
    std::vector<std::pair<std::string, std::string>> out;

    std::shared_lock<std::shared_mutex> lk(mu_);
    auto it       = store_.lower_bound(prefix);
    const auto hi = nextPrefix(prefix);

    for (; it != store_.end() && (hi.empty() || it->first < hi); ++it) {
        if (isExpired(it->second, now))
            continue; // skip; lazy cleanup later
        out.emplace_back(it->first, it->second.value);
        if (limit && out.size() >= limit)
            break;
    }
    return out;
}

std::size_t KVStore::size() const
{
    std::shared_lock<std::shared_mutex> lk(mu_);
    return store_.size();
}

void KVStore::clear()
{
    {
        std::unique_lock<std::shared_mutex> lk(mu_);
        store_.clear();
    }
    {
        std::lock_guard<std::mutex> lk(heap_mu_);
        heap_ = MinHeap{};
    }
}

// --- static helpers ---

bool KVStore::isExpired(const Entry& e, TimePoint now)
{
    return e.hasExpiry && now >= e.expires;
}

std::string KVStore::nextPrefix(const std::string& p)
{
    if (p.empty())
        return {};
    std::string hi = p;
    for (int i = static_cast<int>(hi.size()) - 1; i >= 0; --i) {
        auto ch = static_cast<unsigned char>(hi[i]);
        if (ch != 0xFF) {
            hi[i] = static_cast<char>(ch + 1);
            hi.resize(static_cast<std::size_t>(i) + 1);
            return hi;
        }
    }
    return {}; // no strict upper bound; iterate to end
}

void KVStore::eraseIfExpired(const std::string& key, TimePoint now)
{
    std::unique_lock<std::shared_mutex> lk(mu_);
    auto it = store_.find(key);
    if (it != store_.end() && isExpired(it->second, now)) {
        store_.erase(it);
    }
}

void KVStore::sweepLoop()
{
    std::unique_lock<std::mutex> lk(heap_mu_);
    for (;;) {
        if (stop_.load(std::memory_order_relaxed))
            break;

        TimePoint nextWake{};
        if (!heap_.empty()) {
            auto n   = heap_.top();
            nextWake = n.expires;
            const auto now = Clock::now();
            if (now >= n.expires) {
                heap_.pop();
                // Validate version matches live entry before erase.
                lk.unlock();
                {
                    std::unique_lock<std::shared_mutex> wlk(mu_);
                    auto it = store_.find(n.key);
                    if (it != store_.end() &&
                        it->second.hasExpiry &&
                        it->second.expires <= now &&
                        it->second.version == n.version)
                    {
                        store_.erase(it);
                    }
                }
                lk.lock();
                continue;
            }
        }

        if (stop_.load(std::memory_order_relaxed))
            break;

        if (heap_.empty()) {
            cv_.wait_for(lk, sweep_interval_);
        } else {
            cv_.wait_until(lk, nextWake);
        }
    }
}

} // namespace in_memory_redis

