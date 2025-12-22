# Smart Pointers (C++)

`smart_pointers` is a self-contained module implementing **custom RAII-based smart pointers** in modern C++.

Instead of using the standard library’s `std::unique_ptr` / `std::shared_ptr`, this project builds them from scratch to demonstrate:

* ownership semantics
* resource lifetime management
* reference counting
* weak references
* move-only types
* custom deleters
---

## ✨ Implemented Smart Pointers

### ✔ `UniquePtr<T>`

A move-only smart pointer providing **exclusive ownership** of a dynamically allocated object.

Features:

| Feature                 | Support |
| ----------------------- | ------- |
| Move semantics          | ✔       |
| Non-copyable            | ✔       |
| `reset()` / `release()` | ✔       |
| Custom deleter support  | ✔       |
| Array specialization    | ✔       |
| `make_unique()` helper  | ✔       |

### ✔ `SharedPtr<T>`

A reference-counted smart pointer using a **control block** to track strong references.

Features:

| Feature                            | Support |
| ---------------------------------- | ------- |
| Copy semantics                     | ✔       |
| Move semantics                     | ✔       |
| Reference counting (`use_count()`) | ✔       |
| Auto delete on last strong ref     | ✔       |
| Compatible `make_shared()`         | ✔       |

### ✔ `WeakPtr<T>`

A non-owning reference to a `SharedPtr`.

Features:

| Feature                        | Support |
| ------------------------------ | ------- |
| No ownership                   | ✔       |
| Locking (`lock() → SharedPtr`) | ✔       |
| Non-intrusive ref counting     | ✔       |
| Use-count inspection           | ✔       |
| Detect expired state           | ✔       |

> These implementations are non-thread-safe and optimized for learning, not production.

---

## 📁 Module Structure

```
smart_pointers/
  include/
    smart_pointers/
      unique_ptr.hpp
      shared_ptr.hpp
  src/
    main.cpp                     # demo usage
  tests/
    smart_pointers_tests.cpp     # assert-based tests
  CMakeLists.txt
```

---

## 🚀 Demo

Build and run:

```bash
cmake --build build --target smart_pointers_demo
./build/smart_pointers/smart_pointers_demo
```

Expected output (example):

```
=== UniquePtr demo ===
value = 42
up is null
UniquePtr reset() done

=== SharedPtr / WeakPtr demo ===
Foo(100) constructed
use_count after sp1: 1
...
Foo(100) destructed
```

---

## 📖 Example Usage

### UniquePtr

```cpp
auto up = smart_pointers::make_unique<int>(10);
assert(*up == 10);

smart_pointers::UniquePtr<int> up2 = std::move(up);
assert(!up);
assert(*up2 == 10);
```

### SharedPtr + WeakPtr

```cpp
auto sp1 = smart_pointers::make_shared<std::string>("hello");
assert(sp1.use_count() == 1);

{
    smart_pointers::SharedPtr<std::string> sp2 = sp1;
    assert(sp1.use_count() == 2);
}

assert(sp1.use_count() == 1);

smart_pointers::WeakPtr<std::string> wp(sp1);
auto locked = wp.lock();
assert(locked && *locked == "hello");
```

---

## 🧪 Testing

To run tests:

```bash
ctest --output-on-failure
```

Or directly:

```bash
./build/smart_pointers/smart_pointers_tests
```

Tests validate:

* move semantics
* exclusive ownership
* strong/weak count tracking
* object destruction timing
* `lock()` behavior

---


## 🛠 Future Enhancements

Possible next steps:

* thread-safe control block (`std::atomic`)
* intrusive pointer support
* aliasing constructors
* control block allocation optimization (`make_shared` trick)
* reference counting as `int vs size_t`
* debug hooks for tracking lifetimes

---

## 📜 License

MIT (or same as parent repo)

---
