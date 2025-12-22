# In-Memory Redis (C++)

`in_memory_redis` is a lightweight, single-header + source implementation of a Redis-like key-value store written in modern C++20.

It supports:

* Key-value storage
* Time-to-live (TTL) expiration
* Prefix search (lexicographic)
* Concurrency (thread-safe)
* Background expiration sweeper

This module is part of the **cpp-systems-playground** repository and demonstrates building a miniature Redis-style in-memory data store with efficient TTL management.

---

## ✨ Features

| Feature                | Description                                                           |
| ---------------------- | --------------------------------------------------------------------- |
| `put(key, value, ttl)` | Insert or update key. Optional TTL in milliseconds.                   |
| `get(key)`             | Fetch value if present and not expired.                               |
| `erase(key)`           | Remove key explicitly.                                                |
| `prefix_get(prefix)`   | Returns all keys starting with a prefix.                              |
| Automatic TTL Sweeper  | Background thread removes expired keys using a min-heap.              |
| Thread-safe            | Readers/writers fully synchronized using shared mutex and heap mutex. |
| Sorted keys            | Backed by std::map for ordered prefix scanning.                       |

---

## 🛠 Internals

Core design choices:

* **std::map** is used to keep keys sorted → efficient `prefix_get()`.
* **std::priority_queue (min-heap)** schedules TTL expiry efficiently.
* **std::shared_mutex** enables concurrent reads and exclusive writes.
* **Background sweeper thread** wakes based on earliest expiration.
* **Version counters** ensure correct erase in presence of key updates.

This gives O(log N) insert + expiry and deterministic prefix scanning.

---

## 📁 Directory Structure

```
in_memory_redis/
  include/
    in_memory_redis/
      kv_store.hpp
  src/
    kv_store.cpp
    main.cpp
  CMakeLists.txt
```

---

## 🚀 Build & Run

From project root:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target in_memory_redis_demo
./in_memory_redis_demo
```

Expected output:

```
prefix 'ap':
  app -> prefix
  apple -> red
  apricot -> orange
get apple after ttl: expired
prefix 'ap' after ttl:
  app -> prefix
  apricot -> orange
```

---

## 📖 Example Usage

```cpp
using in_memory_redis::KVStore;

KVStore kv;

// Set with TTL (500 ms)
kv.put("apple", "red", KVStore::Ms{500});

// No TTL
kv.put("app", "prefix");

// Read
auto v = kv.get("apple");

// Prefix search
auto list = kv.prefix_get("ap");
```

---

## ⚙️ Concurrency Model

* Readers use **shared_lock**
* Writers use **unique_lock**
* Expiry heap protected by a dedicated mutex
* Background thread waits on condition variable
* `stop_` atomic coordinates shutdown safely

This allows non-blocking `get()` operations even with many concurrent readers.

---

## 🔥 Why prefix search?

Redis `SCAN` and filtered operations are expensive.

Here we exploit:

* `std::map` sorted order
* prefix upper-bound calculation (`nextPrefix()`)

So:

```cpp
prefix_get("ap")
```

efficiently returns:

```
app, apple, apricot
```

without scanning the whole keyspace.

---

## 🧩 Limitations (current)

This is a demo project—not a production store.
Limitations include:

* Single-node, in-process only
* No persistence / AOF
* No networking protocol
* Expired items visible until sweeper wakes
* Value type restricted to `std::string`

---

## 🧱 Possible Extensions

If you want next steps:

* Redis-protocol parser
* TCP server over epoll
* Multi-shard (lock-stripped) buckets
* Pluggable serializer
* Optional persistence (mmap / append-only)
* Pub/sub channels

---


## 📜 License

MIT (or same as parent repo)

---

