# 🚀 C++ Systems Playground

A hands-on C++ monorepo designed to practice **systems programming fundamentals**, including:

* custom memory allocation
* in-memory data structures
* hash tables (chaining + open addressing)
* caching
* RAII + smart pointers
* modular CMake builds
* concurrency
* logging
* unit testing

This repository contains multiple realistic projects that build on each other — moving from memory allocators to hash maps, KV stores, caching, and pointer semantics.

---

## 📦 Modules Overview

| Module               | Description                                                                   |
| -------------------- | ----------------------------------------------------------------------------- |
| `common/`            | Shared utilities (logging)                                                    |
| `memory_pool/`       | Fixed-block allocator for efficient small allocations                         |
| `hash_map/`          | Custom separate-chaining HashMap (replaces `std::unordered_map`)              |
| `kv_store_chaining/` | Key/value store using chaining + memory_pool                                  |
| `kv_store_linear/`   | Key/value store using open addressing + inline fixed buffers (cache-friendly) |
| `lru_cache/`         | Modern LRU cache built using `hash_map` + `std::list`                         |
| `in_memory_redis/`   | Redis-style store with TTL, prefix queries & background sweeper               |
| `smart_pointers/`    | Custom RAII smart pointers (`UniquePtr`, `SharedPtr`, `WeakPtr`)              |

Each module:

* is self-contained
* exports a library target
* has a demo
* has unit tests

---

## 🧩 Monorepo Structure

```
cpp-systems-playground/
│
├── CMakeLists.txt
│
├── common/                     
├── memory_pool/                
├── hash_map/
│
├── kv_store_chaining/          # chaining + memory pool
├── kv_store_linear/            # open addressing / linear probing
│
├── lru_cache/
│
├── in_memory_redis/
│
├── smart_pointers/
│
└── build/
```

### Differences between KV store modules:

| Module              | Collision Handling | Allocation Model | Value Storage     | Best For            |
| ------------------- | ------------------ | ---------------- | ----------------- | ------------------- |
| `kv_store_chaining` | Separate chaining  | memory pool      | dynamic           | flexible workloads  |
| `kv_store_linear`   | Open addressing    | contiguous array | fixed-size inline | high cache locality |

---

## ⚙️ Build Instructions

Configure:

```bash
cmake -S . -B build
```

Build all:

```bash
cmake --build build -j
```

Build a specific module (examples):

```bash
cmake --build build --target kv_store_chaining_demo
cmake --build build --target kv_store_linear_demo
cmake --build build --target smart_pointers_demo
```

---

## 🔥 Run Demos

```bash
./build/hash_map/hash_map_demo
./build/kv_store_chaining/kv_store_chaining_demo
./build/kv_store_linear/kv_store_linear_demo
./build/lru_cache/lru_cache_demo
./build/smart_pointers/smart_pointers_demo
```

---

## 🧪 Unit Testing

Run all tests:

```bash
cd build
ctest --output-on-failure
```

Examples:

```bash
./build/kv_store_linear/kv_store_linear_tests
./build/kv_store_chaining/kv_store_chaining_tests
./build/smart_pointers/smart_pointers_tests
```

Tests are small, assert-driven, and dependency-free (except Redis tests may use gtest).

---


## 🚧 Future Enhancements

Next possible modules:

* thread-safe `hash_map` & `lru_cache`
* robin-hood / quadratic probing version of `kv_store_linear`
* PMR-backed allocators for all structures
* persistence layer / WAL for Redis
* lock-free queue or stack
* bloom filter for key existence
* benchmarking suite

---

## 🤝 Contributing

Contributions welcome:

* performance optimizations
* lock-free variations
* testing & benchmarking
* more data structures
* allocator experiments

This playground is a space to explore, break, optimize, and learn.

---

## 📄 License

MIT — free to use, modify, and learn from.

---
