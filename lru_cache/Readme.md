# LRU Cache (C++)

`lru_cache` is a modern, generic implementation of a **Least Recently Used (LRU)** cache.
It provides constant-time lookup and eviction using an efficient combination of:

* **std::list** for O(1) LRU ordering
* **std::unordered_map** for O(1) key lookup

This project is part of the **cpp-systems-playground** repository and demonstrates a production-grade LRU cache design suitable for systems programming & high-performance applications.

---

## ✨ Features

| Feature            | Description                                            |
| ------------------ | ------------------------------------------------------ |
| `put(key, value)`  | Insert/update a key-value pair.                        |
| `get(key)`         | Returns value if present and refreshes its LRU status. |
| O(1) operations    | `get` and `put` run in constant time.                  |
| Automatic eviction | Oldest key evicted when capacity is exceeded.          |
| Generic            | Works with arbitrary `Key` and `Value` types.          |
| Move-aware         | Supports move semantics.                               |
| Optional return    | Uses `std::optional` for cache miss.                   |
| Clear + erase      | Remove individual keys or reset whole cache.           |

---

## 🛠 Internals

The core data structures:

* `std::list<std::pair<Key,Value>>`

  * Front: Most Recently Used (MRU)
  * Back: Least Recently Used (LRU)

* `std::unordered_map<Key, ListIterator>`

  * Direct iterator access into the list

Eviction strategy:

* On `put()`:

  * If key exists → update + move to MRU
  * Else if at capacity → remove LRU (list back)

Time complexity:

| Operation | Complexity |
| --------- | ---------- |
| `get()`   | O(1)       |
| `put()`   | O(1)       |
| `erase()` | O(1)       |
| `clear()` | O(n)       |

---

## 📁 Directory Structure

```
lru_cache/
  include/
    lru_cache/
      lru_cache.hpp
  src/
    main.cpp        # Demo usage
  tests/
    lru_cache_tests.cpp
  CMakeLists.txt
```

---

## 🚀 Build & Run

From the repository root:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target lru_cache_demo
./lru_cache_demo
```

Expected output:

```
Get 1: 10
Get 2: 20
Get 3: <miss>
Get 4: 40
...
```

---

## 📖 Example Usage

```cpp
#include "lru_cache/lru_cache.hpp"

int main() {
    lru::LRUCache<int, std::string> cache(2);

    cache.put(1, "one");
    cache.put(2, "two");

    auto v = cache.get(1); // hits + refresh LRU
    cache.put(3, "three"); // evicts key 2

    assert(!cache.get(2).has_value());
    assert(cache.get(3).value() == "three");
}
```

---

## 🧪 Tests

Tests are located in:

```
lru_cache/tests/lru_cache_tests.cpp
```

Run via:

```bash
ctest --output-on-failure
```

Includes:

* basic get/put
* LRU eviction correctness
* update/move-to-front
* erase & clear
* contains()
* exception on zero capacity

---

## 🎯 Use Cases

LRU caches are widely used in:

* database/page caching
* memory-constrained embedded systems
* web caching / proxy layers
* compiler runtime & JIT memoization
* block device caching
* DNS / ARP caching

---

## 🔥 Possible Enhancements

Optional future directions:

* thread-safe wrapper
* TTL-based eviction (LRU + expiration)
* shard-based striping
* LFU / ARC hybrid policies
* custom allocator support

---


## 📜 License

MIT (or same as parent repo)

---
