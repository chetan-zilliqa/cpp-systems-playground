#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include "kvstore_optimized.hpp"

int main() {
    using namespace std::chrono;

    const std::size_t capacity = 100000;  // buckets
    const std::size_t num_entries = 50000;

    kv_opt::KVStoreOptimized store(capacity);

    std::cout << "KVStoreOptimized demo\n";
    std::cout << "Capacity: " << capacity
              << ", inserting: " << num_entries << " entries\n\n";

    // --- 1. Single insert with very large key/value ------------------------

    std::string big_key(kv_opt::MAX_KEY, 'K');       // key = "KKKKK..."
    std::string big_value(kv_opt::MAX_VALUE, 'V');   // value = "VVVV..."

    if (!store.insert(big_key, big_value)) {
        std::cerr << "Failed to insert big key/value (too large?)\n";
    } else {
        auto v = store.get(big_key);
        std::cout << "Big entry insert/get: "
                  << (v.has_value() ? "OK" : "FAILED") << "\n";
    }

    // --- 2. Bulk insert with moderately large values -----------------------

    auto start_insert = steady_clock::now();

    for (std::size_t i = 0; i < num_entries; ++i) {
        std::string key = "key_" + std::to_string(i);

        // ensure we don't exceed MAX_KEY
        if (key.size() > kv_opt::MAX_KEY) {
            key.resize(kv_opt::MAX_KEY);
        }

        // build a relatively large value near MAX_VALUE
        std::string prefix = "value_" + std::to_string(i) + "_";
        std::size_t remaining =
            (kv_opt::MAX_VALUE > prefix.size())
                ? (kv_opt::MAX_VALUE - prefix.size())
                : 0;

        std::string value = prefix + std::string(remaining, 'X');

        bool ok = store.insert(key, value);
        if (!ok) {
            std::cerr << "Insert failed at i=" << i
                      << " (table full or value too big)\n";
            break;
        }
    }

    auto end_insert = steady_clock::now();
    auto insert_ms = duration_cast<milliseconds>(end_insert - start_insert).count();

    std::cout << "\nBulk insert done in " << insert_ms << " ms\n";
    std::cout << "Store size (approx): " << store.size() << "\n";

    // --- 3. Random reads ---------------------------------------------------

    std::cout << "\nVerifying a few sample lookups...\n";
    bool all_ok = true;

    for (int i = 0; i < 5; ++i) {
        std::size_t idx = (i * 9973) % num_entries;  // pseudo-random
        std::string key = "key_" + std::to_string(idx);
        if (key.size() > kv_opt::MAX_KEY) {
            key.resize(kv_opt::MAX_KEY);
        }

        std::string prefix = "value_" + std::to_string(idx) + "_";
        std::size_t remaining =
            (kv_opt::MAX_VALUE > prefix.size())
                ? (kv_opt::MAX_VALUE - prefix.size())
                : 0;
        std::string expected = prefix + std::string(remaining, 'X');

        auto v = store.get(key);
        if (!v.has_value()) {
            std::cout << "  [FAIL] key \"" << key << "\" not found\n";
            all_ok = false;
        } else {
            if (v.value() == expected) {
                std::cout << "  [OK] key \"" << key << "\" verified\n";
            } else {
                std::cout << "  [FAIL] key \"" << key
                          << "\" has unexpected value\n";
                all_ok = false;
            }
        }
    }

    // --- 4. Overwrite + erase demo ----------------------------------------

    std::cout << "\nTesting overwrite + erase...\n";

    std::string ow_key = "special_key";
    std::string val1 = "first_value";
    std::string val2 = "second_value_large_" + std::string(32, 'Z');

    store.insert(ow_key, val1);
    store.insert(ow_key, val2);  // overwrite

    auto after_overwrite = store.get(ow_key);
    std::cout << "  After overwrite: "
              << (after_overwrite.has_value() ? after_overwrite.value() : "<none>")
              << "\n";

    bool erased = store.erase(ow_key);
    std::cout << "  Erase result: " << (erased ? "true" : "false") << "\n";

    auto after_erase = store.get(ow_key);
    std::cout << "  After erase, found? "
              << (after_erase.has_value() ? "yes" : "no") << "\n";

    std::cout << "\nAll sample checks "
              << (all_ok ? "PASSED" : "FAILED (see above)") << "\n";

    return all_ok ? 0 : 1;
}
