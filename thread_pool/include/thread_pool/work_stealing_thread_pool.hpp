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

class WorkStealingThreadPool {
public:
    using Task = std::function<void()>;

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

    [[nodiscard]] std::size_t thread_count() const noexcept
    {
        return workers_.size();
    }

private:
    struct WorkerQueue {
        std::deque<Task> tasks;
        std::mutex       mutex;
    };

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

        global_cv_.notify_one();
    }

    void worker_loop(std::size_t index)
    {
        while (true) {
            Task task;

            if (try_pop_local(index, task) || try_steal(index, task)) {
                task();
                continue;
            }

            std::unique_lock<std::mutex> lk(global_mutex_);
            global_cv_.wait(lk, [this] {
                return stop_.load(std::memory_order_relaxed) || has_work();
            });

            if (stop_.load(std::memory_order_relaxed) && !has_work()) {
                break;
            }
        }
    }

    bool try_pop_local(std::size_t index, Task& out)
    {
        auto& w = *workers_[index];
        std::lock_guard<std::mutex> lk(w.mutex);
        if (w.tasks.empty()) {
            return false;
        }
        out = std::move(w.tasks.back());
        w.tasks.pop_back();
        return true;
    }

    bool try_steal(std::size_t self_index, Task& out)
    {
        const std::size_t n = workers_.size();
        for (std::size_t offset = 1; offset < n; ++offset) {
            const std::size_t victim = (self_index + offset) % n;
            auto& w = *workers_[victim];

            std::lock_guard<std::mutex> lk(w.mutex);
            if (!w.tasks.empty()) {
                out = std::move(w.tasks.front());
                w.tasks.pop_front();
                return true;
            }
        }
        return false;
    }

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

    // data
    std::vector<std::unique_ptr<WorkerQueue>> workers_;
    std::vector<std::thread>                  threads_;

    std::atomic<bool>        stop_;
    std::atomic<std::size_t> next_worker_{0};

    std::mutex              global_mutex_;
    std::condition_variable global_cv_;
};

} // namespace thread_pool

