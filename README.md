# 🚀 C++ Systems Playground

A hands-on C++ monorepo designed to practice **systems programming fundamentals**, including:

* custom memory allocation
* in-memory data structures
* hash tables & caching
* modular CMake builds
* concurrency
* logging
* unit testing

This repository contains multiple small but realistic projects that build on each other, moving from fundamentals (memory) to increasingly advanced data systems (KV stores & caches).

---

## 📦 Modules Overview

| Module             | Description                                                                            |
| ------------------ | -------------------------------------------------------------------------------------- |
| `common/`          | Shared utilities (logging)                                                             |
| `memory_pool/`     | Fixed-block allocator for efficient small allocations                                  |
| `hash_map/`        | Custom separate-chaining HashMap (replacing `std::unordered_map` in dependent modules) |
| `kv_store/`        | In-memory key/value store backed by `memory_pool` (optimized, TTL support)             |
| `lru_cache/`       | Modern LRU cache built using our custom `hash_map` + `std::list`                       |
| `in_memory_redis/` | Redis-style key-value store with prefix query + TTL expiry + background sweeper        |

Each module:

* is self-contained
* exports a library target
* has a demo executable
* has unit tests

---

## 🧩 Monorepo Structure

```
cpp-systems-playground/
│
├── CMakeLists.txt              # root cmake project
│
├── common/                     # logging, shared utilities
│   ├── include/common/logging.hpp
│   ├── tests/common_logging_tests.cpp
│   └── README.md
│
├── memory_pool/                # fixed-block allocator
│   ├── include/memory_pool/fixed_block_memory_pool.hpp
│   ├── tests/memory_pool_tests.cpp
│   └── README.md
│
├── hash_map/                   # custom hash table
│   ├── include/hash_map/hash_map.hpp
│   ├── src/main.cpp
│   ├── tests/hash_map_tests.cpp
│   └── README.md
│
├── kv_store/                   # memory-pool backed kv store
│   ├── include/kv_store/kv_store.hpp
│   ├── src/main.cpp
│   ├── tests/kv_store_tests.cpp
│   └── README.md
│
├── lru_cache/                  # LRU cache using custom HashMap
│   ├── include/lru_cache/lru_cache.hpp
│   ├── src/main.cpp
│   ├── tests/lru_cache_tests.cpp
│   └── README.md
│
├── in_memory_redis/            # Redis-like TTL + prefix + sweeper
│   ├── include/redis/...
│   ├── tests/redis_tests.cpp
│   └── README.md
│
└── build/                      # cmake build directory (ignored by git)
```

---

## ⚙️ Build Instructions

### Configure (from root)

```bash
cmake -S . -B build
```

### Build all

```bash
cmake --build build -j
```

### Build a specific component

```bash
cmake --build build --target hash_map_demo
cmake --build build --target lru_cache_demo
cmake --build build --target kv_store_demo
```

---

## 🔥 Run Demos

```bash
./build/hash_map/hash_map_demo
./build/lru_cache/lru_cache_demo
./build/kv_store/kv_store_demo
```

---

## 🧪 Unit Testing

Integrated using **CTest**.

```bash
cd build
ctest --output-on-failure
```

Example individual runs:

```bash
./build/hash_map/hash_map_tests
./build/lru_cache/lru_cache_tests
./build/memory_pool/memory_pool_tests
./build/kv_store/kv_store_tests
```

Tests use `assert()` and are dependency-free, fast, and deterministic.

---

## 🌟 Learning Goals

This repository aims to teach:

* designing custom containers (`HashMap`)
* eviction & caching policies (`LRU`)
* memory management (`fixed_block_memory_pool`)
* building decentralized modules
* clean CMake & modular dependencies
* profiling and optimization mindset
* testing systems components in isolation

By combining them, we explore real-world systems challenges:

**efficiency, cache locality, TTL expiry, eviction, custom allocators.**

---

## 🚧 Future Enhancements

Planned additions:

* PMR-compatible hashmap + LRU
* lock-free memory pool variant
* benchmark suite (Google Benchmark)
* MVCC or snapshot support in KV
* WAL/persistence layer for redis module
* sharded hash map & LRU for multithreading
* LFU/ARC adaptive cache policy
* serialization & IPC transports

---

## 🤝 Contributing

Contributions welcome:

* performance optimizations
* tests & benchmarking
* design improvements
* memory allocator experiments
* new modules (allocator, cache, db, etc.)

This is a playground — break things, learn, experiment.

---

## 📄 License

MIT — free to modify and learn from.

---
