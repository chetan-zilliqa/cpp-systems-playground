#pragma once

#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hash_map {

template <typename Key,
          typename T,
          typename Hash     = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class HashMap {
public:
    using key_type      = Key;
    using mapped_type   = T;
    using size_type     = std::size_t;
    using hasher        = Hash;
    using key_equal     = KeyEqual;

private:
    struct Node {
        Key key;
        T   value;
    };

    using Bucket  = std::list<Node>;
    using Buckets = std::vector<Bucket>;

public:
    explicit HashMap(size_type bucket_count = kDefaultBucketCount,
                     const Hash& hash       = Hash{},
                     const KeyEqual& eq     = KeyEqual{})
        : buckets_(bucket_count ? bucket_count : kDefaultBucketCount),
          size_(0),
          hash_(hash),
          eq_(eq),
          max_load_factor_(0.75)
    {}

    HashMap(const HashMap&)            = delete;
    HashMap& operator=(const HashMap&) = delete;

    HashMap(HashMap&& other) noexcept
        : buckets_(std::move(other.buckets_)),
          size_(other.size_),
          hash_(std::move(other.hash_)),
          eq_(std::move(other.eq_)),
          max_load_factor_(other.max_load_factor_)
    {
        other.size_ = 0;
    }

    HashMap& operator=(HashMap&& other) noexcept
    {
        if (this == &other) return *this;
        buckets_         = std::move(other.buckets_);
        size_            = other.size_;
        hash_            = std::move(other.hash_);
        eq_              = std::move(other.eq_);
        max_load_factor_ = other.max_load_factor_;
        other.size_      = 0;
        return *this;
    }

    ~HashMap() = default;

    // --- capacity ---
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] size_type size() const noexcept { return size_; }
    [[nodiscard]] size_type bucket_count() const noexcept { return buckets_.size(); }

    [[nodiscard]] double load_factor() const noexcept
    {
        return bucket_count() == 0
            ? 0.0
            : static_cast<double>(size_) / static_cast<double>(bucket_count());
    }

    [[nodiscard]] double max_load_factor() const noexcept { return max_load_factor_; }

    void max_load_factor(double lf)
    {
        if (lf <= 0.0) {
            throw std::invalid_argument("max_load_factor must be > 0");
        }
        max_load_factor_ = lf;
        maybe_rehash();
    }

    void reserve(size_type new_capacity)
    {
        const auto required_buckets =
            static_cast<size_type>(static_cast<float>(new_capacity) / max_load_factor_) + 1;
        if (required_buckets > bucket_count()) {
            rehash(required_buckets);
        }
    }

    // --- modifiers ---

    // Returns true if inserted, false if updated.
    bool insert_or_assign(const Key& key, const T& value)
    {
        return do_insert_or_assign(key, value);
    }

    bool insert_or_assign(Key&& key, T&& value)
    {
        return do_insert_or_assign(std::move(key), std::move(value));
    }

    bool erase(const Key& key)
    {
        const auto idx = bucket_index(key);
        auto& bucket   = buckets_[idx];

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (eq_(it->key, key)) {
                bucket.erase(it);
                --size_;
                return true;
            }
        }
        return false;
    }

    void clear()
    {
        for (auto& b : buckets_) {
            b.clear();
        }
        size_ = 0;
    }

    // --- lookup ---

    [[nodiscard]] bool contains(const Key& key) const
    {
        const auto idx = bucket_index(key);
        const auto& bucket = buckets_[idx];

        for (const auto& node : bucket) {
            if (eq_(node.key, key)) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] size_type count(const Key& key) const
    {
        return contains(key) ? 1 : 0;
    }

    // returns a *copy* of the stored value (fine for iterator types as well)
    [[nodiscard]] std::optional<T> get(const Key& key) const
    {
        const auto idx = bucket_index(key);
        const auto& bucket = buckets_[idx];

        for (const auto& node : bucket) {
            if (eq_(node.key, key)) {
                return node.value;
            }
        }
        return std::nullopt;
    }

    // Returns reference to value; throws if key not found
    T& at(const Key& key)
    {
        const auto idx = bucket_index(key);
        auto& bucket = buckets_[idx];

        for (auto& node : bucket) {
            if (eq_(node.key, key)) {
                return node.value;
            }
        }
        throw std::out_of_range("Key not found in HashMap");
    }

    const T& at(const Key& key) const
    {
        const auto idx = bucket_index(key);
        const auto& bucket = buckets_[idx];

        for (const auto& node : bucket) {
            if (eq_(node.key, key)) {
                return node.value;
            }
        }
        throw std::out_of_range("Key not found in HashMap");
    }

    // Returns reference to value; inserts default value if key not found
    T& operator[](const Key& key)
    {
        const auto idx = bucket_index(key);
        auto& bucket = buckets_[idx];

        for (auto& node : bucket) {
            if (eq_(node.key, key)) {
                return node.value;
            }
        }

        bucket.push_front(Node{key, T{}});
        ++size_;
        maybe_rehash();
        return bucket.front().value;
    }

private:
    template <typename K, typename V>
    bool do_insert_or_assign(K&& key, V&& value)
    {
        const auto idx = bucket_index(key);
        auto& bucket   = buckets_[idx];

        for (auto& node : bucket) {
            if (eq_(node.key, key)) {
                node.value = std::forward<V>(value);
                return false; // updated
            }
        }

        bucket.push_front(Node{std::forward<K>(key), std::forward<V>(value)});
        ++size_;
        maybe_rehash();
        return true; // inserted
    }

    template <typename K>
    [[nodiscard]] size_type bucket_index(const K& key) const
    {
        return hash_(key) % buckets_.size();
    }

    void maybe_rehash()
    {
        if (load_factor() > max_load_factor_) {
            const auto new_bucket_count = bucket_count() * 2;
            rehash(new_bucket_count ? new_bucket_count : kDefaultBucketCount);
        }
    }

    void rehash(size_type new_bucket_count)
    {
        if (new_bucket_count < 1) new_bucket_count = 1;

        Buckets new_buckets(new_bucket_count);

        for (auto& bucket : buckets_) {
            for (auto& node : bucket) {
                const auto idx = hash_(node.key) % new_bucket_count;
                new_buckets[idx].push_front(Node{
                    std::move(node.key),
                    std::move(node.value)
                });
            }
        }

        buckets_.swap(new_buckets);
        // size_ unchanged
    }

    static constexpr size_type kDefaultBucketCount = 16;

    Buckets   buckets_;
    size_type size_;
    Hash      hash_;
    KeyEqual  eq_;
    double    max_load_factor_;
};

} // namespace hash_map

