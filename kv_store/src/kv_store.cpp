#include "kv_store/kv_store.hpp"
#include <new>
#include <stdexcept>
#include <mutex>

namespace kv_store {

InMemoryKVStore::InMemoryKVStore(std::size_t num_buckets,
                                 std::size_t max_items)
    : buckets_(num_buckets, nullptr)
    , mutex_()
    , pool_(sizeof(Node), max_items)
    , size_(0)
{
    if (num_buckets == 0) {
        throw std::invalid_argument("num_buckets must be > 0");
    }

    LOG_INFO("InMemoryKVStore created with " +
             std::to_string(num_buckets) + " buckets, capacity " +
             std::to_string(max_items) + " items");
}

std::size_t InMemoryKVStore::bucket_index(const Key& key) const {
    std::size_t h = std::hash<Key>{}(key);
    return h % buckets_.size();
}

void InMemoryKVStore::put(const Key& key, const Value& value) {
    std::unique_lock lock(mutex_);

    const std::size_t idx = bucket_index(key);
    Node* cur = buckets_[idx];

    for (Node* n = cur; n != nullptr; n = n->next) {
        if (n->key == key) {
            LOG_DEBUG("Updating existing key: " + key);
            n->value = value;
            return;
        }
    }

    LOG_DEBUG("Inserting new key: " + key);
    void* raw = pool_.allocate();
    Node* node = new (raw) Node{key, value, buckets_[idx]};
    buckets_[idx] = node;
    ++size_;
}

std::optional<InMemoryKVStore::Value>
InMemoryKVStore::get(const Key& key) const {
    std::shared_lock lock(mutex_);

    const std::size_t idx = bucket_index(key);
    for (Node* n = buckets_[idx]; n != nullptr; n = n->next) {
        if (n->key == key) {
            LOG_DEBUG("Hit key: " + key);
            return n->value;
        }
    }
    LOG_DEBUG("Miss key: " + key);
    return std::nullopt;
}

bool InMemoryKVStore::erase(const Key& key) {
    std::unique_lock lock(mutex_);

    const std::size_t idx = bucket_index(key);
    Node* cur = buckets_[idx];
    Node* prev = nullptr;

    while (cur) {
        if (cur->key == key) {
            LOG_DEBUG("Erasing key: " + key);
            if (prev) prev->next = cur->next;
            else      buckets_[idx] = cur->next;

            cur->~Node();
            pool_.deallocate(cur);
            --size_;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    LOG_DEBUG("Tried to erase missing key: " + key);
    return false;
}

bool InMemoryKVStore::contains(const Key& key) const {
    return static_cast<bool>(get(key));
}

std::size_t InMemoryKVStore::size() const {
    std::shared_lock lock(mutex_);
    return size_;
}

void InMemoryKVStore::clear_nodes() {
    std::unique_lock lock(mutex_);

    for (Node*& head : buckets_) {
        Node* cur = head;
        while (cur) {
            Node* next = cur->next;
            cur->~Node();
            pool_.deallocate(cur);
            cur = next;
        }
        head = nullptr;
    }
    size_ = 0;
    LOG_INFO("Cleared all nodes from KV store");
}

} // namespace kv_store

