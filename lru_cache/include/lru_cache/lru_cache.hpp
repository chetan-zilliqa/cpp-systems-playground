#pragma once

#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <utility>

#include "hash_map/hash_map.hpp"

namespace lru {

template <typename Key,
          typename Value,
          typename Hash     = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class LRUCache {
public:
    using key_type    = Key;
    using mapped_type = Value;
    using size_type   = std::size_t;

private:
    using List   = std::list<std::pair<Key, Value>>;
    using ListIt = typename List::iterator;

    // internal index: key -> iterator into the list
    using Map = hash_map::HashMap<Key, ListIt, Hash, KeyEqual>;

public:
    explicit LRUCache(size_type capacity)
        : capacity_(capacity)
    {
        if (capacity_ == 0) {
            throw std::invalid_argument("LRUCache capacity must be > 0");
        }
    }

    LRUCache(const LRUCache&)            = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    LRUCache(LRUCache&& other) noexcept
        : capacity_(other.capacity_),
          list_(std::move(other.list_)),
          map_(std::move(other.map_))
    {}

    LRUCache& operator=(LRUCache&& other) noexcept
    {
        if (this == &other) return *this;
        capacity_ = other.capacity_;
        list_     = std::move(other.list_);
        map_      = std::move(other.map_);
        return *this;
    }

    ~LRUCache() = default;

    [[nodiscard]] size_type capacity() const noexcept { return capacity_; }
    [[nodiscard]] size_type size() const noexcept     { return list_.size(); }
    [[nodiscard]] bool empty() const noexcept         { return list_.empty(); }

    void clear()
    {
        list_.clear();
        map_.clear();
    }

    [[nodiscard]] bool contains(const Key& key) const
    {
        return map_.contains(key);
    }

    // Returns value if present; refreshes LRU order.
    [[nodiscard]] std::optional<Value> get(const Key& key)
    {
        auto itOpt = map_.get(key);
        if (!itOpt) {
            return std::nullopt;
        }

        auto nodeIt = *itOpt; // copy ListIt
        touch(nodeIt);
        return nodeIt->second;
    }

    // Insert/update; updates MRU ordering.
    void put(const Key& key, const Value& value)
    {
        do_put(key, value);
    }

    void put(Key&& key, Value&& value)
    {
        do_put(std::move(key), std::move(value));
    }

    // Erase returns true if key existed.
    bool erase(const Key& key)
    {
        auto itOpt = map_.get(key);
        if (!itOpt) return false;

        auto nodeIt = *itOpt;
        list_.erase(nodeIt);
        map_.erase(key);
        return true;
    }

private:
    template <typename K, typename V>
    void do_put(K&& key, V&& value)
    {
        // Check if key already exists
        auto itOpt = map_.get(key);
        if (itOpt) {
            auto nodeIt   = *itOpt;
            nodeIt->second = std::forward<V>(value);
            touch(nodeIt);
            return;
        }

        // Insert new
        if (size() == capacity_) {
            evict_one();
        }

        list_.emplace_front(std::forward<K>(key), std::forward<V>(value));
        ListIt nodeIt = list_.begin();
        map_.insert_or_assign(nodeIt->first, nodeIt);
    }

    void touch(ListIt it)
    {
        if (it == list_.begin()) return;
        list_.splice(list_.begin(), list_, it); // move to front (MRU)
    }

    void evict_one()
    {
        if (list_.empty()) return;

        auto lruIt = std::prev(list_.end());
        const Key& k = lruIt->first;
        map_.erase(k);
        list_.pop_back();
    }

    size_type capacity_;
    List      list_; // front = MRU, back = LRU
    Map       map_;
};

} // namespace lru

