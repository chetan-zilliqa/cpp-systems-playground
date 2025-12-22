#pragma once

#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace lru {

template <typename Key,
          typename Value,
          typename Hash    = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class LRUCache {
public:
    using key_type        = Key;
    using mapped_type     = Value;
    using value_type      = std::pair<Key, Value>;
    using size_type       = std::size_t;

private:
    using List      = std::list<value_type>;
    using ListIt    = typename List::iterator;
    using Map       = std::unordered_map<Key, ListIt, Hash, KeyEqual>;

public:
    explicit LRUCache(size_type capacity)
        : capacity_(capacity)
    {
        if (capacity_ == 0) {
            throw std::invalid_argument("LRUCache capacity must be > 0");
        }
    }

    // Non-copyable, movable
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    LRUCache(LRUCache&& other) noexcept
        : capacity_(other.capacity_),
          list_(std::move(other.list_)),
          map_(std::move(other.map_))
    {
        // map_ iterators remain valid because they point into list_
        // which we've moved as a whole.
    }

    LRUCache& operator=(LRUCache&& other) noexcept
    {
        if (this == &other) return *this;

        capacity_ = other.capacity_;
        list_     = std::move(other.list_);
        map_      = std::move(other.map_);
        return *this;
    }

    ~LRUCache() = default;

    size_type capacity() const noexcept { return capacity_; }
    size_type size() const noexcept     { return map_.size(); }
    bool empty() const noexcept         { return map_.empty(); }

    void clear()
    {
        list_.clear();
        map_.clear();
    }

    bool contains(const Key& key) const
    {
        return map_.find(key) != map_.end();
    }

    // Returns value if present, std::nullopt on miss.
    std::optional<Value> get(const Key& key)
    {
        auto it = map_.find(key);
        if (it == map_.end()) {
            return std::nullopt;
        }

        // Move node to front (most recently used).
        touch(it->second);
        return it->second->second;
    }

    // Insert or update.
    // On existing key: update value + move to front.
    // On new key: evict LRU when at capacity.
    void put(const Key& key, Value value)
    {
        auto it = map_.find(key);
        if (it != map_.end()) {
            // Update existing and move to front
            it->second->second = std::move(value);
            touch(it->second);
            return;
        }

        if (map_.size() == capacity_) {
            evictOne();
        }

        list_.emplace_front(key, std::move(value));
        map_[key] = list_.begin();
    }

    // Erase returns true if key was present.
    bool erase(const Key& key)
    {
        auto it = map_.find(key);
        if (it == map_.end()) return false;

        list_.erase(it->second);
        map_.erase(it);
        return true;
    }

private:
    void touch(ListIt it)
    {
        // Move the element to the front (MRU).
        if (it == list_.begin()) return;
        list_.splice(list_.begin(), list_, it);
    }

    void evictOne()
    {
        // LRU is at the back.
        auto it = list_.end();
        --it;
        const Key& k = it->first;
        map_.erase(k);
        list_.pop_back();
    }

    size_type capacity_;
    List      list_; // front = MRU, back = LRU
    Map       map_;
};

} // namespace lru

