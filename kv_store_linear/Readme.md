# KVStore_linear (Open Addressing KV Store)

`kvstore_linear` is a high-performance, fixed-capacity, in-memory key/value store that uses **open addressing** with **linear probing** instead of separate chaining.

This project focuses on:

* cache-friendly lookup
* compact memory layout
* reduced indirection
* fixed-size key/value buffers
* predictable performance

It is designed as a lightweight educational example of building a hash table optimized for speed and locality.

---

## ✨ Highlights

| Feature                      | Description                                 |
| ---------------------------- | ------------------------------------------- |
| Open addressing              | Linear probing instead of linked lists      |
| Fixed-size buffers           | Keys and values stored inline (`char[]`)    |
| Cache-friendly               | Single allocation, contiguous table         |
| FNV-1a hash                  | Simple deterministic hashing                |
| Overwrite support            | Inserts update existing entries             |
| Optional return              | `std::optional<std::string_view>` for reads |
| Fast lookup                  | No pointer chasing, lower fragmentation     |
| Zero dynamic per-entry alloc | Single allocation of bucket array           |

Unlike the general `kv_store/` module (memory-pool backed, dynamic strings), this variant is optimized for:

* fixed maximum key/value sizes
* large number of entries
* constant memory footprint
* predictable hot-path perf

---

## 📁 Directory Structure

```
kvstore_linear/
  include/
    kvstore_linear.hpp
  src/
    main.cpp
  tests/
    kvstore_linear_tests.cpp
  CMakeLists.txt
```

---

## 🔧 Key Design Choices

### 1️⃣ Open Addressing + Linear Probing

```cpp
size_t idx = hash(key) % cap;
for (i = 0; i < cap; ++i)
{
    slot = (idx + i) % cap;
    ...
}
```

This avoids:

* dynamic bucket vectors
* separate node allocations
* pointer chasing

### 2️⃣ Inline Storage

```cpp
char key[MAX_KEY + 1];
char value[MAX_VALUE + 1];
```

This ensures:

* contiguous layout → better cache locality
* no heap fragmentation
* predictable latency

### 3️⃣ Hashing

Uses FNV-1a:

```cpp
h = (h ^ c) * 16777619
```

Simple, fast, reproducible.

---

## 🚀 Demo

Build:

```bash
cmake --build build --target kvstore_linear_demo
./build/kvstore_linear/kvstore_linear_demo
```

Sample output (for 50k inserts):

```
KVStore_linear demo
Capacity: 100000, inserting: 50000 entries

Bulk insert done in 12 ms
Store size (approx): 50000
[OK] key "key_9973" verified
...
```

---

## 🧪 Unit Tests

Tests use GoogleTest:

```bash
cmake --build build --target kvstore_linear_tests
./build/kvstore_linear/kvstore_linear_tests
```

They cover:

* basic insert and get
* overwrite behavior
* erase semantics

Example:

```cpp
store.insert("key", "value1");
store.insert("key", "value2");
EXPECT_EQ(store.get("key").value(), "value2");
```

---

## Performance Notes

Because:

* entries are contiguous
* hashing is simple
* values are stored inline

This design generally offers:

| Operation        | Time Complexity |
| ---------------- | --------------- |
| insert (average) | O(1)            |
| get (average)    | O(1)            |
| erase            | O(1)            |

Worst-case degenerates to O(n) due to probing, but the memory access pattern is sequential and cache-friendly.

---

## Trade-offs

### Pros

* extremely fast hot path
* minimal dynamic allocation
* compact storage
* predictable memory footprint

### Cons

* fixed key/value size limits
* no tombstone handling (simple erase)
* no TTL / prefix search
* table cannot grow (fixed capacity)

---

## When to Use

Good for:

* embedded / constrained systems
* cache layers
* telemetry/indexing workloads
* high-throughput in-memory workloads
* fixed schema KV stores

Not ideal for:

* variable-length keys
* dynamic growth
* multi-threaded environments (current version)

---

## Future Enhancements

Ideas to explore:

* quadratic probing / robin hood hashing
* tombstones for erase correctness
* resize / rehash support
* SIMD hashing
* PMR allocator support
* cache line padding to avoid false sharing
* batch insert optimization

---

## License

MIT (or same as parent repo)

---

