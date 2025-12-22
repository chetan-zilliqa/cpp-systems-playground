#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "thread_pool/work_stealing_thread_pool.hpp"

int main()
{
    using namespace std::chrono_literals;

    thread_pool::WorkStealingThreadPool pool(std::thread::hardware_concurrency());

    std::cout << "Work-stealing thread pool demo with "
              << pool.thread_count() << " threads\n";

    constexpr int N = 16;

    std::vector<std::future<int>> futures;
    futures.reserve(N);

    for (int i = 0; i < N; ++i) {
        futures.push_back(pool.submit([i] {
            // Simulate some work
            std::this_thread::sleep_for(50ms);
            int result = i * i;
            std::cout << "Task " << i << " computed " << result
                      << " on thread " << std::this_thread::get_id() << "\n";
            return result;
        }));
    }

    int sum = 0;
    for (auto& f : futures) {
        sum += f.get();
    }

    std::cout << "Sum of squares [0.." << (N - 1) << "] = " << sum << "\n";
    std::cout << "Done.\n";

    return 0;
}

