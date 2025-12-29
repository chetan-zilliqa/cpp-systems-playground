#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace thread_pool {

/**
 * Work-stealing thread pool with per-thread task queues and futures.
 * 
 * DESIGN OVERVIEW:
 * - Each worker thread has its own deque of tasks (task affinity)
 * - Work stealing: idle workers attempt to steal tasks from others
 * - Global condition variable for signaling work availability
 * 
 * SCHEDULING POLICY:
 * - When a task is submitted, it's enqueued to a worker in round-robin
 * - Worker executes from local queue (LIFO: pop from back)
 * - When local queue empty, steal from other workers (FIFO: pop from front)
 * 
 * THREAD SAFETY:
 * - Per-queue mutex: protects individual task deques
 * - Global mutex + condition variable: for shutdown signaling
 * - Atomic next_worker_: tracks round-robin submission index
 * - No lock-free algorithms here (favor clarity over extreme performance)
 * 
 * FUTURES:
 * - std::packaged_task wraps user functions
 * - Returns std::future immediately
 * - Blocked calling thread can get_future() to wait
 * 
 * TERMINATION:
 * - Destructor sets stop flag and notifies all
 * - Threads drain remaining work before exiting
 * 
 * EXAMPLE:
 *   auto pool = std::make_unique<WorkStealingThreadPool>(4);
 *   auto fut = pool->submit([]() { return 42; });
 *   int result = fut.get();  // blocks until ready
 */
class WorkStealingThreadPool {
public:
    using Task = std::function<void()>;

    /**
     * Construct with specified thread count.
     * If thread_count is 0, defaults to hardware_concurrency().
     * Each thread gets its own work-stealing deque.
     */
    explicit WorkStealingThreadPool(std::size_t thread_count =
        std::thread::hardware_concurrency())
        : stop_(false)
    {
        if (thread_count == 0) {
            thread_count = 1;
        }

        workers_.reserve(thread_count);
        for (std::size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back(std::make_unique<WorkerQueue>());
        }

        next_worker_.store(0, std::memory_order_relaxed);

        // Launch worker threads
        for (std::size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back([this, i] { worker_loop(i); });
        }
    }

    ~WorkStealingThreadPool()
    {
        {
            std::lock_guard<std::mutex> lk(global_mutex_);
            stop_.store(true, std::memory_order_relaxed);
        }
        global_cv_.notify_all();

        for (auto& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    WorkStealingThreadPool(const WorkStealingThreadPool&)            = delete;
    WorkStealingThreadPool& operator=(const WorkStealingThreadPool&) = delete;
    WorkStealingThreadPool(WorkStealingThreadPool&&)                 = delete;
    WorkStealingThreadPool& operator=(WorkStealingThreadPool&&)      = delete;

    /**
     * Submit a callable with arguments.
     * Returns a future that can be used to wait for and retrieve the result.
     * 
     * USAGE:
     *   auto fut = pool->submit(myfunc, arg1, arg2);
     *   auto result = fut.get();  // blocks until ready
     * 
     * THREAD SAFETY: Safe to call from any thread.
     * BLOCKING: Never blocks; task is queued immediately.
     */
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using R = std::invoke_result_t<F, Args...>;

        auto task_ptr =
            std::make_shared<std::packaged_task<R()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<R> fut = task_ptr->get_future();

        Task wrapper = [task_ptr]() { (*task_ptr)(); };

        enqueue_task(std::move(wrapper));
        return fut;
    }

    /**
     * Returns the number of worker threads in this pool.
     */
    [[nodiscard]] std::size_t thread_count() const noexcept
    {
        return workers_.size();
    }

private:
    // Per-worker task queue with associated mutex
    struct WorkerQueue {
        std::deque<Task> tasks;  // task deque (supports pop_back and pop_front)
        std::mutex       mutex;  // protects tasks list
    };

    /**
     * Enqueue a task to a worker thread (round-robin selection).
     * next_worker_ is incremented in atomic fashion to distribute load.
     * Only uses relaxed memory order since this is just scheduling.
     */
    void enqueue_task(Task task)
    {
        const std::size_t n = workers_.size();
        const std::size_t idx =
            next_worker_.fetch_add(1, std::memory_order_relaxed) % n;

        {
            auto& w = *workers_[idx];
            std::lock_guard<std::mutex> lk(w.mutex);
            w.tasks.emplace_back(std::move(task));
        }

        global_cv_.notify_one();  // wake one sleeping worker
    }

    /**
     * Main worker loop (runs in each worker thread).
     * 
     * ALGORITHM:
     * 1. Try pop from local queue (LIFO: back)
     * 2. If empty, try steal from other queues (FIFO: front)
     * 3. If nothing to do, wait on condition variable
     * 4. Repeat until shutdown
     * 
     * WHY LIFO for local, FIFO for stealing:
     * - LIFO (own queue): prefers recently-submitted (cache-hot) tasks
     * - FIFO (stolen queue): balances load across workers
     */
    void worker_loop(std::size_t index)
    {
        while (true) {
            Task task;

            // Try local queue first, then steal
            if (try_pop_local(index, task) || try_steal(index, task)) {
                task();
                continue;
            }

            // No work found, wait for notification
            std::unique_lock<std::mutex> lk(global_mutex_);
            global_cv_.wait(lk, [this] {
                return stop_.load(std::memory_order_relaxed) || has_work();
            });

            if (stop_.load(std::memory_order_relaxed) && !has_work()) {
                break;  // shutdown and no remaining work
            }
        }
    }

    /**
     * Attempt to pop a task from this worker's local queue (LIFO).
     * Returns true and fills 'out' if task was found.
     */
    bool try_pop_local(std::size_t index, Task& out)
    {
        auto& w = *workers_[index];
        std::lock_guard<std::mutex> lk(w.mutex);
        if (w.tasks.empty()) {
            return false;
        }
        out = std::move(w.tasks.back());  // pop from back (LIFO)
        w.tasks.pop_back();
        return true;
    }

    /**
     * Attempt to steal a task from another worker's queue (FIFO).
     * Tries all other workers in order starting from next victim.
     * Returns true and fills 'out' if a task was stolen.
     */
    bool try_steal(std::size_t self_index, Task& out)
    {
        const std::size_t n = workers_.size();
        for (std::size_t offset = 1; offset < n; ++offset) {
            const std::size_t victim = (self_index + offset) % n;
            auto& w = *workers_[victim];

            std::lock_guard<std::mutex> lk(w.mutex);
            if (!w.tasks.empty()) {
                out = std::move(w.tasks.front());  // pop from front (FIFO)
                w.tasks.pop_front();
                return true;
            }
        }
        return false;
    }

    /**
     * Check if any worker has pending tasks.
     * Used in condition variable predicate to wake sleeping workers.
     * Iterates all queues, so O(n) but only called during waits.
     */
    bool has_work()
    {
        for (auto& wptr : workers_) {
            auto& w = *wptr;
            std::lock_guard<std::mutex> lk(w.mutex);
            if (!w.tasks.empty()) {
                return true;
            }
        }
        return false;
    }

    // Data members
    std::vector<std::unique_ptr<WorkerQueue>> workers_;  // per-thread task queues
    std::vector<std::thread>                  threads_;  // worker threads

    std::atomic<bool>        stop_;          // shutdown flag
    std::atomic<std::size_t> next_worker_{0};  // round-robin submission index

    std::mutex              global_mutex_;   // protects has_work() checks
    std::condition_variable global_cv_;      // notifies idle workers of new work
};

} // namespace thread_pool

