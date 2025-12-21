#pragma once

#include <cstring>
#include <string_view>
#include <optional>
#include <cstdint>

namespace kv_opt {

constexpr size_t MAX_KEY = 64;
constexpr size_t MAX_VALUE = 256;

class KVStoreOptimized {
public:
    explicit KVStoreOptimized(size_t capacity)
        : cap(capacity), size_(0)
    {
        entries = new Entry[cap];
        std::memset(entries, 0, sizeof(Entry) * cap);
    }

    ~KVStoreOptimized()
    {
        delete[] entries;
    }

    bool insert(std::string_view key, std::string_view value)
    {
        if (key.size() > MAX_KEY || value.size() > MAX_VALUE)
            return false;

        uint64_t h = hash(key);
        size_t idx = h % cap;

        for (size_t i = 0; i < cap; ++i) {
            size_t slot = (idx + i) % cap;

            if (!entries[slot].occupied) {
                write_entry(entries[slot], h, key, value);
                size_++;
                return true;
            }

            if (entries[slot].hash == h &&
                std::strcmp(entries[slot].key, key.data()) == 0)
            {
                write_entry(entries[slot], h, key, value);
                return true;
            }
        }
        return false; // full
    }

    std::optional<std::string_view> get(std::string_view key) const
    {
        uint64_t h = hash(key);
        size_t idx = h % cap;

        for (size_t i = 0; i < cap; ++i) {
            size_t slot = (idx + i) % cap;

            if (!entries[slot].occupied)
                return std::nullopt;

            if (entries[slot].hash == h &&
                std::strcmp(entries[slot].key, key.data()) == 0)
            {
                return std::string_view(entries[slot].value);
            }
        }
        return std::nullopt;
    }

    bool erase(std::string_view key)
    {
        uint64_t h = hash(key);
        size_t idx = h % cap;

        for (size_t i = 0; i < cap; ++i) {
            size_t slot = (idx + i) % cap;

            if (!entries[slot].occupied)
                return false;

            if (entries[slot].hash == h &&
                std::strcmp(entries[slot].key, key.data()) == 0)
            {
                entries[slot].occupied = false;
                size_--;
                return true;
            }
        }
        return false;
    }

    size_t size() const { return size_; }

private:
    struct Entry {
        uint64_t hash;
        char key[MAX_KEY + 1];
        char value[MAX_VALUE + 1];
        bool occupied = false;
    };

    size_t cap;
    size_t size_;
    Entry* entries;

    static uint64_t hash(std::string_view s)
    {
        uint64_t h = 2166136261ull;
        for (char c : s)
            h = (h ^ c) * 16777619;
        return h;
    }

    static void write_entry(Entry& e,
                            uint64_t h,
                            std::string_view key,
                            std::string_view value)
    {
        e.hash = h;

        std::memcpy(e.key, key.data(), key.size());
        e.key[key.size()] = '\0';

        std::memcpy(e.value, value.data(), value.size());
        e.value[value.size()] = '\0';

        e.occupied = true;
    }
};

} // namespace kv_opt

