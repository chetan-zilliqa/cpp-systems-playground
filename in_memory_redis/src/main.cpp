#include <chrono>
#include <iostream>
#include <thread>

#include "in_memory_redis/redis.hpp"

using in_memory_redis::KVStore;

int main()
{
    KVStore kv;

    kv.put("apple",   "red",    KVStore::Ms{500}); // TTL 500ms
    kv.put("app",     "prefix", KVStore::Ms{0});   // no TTL
    kv.put("banana",  "yellow", KVStore::Ms{0});
    kv.put("apricot", "orange", KVStore::Ms{0});

    // Prefix search
    auto v = kv.prefix_get("ap"); // app, apple, apricot
    std::cout << "prefix 'ap':\n";
    for (auto& [k, val] : v) {
        std::cout << "  " << k << " -> " << val << "\n";
    }

    // Wait for apple to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    std::cout << "get apple after ttl: "
              << (kv.get("apple").has_value() ? "present" : "expired") << "\n";

    // Still returns app/apricot
    auto v2 = kv.prefix_get("ap");
    std::cout << "prefix 'ap' after ttl:\n";
    for (auto& [k, val] : v2) {
        std::cout << "  " << k << " -> " << val << "\n";
    }

    return 0;
}

