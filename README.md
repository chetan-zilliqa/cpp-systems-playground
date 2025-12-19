
# 📦 C++ Systems Playground

A hands-on C++ monorepo to practice **systems programming, memory allocators, and key-value store design**.

This repository contains multiple small but realistic systems projects that build on each other:

* **memory_pool** — a fixed-block custom allocator
* **kv_store** — an in-memory key–value store using the memory pool
* **common** — shared utilities (e.g., lightweight logging)

The goal is to simulate real-world C++ development workflows:

* modular design
* reusable libraries
* out-of-source CMake builds
* clean separation of headers vs. sources
* linkage between sub-projects
* debugging + iterative development

---

## 🗂 Repository Structure

```
cpp-systems-playground/
├── CMakeLists.txt          # root cmake project
├── common/                 # shared utilities
│   ├── include/common/
│   │   └── logging.hpp
│   └── CMakeLists.txt
├── memory_pool/            # fixed-block memory pool library + demo
│   ├── include/memory_pool/fixed_block_memory_pool.hpp
│   ├── src/main.cpp
│   └── CMakeLists.txt
├── kv_store/               # key-value store backed by memory pool
│   ├── include/kv_store/kv_store.hpp
│   ├── src/kv_store.cpp
│   ├── src/main.cpp
│   └── CMakeLists.txt
└── build/                  # CMake build output (ignored by git)
```

Each subproject exposes a CMake library target:

| Project     | Target Name       | Type       |
| ----------- | ----------------- | ---------- |
| common      | `common`          | INTERFACE  |
| memory_pool | `memory_pool_lib` | INTERFACE  |
| kv_store    | `kv_store_lib`    | STATIC/OBJ |

Demo executables:

* `memory_pool_demo`
* `kv_store_demo`

---

## 🚀 Build & Run

### Configure (run from repo root)

```bash
cmake -S . -B build
```

### Build everything

```bash
cmake --build build -j
```

### Run demos

```bash
./build/memory_pool/memory_pool_demo
./build/kv_store/kv_store_demo
```

---

## 🧱 Components

### 🧩 `memory_pool/`

A fixed-block allocator:

* allocates a pool of N fixed-size blocks
* supports `allocate()` / `deallocate()`
* uses placement new
* free list implemented via pointer-chaining

Goals:

* understand manual memory control
* observe behavior via pointer addresses
* contrast with `new` / `delete` overhead

### 🗄 `kv_store/`

An in-memory KV store:

* std::hash-based bucket indexing
* separate chaining via linked lists
* custom allocation from the memory pool
* thread-safe reads/writes via shared_mutex
* logging instrumentation

This demonstrates:

* integration between modules
* allocator-aware data structures
* synchronization primitives

### 🧰 `common/`

Currently contains:

* lightweight logging with levels (DEBUG, INFO, WARN, ERROR)
* can be expanded for utilities or configuration helpers

---

## 🧩 Planned Extensions

Some potential next steps:

* add unit tests (GoogleTest or Catch2)
* add a block allocator benchmark vs `new`
* add an LRU cache layer over kv_store
* make pool dynamically growable
* implement thread-local allocators
* introduce exceptions vs noexcept policies
* custom allocators for `std::vector`

---


## 💬 Contributing / Customizing

Feel free to:

* rename demo binaries
* convert memory_pool to a polymorphic allocator
* replace `std::vector` buckets with open addressing
* add benchmarks
* integrate sanitizer builds
* add CI with GitHub actions

---

## 📄 License

MIT — free to learn, modify, and extend.

---

