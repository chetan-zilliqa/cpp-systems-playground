#include <chrono>
#include <iostream>
#include <thread>

#include "lock_free_queue/spsc_queue.hpp"

int main() {
    using namespace std::chrono_literals;

    lock_free::SPSCQueue<int, 1024> queue;

    constexpr int N = 10000;

    std::thread producer([&] {
        for (int i = 0; i < N; ++i) {
            // spin until space is available
            while (!queue.push(i)) {
                // backoff
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&] {
        int expected = 0;
        int value    = 0;
        int count    = 0;

        while (count < N) {
            if (queue.pop(value)) {
                if (value != expected) {
                    std::cerr << "Out of order: got " << value
                              << ", expected " << expected << "\n";
                }
                ++expected;
                ++count;
            } else {
                std::this_thread::yield();
            }
        }

        std::cout << "Consumed " << count << " items successfully.\n";
    });

    producer.join();
    consumer.join();

    return 0;
}

