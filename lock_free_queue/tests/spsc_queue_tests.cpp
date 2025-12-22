#include "lock_free_queue/spsc_queue.hpp"

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

using lock_free::SPSCQueue;

static void test_single_thread_basic() {
    SPSCQueue<int, 4> q; // capacity 4

    assert(q.empty());
    assert(!q.full());

    // Push a few values
    assert(q.push(1));
    assert(q.push(2));
    assert(q.push(3));

    assert(!q.empty());

    int x = 0;
    assert(q.pop(x) && x == 1);
    assert(q.pop(x) && x == 2);
    assert(q.pop(x) && x == 3);

    assert(!q.pop(x)); // now empty
    assert(q.empty());
}

static void test_single_thread_full() {
    SPSCQueue<int, 2> q; // capacity 2

    assert(q.push(10));
    assert(q.push(20));
    assert(q.full());

    // Cannot push when full
    assert(!q.push(30));

    int x = 0;
    assert(q.pop(x) && x == 10);
    assert(q.pop(x) && x == 20);
    assert(q.empty());
}

static void test_two_thread_spsc() {
    constexpr int N = 10000;
    SPSCQueue<int, 1024> q;

    std::thread producer([&] {
        for (int i = 0; i < N; ++i) {
            while (!q.push(i)) {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&] {
        int expected = 0;
        int x        = 0;
        int count    = 0;

        while (count < N) {
            if (q.pop(x)) {
                assert(x == expected);
                ++expected;
                ++count;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();
}

int main() {
    std::cout << "Running lock_free_queue tests...\n";

    test_single_thread_basic();
    test_single_thread_full();
    test_two_thread_spsc();

    std::cout << "All lock_free_queue tests passed.\n";
    return 0;
}

