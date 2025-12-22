# Lock-Free SPSC Queue (C++)

`lock_free_queue` is a modern C++ implementation of a **single-producer / single-consumer (SPSC)** lock-free queue using:

* a bounded ring buffer
* `std::atomic` indices
* contiguous storage
* RAII construction/destruction

This module demonstrates the fundamentals of lock-free data structures and is optimized for low latency and high throughput message passing.

---

## ✨ Features

| Feature                               | Description                                           |
| ------------------------------------- | ----------------------------------------------------- |
| Lock-free                             | No mutexes or OS synchronization                      |
| Wait-free (no contention)             | Producer & consumer proceed independently             |
| SPSC design                           | Safe for exactly one producer and one consumer thread |
| Ring buffer                           | Fixed-capacity, cache-friendly                        |
| Placement new                         | Manual control of object lifetime                     |
| Move & copy push support              | `push(const T&)` + `push(T&&)`                        |
| `emplace()` API                       | Construct in-place                                    |
| Zero dynamic alloc after construction | Fully bounded                                         |

---

## 📁 Directory Structure

```
lock_free_queue/
  include/
    lock_free_queue/
      spsc_queue.hpp
  src/
    main.cpp
  tests/
    lock_free_queue_tests.cpp
  CMakeLists.txt
```

---

## 🛠 Implementation Overview

The queue uses a fixed-size ring buffer:

* `head_` → producer index (write position)
* `tail_` → consumer index (read position)

```text
[slot][slot][slot][slot]...
   ^tail           ^head
```

Capacity is `N + 1` to differentiate `full` vs `empty`.

Index updates:

* producer:

  * load `tail_` (acquire)
  * write to buffer
  * store `head_` (release)

* consumer:

  * load `head_` (acquire)
  * read & destroy element
  * store `tail_` (release)

Memory ordering:

* `relaxed` for reads
* `release/acquire` on index updates

The internal storage uses `std::aligned_storage_t` so no dynamic allocation occurs per push/pop.

---

## 🚀 Example Usage

```cpp
#include "lock_free_queue/spsc_queue.hpp"

int main() {
    lock_free::SPSCQueue<int, 1024> queue;

    queue.push(10);
    queue.emplace(20);

    int value;
    if (queue.pop(value)) {
        // value == 10
    }
}
```

---

## 🧪 Demo Program

Build and run:

```bash
cmake --build build --target lock_free_queue_demo
./build/lock_free_queue/lock_free_queue_demo
```

Output (example):

```
Consumed 10000 items successfully.
```

---

## 🧪 Unit Tests

Tests validate:

* pushing and popping
* full/empty detection
* correctness under two threads
* ring wraparound behavior
* object construction/destruction

Run via:

```bash
ctest --output-on-failure
```

Or:

```bash
./build/lock_free_queue/lock_free_queue_tests
```

---

## 🚧 Threading Contract

This implementation **requires**:

* exactly **one producer thread**
* exactly **one consumer thread**

Violating this results in undefined behavior.

### Intended Use Cases

* messaging between two threads
* work submission → worker thread
* real-time producer ↔ consumer links
* audio/video pipelines
* actor systems

---

## 🔥 Performance Characteristics

| Operation           | Complexity |
| ------------------- | ---------- |
| push                | O(1)       |
| pop                 | O(1)       |
| empty/full checking | O(1)       |

Cache locality is excellent due to contiguous memory and minimal indirection.

No malloc/free is performed after creation.

---

## 📌 Extensions & Next Steps

Possible evolutions:

* MPMC queue (Michael–Scott algorithm)
* lock-free freelist + allocator
* exponential backoff for contention
* non-blocking multi-slot batch operations
* release/consume semantics tuning
* exposed capacity querying

---

## 📜 License

MIT (or same as parent repo)

---

Understanding this lays the groundwork for more advanced:

* MPMC queues
* concurrent hash tables
* lock-free freelists
* actor runtimes

---
