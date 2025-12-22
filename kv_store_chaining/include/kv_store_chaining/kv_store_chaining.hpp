#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <shared_mutex>

#include "common/logging.hpp"
#include "memory_pool/fixed_block_memory_pool.hpp"

namespace kv_store {

class InMemorykvstore_chaining {
public:
    using Key   = std::string;
    using Value = std::string;

    explicit InMemorykvstore_chaining(std::size_t num_buckets = 16,
                             std::size_t max_items   = 64);

    InMemorykvstore_chaining(const InMemorykvstore_chaining&) = delete;
    InMemorykvstore_chaining& operator=(const InMemorykvstore_chaining&) = delete;

    void put(const Key& key, const Value& value);
    std::optional<Value> get(const Key& key) const;
    bool erase(const Key& key);
    bool contains(const Key& key) const;
    std::size_t size() const;

private:
    struct Node {
        Key key;
        Value value;
        Node* next;
    };

    std::size_t bucket_index(const Key& key) const;
    void clear_nodes();

    std::vector<Node*> buckets_;
    mutable std::shared_mutex mutex_;
    memory_pool::FixedBlockMemoryPool pool_;
    std::size_t size_ = 0;
};

} // namespace kv_store

