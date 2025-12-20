# 🚀 C++ Systems Playground

A hands-on C++ monorepo designed to practice **systems programming fundamentals**, including:

* custom memory allocation
* in-memory data structures
* modular CMake builds
* concurrency
* logging
* unit testing

This repository contains multiple small but realistic projects that build on each other:

| Module         | Description                                         |
| -------------- | --------------------------------------------------- |
| `common/`      | Shared utilities (logging)                          |
| `memory_pool/` | Fixed-block allocator                               |
| `kv_store/`    | In-memory key/value store backed by the memory pool |

---

## 🧩 Monorepo Structure

```
cpp-systems-playground/
│
├── CMakeLists.txt               # root cmake project
│
├── common/                      # shared utilities module
│   ├── include/common/logging.hpp
│   ├── tests/common_logging_tests.cpp
│   └── README.md
│
├── memory_pool/                 # memory allocator module
│   ├── include/memory_pool/fixed_block_memory_pool.hpp
│   ├── src/main.cpp
│   ├── tests/memory_pool_tests.cpp
│   └── README.md
│
├── kv_store/                    # key-value store module
│   ├── include/kv_store/kv_store.hpp
│   ├── src/kv_store.cpp
│   ├── src/main.cpp
│   ├── tests/kv_store_tests.cpp
│   └── README.md
│
└── build/                       # cmake build directory (ignored by git)
```

Each module defines:

* a library target
* a demo executable
* a test executable

---

## ⚙️ Build Instructions

### Configure (from root)

```bash
cmake -S . -B build
```

### Build all modules

```bash
cmake --build build -j
```

### Build a specific target

```bash
cmake --build build --target memory_pool_demo -j
cmake --build build --target kv_store_demo -j
cmake --build build --target common_tests -j
```

### Run demos

```bash
./build/memory_pool/memory_pool_demo
./build/kv_store/kv_store_demo
```

---

## 🧪 Running Unit Tests

Tests are integrated using **CTest**.

```bash
cd build
ctest --output-on-failure
```

Or run individually:

```bash
./build/common/common_tests
./build/memory_pool/memory_pool_tests
./build/kv_store/kv_store_tests
```

The testing design is intentionally simple: assert-based, fast, and dependency-free.

---

## 🚧 Future Enhancements

Potential next modules:

* LRU cache using `kv_store`
* benchmark suite
* `std::pmr` compatible allocator
* lock-free pool
* persistent storage backend
* custom serialization
* thread-local allocators

---

## 🤝 Contributing

This project welcomes:

* refactors
* improved logging
* benchmarks
* optimizations
* tests
* additional modules

It is a playground — break things, learn, rebuild.

---

## 📄 License

MIT — free to modify and learn from.

---

