#include "hash_map/hash_map.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <utility>

using hash_map::HashMap;

static void test_basic_insert_get()
{
    HashMap<std::string, int> m;

    assert(m.empty());
    assert(m.size() == 0);

    m.insert_or_assign("a", 1);
    m.insert_or_assign("b", 2);
    m.insert_or_assign("c", 3);

    assert(!m.empty());
    assert(m.size() == 3);

    auto va = m.get("a");
    auto vb = m.get("b");
    auto vc = m.get("c");
    auto vd = m.get("missing");

    assert(va && *va == 1);
    assert(vb && *vb == 2);
    assert(vc && *vc == 3);
    assert(!vd.has_value());
}

static void test_update_existing()
{
    HashMap<std::string, int> m;

    bool inserted = m.insert_or_assign("key", 10);
    assert(inserted);

    auto v1 = m.get("key");
    assert(v1 && *v1 == 10);

    bool inserted2 = m.insert_or_assign("key", 42);
    assert(!inserted2); // updated, not inserted

    auto v2 = m.get("key");
    assert(v2 && *v2 == 42);
}

static void test_erase_and_clear()
{
    HashMap<int, std::string> m;

    m.insert_or_assign(1, "one");
    m.insert_or_assign(2, "two");
    m.insert_or_assign(3, "three");

    assert(m.size() == 3);
    assert(m.contains(2));

    bool erased = m.erase(2);
    assert(erased);
    assert(!m.contains(2));
    assert(m.size() == 2);

    // Erasing non-existent key should return false
    bool erased_again = m.erase(2);
    assert(!erased_again);

    m.clear();
    assert(m.size() == 0);
    assert(!m.contains(1));
    assert(!m.contains(3));
}

static void test_reserve_and_rehash()
{
    HashMap<int, int> m;

    // Insert some values
    for (int i = 0; i < 20; ++i) {
        m.insert_or_assign(i, i * 10);
    }

    auto old_bucket_count = m.bucket_count();
    m.reserve(200); // should increase bucket_count

    auto new_bucket_count = m.bucket_count();
    assert(new_bucket_count >= old_bucket_count);

    // Make sure all values still exist after rehash
    for (int i = 0; i < 20; ++i) {
        auto v = m.get(i);
        assert(v && *v == i * 10);
    }
}

static void test_move_constructor_and_assignment()
{
    HashMap<std::string, int> m1;
    m1.insert_or_assign("x", 1);
    m1.insert_or_assign("y", 2);

    // Move construct
    HashMap<std::string, int> m2(std::move(m1));
    assert(m2.size() == 2);
    assert(m2.get("x").value_or(0) == 1);
    assert(m2.get("y").value_or(0) == 2);

    // m1 should be logically empty now
    assert(m1.size() == 0);

    // Move assign
    HashMap<std::string, int> m3;
    m3 = std::move(m2);

    assert(m3.size() == 2);
    assert(m3.get("x").value_or(0) == 1);
    assert(m3.get("y").value_or(0) == 2);
}

int main()
{
    std::cout << "Running HashMap tests...\n";

    test_basic_insert_get();
    test_update_existing();
    test_erase_and_clear();
    test_reserve_and_rehash();
    test_move_constructor_and_assignment();

    std::cout << "All HashMap tests passed.\n";
    return 0;
}

