# 🗄 In-Memory Key–Value Store

This module implements a simple but realistic **in-memory key–value store** using:

* a custom hash table with separate chaining
* thread-safe access via `std::shared_mutex`
* **a custom memory pool allocator** for node storage

The purpose is educational: to practice combining custom allocators, data-structure design, and concurrency — similar to how caches and embedded stores work in real systems.

---

## 🚀 Features

* `put`, `get`, `erase`, `contains` operations
* bucket-based hash table
* linked-list chaining for collisions
* thread-safe operations using:

  * `std::unique_lock`
  * `std::shared_lock`
* allocator-backed node storage
* logging for visibility

---

## 🗂 Directory Layout

```
kv_store/
│
├── include/kv_store/
│   └── kv_store.hpp       # public interface + class definition
│
├── src/
│   ├── kv_store.cpp       # implementation
│   └── kv_store_demo.cpp  # demo program
│
└── CMakeLists.txt         # library + demo target
```

---

## 📦 Public API

### Create store

```cpp
InMemorykvstore_chaining store(num_buckets, max_items);
```

### Put

```cpp
store.put("key", "value");
```

### Get

```cpp
auto v = store.get("key");
if (v) std::cout << *v << "\n";
```

### Erase

```cpp
store.erase("key");
```

### Contains

```cpp
bool ok = store.contains("session:123");
```

### Size

```cpp
size_t n = store.size();
```

---

## ⚙️ Internal Design

### 🧱 Hash Table with Separate Chaining

```
buckets[i] -> Node -> Node -> nullptr
```

* bucket index = `hash(key) % bucket_count`
* collisions resolved using singly-linked list

### 🧠 Custom Allocator (Memory Pool)

Nodes are NOT created using `new` / `delete`.

Instead:

* allocation: `pool_.allocate()`
* construction: placement new
* destruction: explicit `->~Node()`
* deallocation: `pool_.deallocate(ptr)`

Why?

* faster allocation/deallocation
* better locality (contiguous pool memory)
* avoids heap fragmentation
* predictable latency

### 🔐 Thread Safety

* reads use `shared_lock`
* writes use `unique_lock`

This means:

* multiple concurrent readers allowed
* writes are exclusive

---

## 🧪 Demo

Run:

```bash
./build/kv_store/kv_store_demo
```

Example interaction:

```
========================================
  InMemorykvstore_chaining (pool-backed) demo
========================================
Putting some keys...
GET user:1 -> Alice
GET user:2 -> Bob
GET user:3 -> <null>
Size = 2
```

---

## 🔧 CMake Integration

Library definition:

```cmake
add_library(kv_store_lib src/kv_store.cpp)

target_include_directories(kv_store_lib PUBLIC include)

target_link_libraries(kv_store_lib
    PUBLIC
        common
        memory_pool_lib
)
```

Executable demo:

```cmake
add_executable(kv_store_demo src/kv_store_demo.cpp)
target_link_libraries(kv_store_demo PRIVATE kv_store_lib)
```

---

## 🎓 Concepts Practiced

This module reinforces:

* allocator-aware data structure design
* hash maps and collision handling
* ownership & lifetime management
* thread synchronization
* modular CMake linking
* separation of interface vs implementation

---

## 🧠 Real-World Applications

This architecture resembles:

* Redis-style in-memory structures
* blockchain node storage
* network state caches
* game engines
* embedded store components

---

## 🚧 Future Enhancements

Ideas for next steps:

* open addressing (linear probing or cuckoo)
* automatic rehashing
* growable memory pool
* TTL or LRU eviction
* iterator support
* persistent backing store
* benchmarking suite
* lock striping for improved concurrency

---

## 📦 Dependencies

* C++20
* `memory_pool_lib` (custom allocator)
* `common` (logging)

No external library requirements.

---

## 🎯 Goal of this Module

This is intentionally not a production-grade KV store.

It is a learning + systems-design playground demonstrating:

* custom allocation
* hash table internals
* modular CMake builds
* concurrency discipline

---

