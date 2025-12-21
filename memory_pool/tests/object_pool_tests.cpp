#include <gtest/gtest.h>
#include "memory_pool/object_pool.hpp"

using memory_pool::ObjectPool;

struct TestObject {
    inline static int live_count = 0;

    int id;
    std::string payload;

    TestObject(int i = 0, std::string p = {})
        : id(i), payload(std::move(p))
    {
        ++live_count;
    }

    ~TestObject() {
        --live_count;
    }

    static void reset_counters() {
        live_count = 0;
    }
};

TEST(ObjectPool, BasicAllocateDeallocate)
{
    TestObject::reset_counters();

    constexpr std::size_t N = 4;
    ObjectPool<TestObject, N> pool;

    EXPECT_EQ(pool.capacity(), N);
    EXPECT_EQ(pool.free_slots(), N);
    EXPECT_EQ(TestObject::live_count, 0);

    auto* obj1 = pool.allocate(1, "one");
    auto* obj2 = pool.allocate(2, "two");

    EXPECT_EQ(TestObject::live_count, 2);
    EXPECT_EQ(pool.free_slots(), N - 2);

    EXPECT_EQ(obj1->id, 1);
    EXPECT_EQ(obj1->payload, "one");
    EXPECT_EQ(obj2->id, 2);
    EXPECT_EQ(obj2->payload, "two");

    pool.deallocate(obj1);
    pool.deallocate(obj2);

    EXPECT_EQ(TestObject::live_count, 0);
    EXPECT_EQ(pool.free_slots(), N);
}

TEST(ObjectPool, MakeUniqueRAII)
{
    TestObject::reset_counters();

    constexpr std::size_t N = 2;
    ObjectPool<TestObject, N> pool;

    EXPECT_EQ(pool.free_slots(), N);
    EXPECT_EQ(TestObject::live_count, 0);

    {
        auto p1 = pool.make_unique(10, "ten");
        auto p2 = pool.make_unique(20, "twenty");

        EXPECT_EQ(TestObject::live_count, 2);
        EXPECT_EQ(pool.free_slots(), 0);

        EXPECT_EQ(p1->id, 10);
        EXPECT_EQ(p1->payload, "ten");
        EXPECT_EQ(p2->id, 20);
        EXPECT_EQ(p2->payload, "twenty");
    }

    // All unique_ptrs out of scope -> objects destroyed and returned to pool
    EXPECT_EQ(TestObject::live_count, 0);
    EXPECT_EQ(pool.free_slots(), N);
}

TEST(ObjectPool, ExhaustionThrows)
{
    TestObject::reset_counters();

    constexpr std::size_t N = 2;
    ObjectPool<TestObject, N> pool;

    auto p1 = pool.make_unique(1, "one");
    auto p2 = pool.make_unique(2, "two");

    EXPECT_EQ(TestObject::live_count, 2);
    EXPECT_EQ(pool.free_slots(), 0);

    EXPECT_THROW(
        {
            auto p3 = pool.make_unique(3, "three");
            (void)p3;
        },
        std::bad_alloc
    );

    // Still only 2 live
    EXPECT_EQ(TestObject::live_count, 2);
}

TEST(ObjectPool, ReuseAfterRAII)
{
    TestObject::reset_counters();

    constexpr std::size_t N = 1;
    ObjectPool<TestObject, N> pool;

    {
        auto p = pool.make_unique(42, "first");
        EXPECT_EQ(TestObject::live_count, 1);
        EXPECT_EQ(pool.free_slots(), 0);
        EXPECT_EQ(p->id, 42);
    }

    // p destroyed, slot returned
    EXPECT_EQ(TestObject::live_count, 0);
    EXPECT_EQ(pool.free_slots(), 1);

    {
        auto p2 = pool.make_unique(77, "second");
        EXPECT_EQ(TestObject::live_count, 1);
        EXPECT_EQ(pool.free_slots(), 0);
        EXPECT_EQ(p2->id, 77);
    }

    EXPECT_EQ(TestObject::live_count, 0);
    EXPECT_EQ(pool.free_slots(), 1);
}

TEST(ObjectPool, NullptrDeallocateIsNoop)
{
    TestObject::reset_counters();

    constexpr std::size_t N = 2;
    ObjectPool<TestObject, N> pool;

    EXPECT_EQ(pool.free_slots(), N);
    EXPECT_EQ(TestObject::live_count, 0);

    // Should not crash or change state
    pool.deallocate(nullptr);

    EXPECT_EQ(pool.free_slots(), N);
    EXPECT_EQ(TestObject::live_count, 0);
}

TEST(ObjectPool, ConstructorArgumentsForwarded)
{
    TestObject::reset_counters();

    constexpr std::size_t N = 1;
    ObjectPool<TestObject, N> pool;

    auto* obj = pool.allocate(99, "payload");
    EXPECT_EQ(obj->id, 99);
    EXPECT_EQ(obj->payload, "payload");

    pool.deallocate(obj);
    EXPECT_EQ(TestObject::live_count, 0);
}

