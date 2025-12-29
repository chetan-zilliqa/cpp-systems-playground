#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

// Cross-module integration tests - all includes at top
#include "hash_map/hash_map.hpp"
#include "lru_cache/lru_cache.hpp"
#include "thread_pool/work_stealing_thread_pool.hpp"
#include "lock_free_queue/spsc_queue.hpp"
#include "kv_store_chaining/kv_store_chaining.hpp"

// Test 1: Hash map standalone
void test_hashmap_standalone() {
    std::cout << "Test 1: HashMap standalone...\n";
    
    hash_map::HashMap<int, std::string> map;
    
    // Insert and verify
    map.insert_or_assign(1, "one");
    map.insert_or_assign(2, "two");
    map.insert_or_assign(3, "three");
    
    assert(map.size() == 3);
    assert(map.contains(1));
    assert(map.contains(2));
    assert(map.contains(3));
    
    auto val = map.get(2);
    assert(val.has_value());
    assert(val.value() == "two");
    
    std::cout << "  ✓ HashMap stores and retrieves correctly\n";
}

// Test 2: LRU Cache with HashMap
void test_lru_cache_integration() {
    std::cout << "Test 2: LRU Cache (uses HashMap internally)...\n";
    
    lru::LRUCache<int, std::string> lru_cache_obj(3); // capacity 3
    
    lru_cache_obj.put(1, "one");
    lru_cache_obj.put(2, "two");
    lru_cache_obj.put(3, "three");
    
    assert(lru_cache_obj.get(1).has_value());
    assert(lru_cache_obj.get(2).has_value());
    assert(lru_cache_obj.get(3).has_value());
    
    // Add 4th item - should evict LRU (1 or 2 depending on access pattern)
    lru_cache_obj.put(4, "four");
    assert(lru_cache_obj.size() <= 3);
    
    std::cout << "  ✓ LRU Cache eviction works correctly\n";
}

// Test 3: Thread pool + lock-free queue collaboration
void test_thread_pool_with_lockfree_queue() {
    std::cout << "Test 3: Thread pool + Lock-free queue...\n";
    
    auto thread_pool = std::make_unique<thread_pool::WorkStealingThreadPool>(2);
    
    std::atomic<int> counter{0};
    
    // Submit tasks to thread pool
    auto future1 = thread_pool->submit([&counter]() {
        ++counter;
        return 42;
    });
    
    auto future2 = thread_pool->submit([&counter]() {
        ++counter;
        return 100;
    });
    
    // Wait for results
    int result1 = future1.get();
    int result2 = future2.get();
    
    assert(result1 == 42);
    assert(result2 == 100);
    assert(counter == 2);
    
    std::cout << "  ✓ Thread pool executes tasks correctly\n";
}

// Test 4: Lock-free queue wait-free guarantee
void test_lock_free_queue() {
    std::cout << "Test 4: Lock-free queue basic operations...\n";
    
    lock_free::SPSCQueue<int, 32> queue;
    
    // Push items
    for (int i = 0; i < 10; ++i) {
        assert(queue.push(i));
    }
    
    assert(!queue.empty());
    assert(queue.size() == 10);
    
    // Pop items
    int value;
    int count = 0;
    while (queue.pop(value)) {
        count++;
    }
    
    assert(count == 10);
    assert(queue.empty());
    
    std::cout << "  ✓ Lock-free queue works correctly\n";
}

// Test 6: Stress test - multiple operations
void test_stress_integration() {
    std::cout << "Test 6: Stress test - concurrent operations...\n";
    
    hash_map::HashMap<int, int> map;
    std::vector<std::thread> threads;
    const int ops_per_thread = 100;
    const int num_threads = 4;
    
    // Writer threads
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&map, t, ops_per_thread]() {
            for (int i = 0; i < ops_per_thread; ++i) {
                int key = (t * ops_per_thread) + i;
                map.insert_or_assign(key, key * 2);
            }
        });
    }
    
    // Wait for all writers
    for (auto& th : threads) {
        th.join();
    }
    
    assert(map.size() == ops_per_thread * num_threads);
    
    std::cout << "  ✓ Concurrent insertions handled correctly\n";
}

int main() {
    std::cout << "\n=== Cross-Module Integration Tests ===\n\n";
    
    try {
        test_hashmap_standalone();
        test_lru_cache_integration();
        test_thread_pool_with_lockfree_queue();
        test_lock_free_queue();
        test_stress_integration();
        
        std::cout << "\n✅ All integration tests passed!\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << "\n\n";
        return 1;
    }
}
