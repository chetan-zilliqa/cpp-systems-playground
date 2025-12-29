# 🚀 C++ Systems Playground

A hands-on C++ monorepo designed to practice **systems programming fundamentals**, including:

* custom memory allocation
* in-memory data structures
* hash tables (chaining + open addressing)
* caching & eviction
* RAII + smart pointers
* lock-free concurrency
* thread pools & work stealing
* modular CMake builds
* logging
* unit testing

This repository contains multiple realistic projects that build on each other — progressing from memory allocators → hash maps → KV stores → caching → smart pointers → concurrency.

---

## 📦 Modules Overview

| Module               | Description                                                            |
| -------------------- | ---------------------------------------------------------------------- |
| `common/`            | Shared utilities (logging)                                             |
| `memory_pool/`       | Fixed-block allocator for efficient small allocations                  |
| `hash_map/`          | Custom separate-chaining HashMap (replaces `std::unordered_map`)       |
| `kv_store_chaining/` | KV store using chaining + memory_pool                                  |
| `kv_store_linear/`   | KV store using open addressing + fixed-inline buffers (cache-friendly) |
| `lru_cache/`         | Modern LRU cache using `hash_map` + `std::list`                        |
| `in_memory_redis/`   | Redis-style store with TTL, prefix lookup, background sweeper          |
| `smart_pointers/`    | Custom RAII pointers (`UniquePtr`, `SharedPtr`, `WeakPtr`)             |
| `lock_free_queue/`   | MPMC lock-free queue using atomic CAS (Michael–Scott style)            |
| `thread_pool/`       | Work-stealing thread pool with per-thread task queues & futures        |

Each module:

* is standalone
* exports a library target
* has a demo executable
* has unit tests
* avoids dependencies except STL / gtest

---

## 📋 Module Dependency Graph

```
common (logging utilities)
  ├─ memory_pool (allocators)
  │  └─ kv_store_chaining
  │     └─ (application layer)
  │
  ├─ hash_map (chaining-based)
  │  ├─ kv_store_chaining
  │  └─ lru_cache
  │     └─ in_memory_redis
  │
  ├─ kv_store_linear (independent)
  ├─ smart_pointers (RAII utilities)
  ├─ lock_free_queue (concurrency)
  └─ thread_pool (concurrency)
```

**Dependency Rules:**
- `common` ← base layer (no dependencies)
- Foundation layer ← `common` only
- Application layer ← Foundation + others as needed
- Concurrency (`lock_free_queue`, `thread_pool`) ← `common` only (no cross-dependencies)

---

## 🧩 Monorepo Structure

```
cpp-systems-playground/
│
├── common/
├── memory_pool/
├── hash_map/
│
├── kv_store_chaining/
├── kv_store_linear/
│
├── lru_cache/
├── in_memory_redis/
│
├── smart_pointers/
├── lock_free_queue/
├── thread_pool/
│
└── build/
```

### KV Store Differences

| Module         | Collision Handling | Allocation | Value Storage | Strength               |
| -------------- | ------------------ | ---------- | ------------- | ---------------------- |
| chaining       | separate chaining  | pool       | dynamic       | flexible & simple      |
| linear probing | open addressing    | contiguous | fixed inline  | fast & cache-efficient |

### Concurrency Modules

| Module            | Focus / Concept                             |
| ----------------- | ------------------------------------------- |
| `lock_free_queue` | atomic CAS, ABA avoidance, MPMC queues      |
| `thread_pool`     | work stealing + futures + per-thread queues |

---

## ⚙️ Build Instructions

Configure:

```bash
cmake -S . -B build
```

Build everything:

```bash
cmake --build build -j
```

Sample specific targets:

```bash
cmake --build build --target lock_free_queue_demo
cmake --build build --target thread_pool_demo
cmake --build build --target in_memory_redis_demo
```

---

## 🔥 Run Demos

```bash
./build/lock_free_queue/lock_free_queue_demo
./build/thread_pool/work_stealing_thread_pool_demo
```

Also available:

```bash
./build/hash_map/hash_map_demo
./build/lru_cache/lru_cache_demo
./build/smart_pointers/smart_pointers_demo
```

---

## 🧪 Unit Testing

Run all:

```bash
cd build
ctest --output-on-failure
```

Examples:

```bash
./build/lock_free_queue/lock_free_queue_tests
./build/thread_pool/thread_pool_tests
./build/kv_store_linear/kv_store_linear_tests
```

---

## 🌟 Learning Outcomes

By completing this repo, you’ll understand:

### Data Structures

* hash maps (chaining + open addressing)
* memory pool allocators
* KV stores
* LRU caching

### Concurrency

* lock-free MPMC queues
* thread pools & futures
* work-stealing schedulers
* condition variable wake/sleep
* atomic CAS operations

### Modern C++ Concepts

* RAII + ownership models
* templates + type traits
* `std::invoke_result_t`
* `std::function`, `std::packaged_task`
* orphan avoidance & move semantics

---

## 🚧 Future Enhancements

Potential next modules:

* robin-hood hashing
* PMR-optimized structures
* persistent backed Redis (WAL)
* parallel sort / map-reduce
* Lock-free stack
* actor runtime on thread pool
* benchmark suite

---

## 🤝 Contributing

Open to:

* performance improvements
* lock-free structures
* thread pool extensions
* bug reports & tests
* benchmark additions
* more allocator patterns

This is a playground — explore, break, optimize, and learn.

---

## 📄 License

MIT — free to use and modify.

---

