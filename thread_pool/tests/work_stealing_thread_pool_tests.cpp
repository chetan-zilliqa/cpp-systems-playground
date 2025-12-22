#include "thread_pool/work_stealing_thread_pool.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using thread_pool::WorkStealingThreadPool;

static void test_basic_sum()
{
    WorkStealingThreadPool pool(4);

    constexpr int N = 100;
    std::vector<std::future<int>> futures;
    futures.reserve(N);

    for (int i = 0; i < N; ++i) {
        futures.push_back(pool.submit([i] { return i; }));
    }

    int sum = 0;
    for (auto& f : futures) {
        sum += f.get();
    }

    // Sum 0..99 = 4950
    assert(sum == (N - 1) * N / 2);
}

static void test_parallel_increment()
{
    WorkStealingThreadPool pool(4);

    std::atomic<int> counter{0};
    constexpr int N = 1000;

    std::vector<std::future<void>> futures;
    futures.reserve(N);

    for (int i = 0; i < N; ++i) {
        futures.push_back(pool.submit([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    assert(counter.load(std::memory_order_relaxed) == N);
}

int main()
{
    std::cout << "Running thread_pool tests...\n";

    test_basic_sum();
    test_parallel_increment();

    std::cout << "All thread_pool tests passed.\n";
    return 0;
}

