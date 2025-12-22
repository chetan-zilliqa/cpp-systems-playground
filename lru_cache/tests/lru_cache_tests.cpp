#include "lru_cache/lru_cache.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

using lru::LRUCache;

static void test_basic_get_put()
{
    LRUCache<int, int> cache(2);

    cache.put(1, 10);
    cache.put(2, 20);

    auto v1 = cache.get(1);
    auto v2 = cache.get(2);

    assert(v1.has_value());
    assert(v2.has_value());
    assert(*v1 == 10);
    assert(*v2 == 20);
    assert(cache.size() == 2);
}

static void test_eviction_order()
{
    LRUCache<int, int> cache(2);

    cache.put(1, 10); // [1]
    cache.put(2, 20); // [2,1] (2 MRU)

    // Access 1 -> makes 1 MRU, 2 LRU
    auto v1 = cache.get(1);
    assert(v1 && *v1 == 10);

    cache.put(3, 30); // evict key 2

    auto m2 = cache.get(2);
    auto g1 = cache.get(1);
    auto g3 = cache.get(3);

    assert(!m2.has_value());
    assert(g1 && *g1 == 10);
    assert(g3 && *g3 == 30);
    assert(cache.size() == 2);
}

static void test_update_existing()
{
    LRUCache<int, int> cache(2);

    cache.put(1, 10);
    cache.put(2, 20);

    cache.put(1, 100); // update + move to MRU

    auto v1 = cache.get(1);
    auto v2 = cache.get(2);

    assert(v1 && *v1 == 100);
    assert(v2 && *v2 == 20);

    // Access 2 so it becomes MRU, 1 becomes LRU
    (void)cache.get(2);

    cache.put(3, 30); // evicts 1

    auto m1 = cache.get(1);
    auto g2 = cache.get(2);
    auto g3 = cache.get(3);

    assert(!m1.has_value());
    assert(g2 && *g2 == 20);
    assert(g3 && *g3 == 30);
}

static void test_erase_and_clear()
{
    LRUCache<std::string, int> cache(3);

    cache.put("a", 1);
    cache.put("b", 2);
    cache.put("c", 3);

    assert(cache.size() == 3);
    assert(cache.erase("b") == true);
    assert(cache.size() == 2);
    assert(!cache.get("b").has_value());

    // Erasing non-existent key should return false
    assert(cache.erase("b") == false);

    cache.clear();
    assert(cache.size() == 0);
    assert(!cache.get("a").has_value());
    assert(!cache.get("c").has_value());
}

static void test_contains()
{
    LRUCache<int, int> cache(2);

    cache.put(1, 10);
    assert(cache.contains(1));
    assert(!cache.contains(2));

    cache.put(2, 20);
    assert(cache.contains(2));

    cache.erase(1);
    assert(!cache.contains(1));
}

static void test_capacity_zero_throws()
{
    bool threw = false;
    try {
        LRUCache<int, int> cache(0);
        (void)cache;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "LRUCache(capacity=0) must throw std::invalid_argument");
}

int main()
{
    std::cout << "Running LRUCache tests...\n";

    test_basic_get_put();
    test_eviction_order();
    test_update_existing();
    test_erase_and_clear();
    test_contains();
    test_capacity_zero_throws();

    std::cout << "All LRUCache tests passed.\n";
    return 0;
}

