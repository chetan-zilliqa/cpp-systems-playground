#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>
#include <condition_variable>

namespace in_memory_redis {

class KVStore {
public:
    using Clock = std::chrono::steady_clock;
    using Ms    = std::chrono::milliseconds;

    explicit KVStore(Ms sweep_interval = Ms{200});
    KVStore(const KVStore&) = delete;
    KVStore& operator=(const KVStore&) = delete;
    KVStore(KVStore&&) = delete;
    KVStore& operator=(KVStore&&) = delete;

    ~KVStore();

    // Insert or update. ttl<=0ms => no expiration.
    void put(const std::string& key, std::string value, Ms ttl = Ms{0});

    // Get current value (if present and not expired).
    [[nodiscard]]
    std::optional<std::string> get(const std::string& key);

    // Remove a key (idempotent).
    void erase(const std::string& key);

    // Prefix search; returns up to `limit` (0 = unlimited).
    // Results are sorted by key.
    std::vector<std::pair<std::string, std::string>>
    prefix_get(const std::string& prefix, std::size_t limit = 0);

    // Number of entries currently in the map (including ones that
    // might be expired but not yet swept).
    std::size_t size() const;

    // Clear all keys and TTL heap.
    void clear();

private:
    using TimePoint = Clock::time_point;

    struct Entry {
        std::string value;
        TimePoint   expires{};   // undefined if !hasExpiry
        uint64_t    version{0};  // increments each put
        bool        hasExpiry{false};
    };

    struct Node {
        TimePoint   expires;
        uint64_t    version;
        std::string key;

        bool operator>(const Node& rhs) const {
            if (expires != rhs.expires)
                return expires > rhs.expires;
            return version > rhs.version;
        }
    };

    using MinHeap = std::priority_queue<Node, std::vector<Node>, std::greater<Node>>;

    // --- internal helpers ---
    static bool isExpired(const Entry& e, TimePoint now);
    static std::string nextPrefix(const std::string& p);

    void eraseIfExpired(const std::string& key, TimePoint now);
    void sweepLoop();

    // --- data ---
    mutable std::shared_mutex mu_;
    std::map<std::string, Entry> store_;

    std::mutex              heap_mu_;
    MinHeap                 heap_;
    Ms                      sweep_interval_;
    std::thread             sweeper_;
    std::condition_variable_any cv_;

    std::atomic<bool>      stop_;
    std::atomic<uint64_t>  version_counter_;
};

} // namespace in_memory_redis

