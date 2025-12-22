# Work-Stealing Thread Pool (C++)

`thread_pool` is a modern C++ implementation of a **work-stealing thread pool**.

Each worker thread owns its own task deque and:

* runs tasks from its **own queue** (LIFO – better cache locality)
* when idle, **steals** tasks from other workers’ queues (FIFO – fairer distribution)

This module is meant as a practical, readable implementation of task-based parallelism and basic work stealing.

---

## ✨ Features

| Feature           | Description                                     |
| ----------------- | ----------------------------------------------- |
| Work-stealing     | Idle workers steal tasks from others            |
| Task-based API    | `submit(f, args...) → std::future<R>`           |
| RAII              | Threads start in ctor and join in dtor          |
| Configurable size | Custom thread count or `hardware_concurrency()` |
| Per-worker queues | Reduced contention vs single global queue       |
| Futures & tasks   | Uses `std::packaged_task` + `std::future`       |
| Modern C++        | `std::invoke_result_t`, lambdas, move semantics |

This is intentionally **not** a full-blown production scheduler, but the code structure mirrors real-world thread pool designs found in runtimes and servers.

---

## 📁 Directory Structure

```text
thread_pool/
  include/
    thread_pool/
      work_stealing_thread_pool.hpp
  src/
    work_stealing_thread_pool_demo.cpp
  tests/
    work_stealing_thread_pool_tests.cpp
  CMakeLists.txt
  README.md
```

---

## 🧠 Design Overview

### 1️⃣ Core idea

* `N` worker threads.

* Each worker has:

  ```cpp
  struct WorkerQueue {
      std::deque<Task> tasks;
      std::mutex       mutex;
  };
  ```

* When you call `submit(...)`, the task is assigned to one worker (round-robin).

### 2️⃣ Execution logic

For each worker thread:

1. Try to pop from its own deque (back → LIFO).
2. If empty, try to **steal** from other workers (front → FIFO).
3. If no work anywhere, wait on a global `std::condition_variable`.
4. On shutdown, all threads are joined in the pool destructor.

### 3️⃣ Task type

```cpp
using Task = std::function<void()>;
```

Tasks are stored as `std::function`, constructed from `std::packaged_task<R()>` held via `std::shared_ptr`, so they remain copyable.

---

## 🧾 Public API

```cpp
namespace thread_pool {

class WorkStealingThreadPool {
public:
    explicit WorkStealingThreadPool(
        std::size_t thread_count = std::thread::hardware_concurrency());

    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    [[nodiscard]] std::size_t thread_count() const noexcept;
};

} // namespace thread_pool
```

Usage:

```cpp
thread_pool::WorkStealingThreadPool pool(4);
auto fut = pool.submit([](int x) { return x * x; }, 7);
int r = fut.get(); // 49
```

---

## 🚀 Build & Run

From repo root:

```bash
cmake -S . -B build
cmake --build build --target thread_pool_demo -j
./build/thread_pool/thread_pool_demo
```

Example output (shape):

```text
Work-stealing thread pool demo with 8 threads
Task 0 computed 0 on thread ...
Task 1 computed 1 on thread ...
...
Sum of squares [0..15] = 1240
Done.
```

---

## 🧪 Unit Tests

Tests are simple, assert-based executables (no external testing framework).

Build & run:

```bash
cmake --build build --target thread_pool_tests -j
./build/thread_pool/thread_pool_tests
```

Tests cover:

* summation with many tasks
* atomic increments across many tasks
* basic correctness of submit + futures

---

## 🎯 Learning Goals

This module is designed to illustrate:

* how to structure a thread pool
* task submission using `std::packaged_task` and `std::future`
* work-stealing between per-thread queues
* graceful shutdown with condition variables
* separation of **task scheduling** vs **task execution**

It’s a good stepping stone to:

* lock-free work-stealing deques
* actor-style schedulers
* async runtimes / executors

---

## 🚧 Future Enhancements

Possible next steps:

* priorities (high/low priority queues)
* delayed / scheduled tasks (run at time T)
* cooperative cancellation / stop tokens
* per-thread local submission API
* metrics (queue length, tasks processed, etc.)
* MPMC queue variant (no per-worker queues)

---

## 📜 License

MIT (or same as parent repo).

---
