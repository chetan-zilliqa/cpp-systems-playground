#include <gtest/gtest.h>
#include "kvstore_optimized.hpp"

TEST(KVStoreOptimized, BasicInsertGet) {
    kv_opt::KVStoreOptimized store(128);

    ASSERT_TRUE(store.insert("foo", "bar"));
    ASSERT_TRUE(store.insert("hello", "world"));

    auto v1 = store.get("foo");
    auto v2 = store.get("hello");

    ASSERT_TRUE(v1.has_value());
    ASSERT_TRUE(v2.has_value());

    EXPECT_EQ(v1.value(), "bar");
    EXPECT_EQ(v2.value(), "world");
}

TEST(KVStoreOptimized, Overwrite) {
    kv_opt::KVStoreOptimized store(64);

    store.insert("key", "value1");
    store.insert("key", "value2");

    EXPECT_EQ(store.get("key").value(), "value2");
}

TEST(KVStoreOptimized, Erase) {
    kv_opt::KVStoreOptimized store(64);

    store.insert("k", "v");
    ASSERT_TRUE(store.erase("k"));
    ASSERT_FALSE(store.get("k").has_value());
}

