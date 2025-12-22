# LRU Cache (C++)

`lru_cache` is a modern, generic implementation of a **Least Recently Used (LRU)** cache.

It provides constant-time lookup and eviction using an efficient combination of:

* **std::list** for O(1) LRU ordering
* **our own HashMap implementation** for O(1) key lookup (instead of `std::unordered_map`)

This module is part of the **cpp-systems-playground** repository, and demonstrates a production-grade LRU cache design that combines custom hash-table internals with a well-known eviction policy.

---

## ✨ Features

| Feature            | Description                                              |
| ------------------ | -------------------------------------------------------- |
| `put(key, value)`  | Insert/update a key-value pair.                          |
| `get(key)`         | Returns value if present and refreshes its LRU status.   |
| O(1) operations    | `get` and `put` run in constant time.                    |
| Custom HashMap     | Uses our own hash-table instead of `std::unordered_map`. |
| Automatic eviction | Oldest key evicted when capacity is exceeded.            |
| Generic            | Works with arbitrary `Key` and `Value` types.            |
| Move-aware         | Supports move semantics.                                 |
| Optional return    | Uses `std::optional` for cache miss.                     |
| Clear + erase      | Remove individual keys or reset whole cache.             |

---

## 🛠 Internals

The core data structures:

* `std::list<std::pair<Key,Value>>`

  * Front → Most Recently Used (MRU)
  * Back → Least Recently Used (LRU)

* `hash_map::HashMap<Key, ListIterator>`

  * Our own separate-chaining hash table
  * Provides O(1) lookup, insert, and erase
  * Lets us decouple from `std::unordered_map`

### Eviction strategy

* On `put()`:

  * If key exists → update + move to MRU
  * Else if full → evict back (LRU node)
  * Insert new node at front (MRU)

### Complexity

| Operation | Complexity |
| --------- | ---------- |
| `get()`   | O(1)       |
| `put()`   | O(1)       |
| `erase()` | O(1)       |
| `clear()` | O(n)       |

(Assuming the HashMap load factor remains controlled)

---

## 📁 Directory Structure

```
lru_cache/
  include/
    lru_cache/
      lru_cache.hpp     # uses our custom HashMap
  src/
    main.cpp            # demo usage
  tests/
    lru_cache_tests.cpp # unit tests
  CMakeLists.txt
```

`hash_map/` module lives alongside and is linked to this library.

---

## 🚀 Build & Run

From repo root:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target lru_cache_demo
./lru_cache_demo
```

---

## 📖 Example Usage

```cpp
#include "lru_cache/lru_cache.hpp"

int main() {
    lru::LRUCache<int, std::string> cache(2);

    cache.put(1, "one");
    cache.put(2, "two");

    auto v = cache.get(1); // MRU
    cache.put(3, "three"); // evicts key 2

    assert(!cache.get(2).has_value());
    assert(cache.get(3).value() == "three");
}
```

---

## 🧪 Tests

Located in:

```
lru_cache/tests/lru_cache_tests.cpp
```

To run:

```bash
ctest --output-on-failure
```

Covers:

* basic get/put
* LRU eviction correctness
* update move-to-MRU
* erase & clear
* contains()
* zero-capacity exception

---

## 🎯 Use Cases

LRU caching fits well in:

* in-memory database caching
* filesystems & block caching
* compiler optimization caches
* virtual memory paging
* network layer caching (DNS, ARP)
* embedded memory-bounded systems

---

## 🔥 Possible Enhancements

* thread-safe wrapper
* TTL + LRU hybrid eviction
* striped hashing for lock sharding
* LFU/LRU or adaptive replacement cache
* custom allocators / pmr
* serialization + persistence

---

## 📜 License

MIT (or parent repo license)

---
