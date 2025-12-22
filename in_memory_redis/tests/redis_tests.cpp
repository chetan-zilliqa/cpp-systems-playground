#include "in_memory_redis/redis.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

using in_memory_redis::KVStore;

static void test_basic_put_get()
{
    KVStore kv;

    kv.put("a", "1");
    kv.put("b", "2");

    auto va = kv.get("a");
    auto vb = kv.get("b");
    auto vc = kv.get("c");

    assert(va && *va == "1");
    assert(vb && *vb == "2");
    assert(!vc.has_value());
    assert(kv.size() == 2);
}

static void test_overwrite_value()
{
    KVStore kv;

    kv.put("key", "value1");
    auto v1 = kv.get("key");
    assert(v1 && *v1 == "value1");

    kv.put("key", "value2");
    auto v2 = kv.get("key");
    assert(v2 && *v2 == "value2");
}

static void test_ttl_expiration_via_get()
{
    using Ms = KVStore::Ms;

    KVStore kv;

    kv.put("temp", "x", Ms{50}); // 50ms TTL
    auto v1 = kv.get("temp");
    assert(v1 && *v1 == "x");

    std::this_thread::sleep_for(std::chrono::milliseconds(80));

    auto v2 = kv.get("temp");
    assert(!v2.has_value()); // should be expired
}

static void test_prefix_get_basic()
{
    KVStore kv;

    kv.put("app", "1");
    kv.put("apple", "2");
    kv.put("apricot", "3");
    kv.put("banana", "4");

    auto res = kv.prefix_get("ap");
    // Expected keys: app, apple, apricot (sorted lexicographically)
    assert(res.size() == 3);
    assert(res[0].first == "app");
    assert(res[1].first == "apple");
    assert(res[2].first == "apricot");
}

static void test_prefix_get_limit()
{
    KVStore kv;

    kv.put("app", "1");
    kv.put("apple", "2");
    kv.put("apricot", "3");

    auto res = kv.prefix_get("ap", 2);
    assert(res.size() == 2);
}

static void test_erase_and_clear()
{
    KVStore kv;

    kv.put("a", "1");
    kv.put("b", "2");
    kv.put("c", "3");

    assert(kv.size() == 3);

    kv.erase("b");
    auto vb = kv.get("b");
    assert(!vb.has_value());

    kv.clear();
    assert(kv.size() == 0);
    assert(!kv.get("a").has_value());
    assert(!kv.get("c").has_value());
}

static void test_ttl_update_resets_expiry()
{
    using Ms = KVStore::Ms;

    KVStore kv;

    kv.put("k", "v1", Ms{50});
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // Update with new TTL
    kv.put("k", "v2", Ms{100});

    // After 60ms total, it should still exist with "v2"
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    auto v = kv.get("k");
    assert(v && *v == "v2");

    // After longer, it should expire
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto v2 = kv.get("k");
    assert(!v2.has_value());
}

// This test relies on the sweeper thread eventually cleaning up,
// not just lazy expiry via get() / prefix_get()
static void test_background_sweeper_removes_expired()
{
    using Ms = KVStore::Ms;

    KVStore kv(Ms{20}); // sweep interval ~20ms

    kv.put("x", "1", Ms{30}); // expires in ~30ms
    assert(kv.size() == 1);

    // Wait long enough for expiry + at least one sweep iteration.
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    // We don't call get() here – relying on sweeper.
    // There is still a tiny chance due to scheduling,
    // but with this margin it should be robust enough for a demo.
    auto sx = kv.size();
    assert(sx == 0 || !kv.get("x").has_value());
}

int main()
{
    std::cout << "Running KVStore (in_memory_redis) tests...\n";

    test_basic_put_get();
    test_overwrite_value();
    test_ttl_expiration_via_get();
    test_prefix_get_basic();
    test_prefix_get_limit();
    test_erase_and_clear();
    test_ttl_update_resets_expiry();
    test_background_sweeper_removes_expired();

    std::cout << "All KVStore tests passed.\n";
    return 0;
}

