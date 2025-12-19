
# 🧠 Memory Pool — Fixed-Block Allocator

This module implements a **fixed-block memory pool** designed to provide deterministic and fast allocations for objects of uniform size. It is a foundational systems component often used in:

* networking stacks
* game engines
* blockchains
* embedded systems
* custom data structures

---

## 🚀 Features

* Fixed-size block allocator
* Constant-time `allocate()` and `deallocate()`
* Free-list backed implementation
* Placement new support
* No fragmentation
* Header-only for easy reuse
* Integrated logging (via `common/logging.hpp`)

---

## 📁 File Overview

```
memory_pool/
│
├── include/memory_pool/
│   └── fixed_block_memory_pool.hpp   # allocator implementation
│
├── src/
│   └── memory_pool_demo.cpp          # usage demo
│
└── CMakeLists.txt                    # builds library + demo
```

---

## ⚙️ How It Works

### 1️⃣ Pool Pre-Allocation

At construction, the pool allocates:

```
capacity × block_size
```

bytes in a single contiguous buffer.

### 2️⃣ Free List Layout

Each block starts with a pointer to the next free block:

```
[block0] -> [block1] -> [block2] -> ... -> nullptr
```

This allows constant-time:

* popping a block on allocate
* pushing it back on deallocate

### 3️⃣ Allocation

```cpp
void* allocate();
```

* returns a raw block
* caller constructs object via placement-new:

```cpp
void* raw = pool.allocate();
auto* node = new (raw) Node{...};
```

### 4️⃣ Deallocation

```cpp
pool.deallocate(ptr);
```

Caller must:

* call destructor manually
* return block to pool

```cpp
node->~Node();
pool.deallocate(node);
```

This mirrors real custom allocators and arenas.

---

## 🧪 Demo

Run the example:

```bash
./build/memory_pool/memory_pool_demo
```

Example output:

```
========================================
  FixedBlockMemoryPool demo
========================================
Block size: 16 bytes
Capacity  : 8 blocks
Node value = 4 at 0x7ff...
Node value = 3 at 0x7ff...
...
All nodes destroyed and deallocated back to the pool.
```

---

## 🧠 Why a Memory Pool?

General `new/delete`:

* allocate per object
* metadata overhead
* fragmentation risk
* slower

Memory pools benefit:

* uniform object size → predictable
* one large allocation → better locality
* no fragmentation → stable latency
* extremely fast free/alloc → O(1)

This is common in:

* real-time systems
* network packet buffers
* custom container implementations
* object pools in game loops
* blockchain node storage

---

## ✏ Example Usage

```cpp
FixedBlockMemoryPool pool(sizeof(Node), 8);

void* raw = pool.allocate();
Node* n = new (raw) Node{42, nullptr};

LOG_DEBUG("Allocated node");

n->~Node();
pool.deallocate(n);

LOG_INFO("Returned block to pool");
```

---

## 🔧 CMake Integration

The pool is exported as a library:

```cmake
add_library(memory_pool_lib INTERFACE)
target_include_directories(memory_pool_lib INTERFACE include)
target_link_libraries(memory_pool_lib INTERFACE common)
```

Other modules link via:

```cmake
target_link_libraries(kv_store_lib PUBLIC memory_pool_lib)
```

---

## ✨ Extension Ideas

Potential feature upgrades:

* growable pool (fallback to heap)
* lock-free free list
* thread-local pools
* debug tracking for allocations
* alignment control
* STL allocator adapter (`std::pmr::memory_resource`)

---

## 📦 Dependencies

* C++20
* `common/logging.hpp` (optional, used in demo)

No external libraries required.

---

## 🎯 Learning Outcomes

By working with this module, you gain hands-on understanding of:

* pointer arithmetic
* free-list design
* placement new
* manual destructor calls
* resource boundaries (allocation vs construction)
* allocator-aware data structures

---
