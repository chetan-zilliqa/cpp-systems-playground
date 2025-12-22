#include <iostream>
#include "common/logging.hpp"
#include "kv_store_chaining/kv_store_chaining.hpp"

using kv_store::InMemorykvstore_chaining;

int main() {
    common::set_log_level(common::LogLevel::Debug);

    LOG_INFO("Starting InMemorykvstore_chaining demo");

    InMemorykvstore_chaining store(16, 32);

    std::cout << "========================================\n";
    std::cout << "  InMemorykvstore_chaining (pool-backed) demo\n";
    std::cout << "========================================\n";

    std::cout << "Putting some keys...\n";
    store.put("user:1", "Alice");
    store.put("user:2", "Bob");
    store.put("session:abc", "active");

    auto print_get = [&](const std::string& key) {
        auto v = store.get(key);
        if (v) std::cout << "GET " << key << " -> " << *v << "\n";
        else   std::cout << "GET " << key << " -> <null>\n";
    };

    print_get("user:1");
    print_get("user:2");
    print_get("user:3");
    print_get("session:abc");

    std::cout << "\nErasing user:2...\n";
    store.erase("user:2");
    print_get("user:2");

    std::cout << "Size = " << store.size() << "\n";

    LOG_INFO("KV store demo finished");
    return 0;
}

