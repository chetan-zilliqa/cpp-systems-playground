/**
 * Example: Combining Work-Stealing Thread Pool with Lock-Free Queue
 * 
 * This example demonstrates how to use a thread pool to process items
 * from a lock-free queue. This pattern is common in:
 * - Producer-consumer pipelines
 * - Real-time systems
 * - Event processing systems
 * 
 * SCENARIO:
 * - One producer thread enqueues work items to a lock-free queue
 * - Worker threads pull from the queue and process them
 * - Using futures to track completion and gather results
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <iomanip>

#include "thread_pool/work_stealing_thread_pool.hpp"
#include "lock_free_queue/spsc_queue.hpp"

// ============================================================================
// Example 1: Simple Producer-Consumer with Lock-Free Queue
// ============================================================================

void example_basic_producer_consumer() {
    std::cout << "\n=== Example 1: Basic Producer-Consumer ===\n";
    
    constexpr int queue_capacity = 100;
    lock_free::SPSCQueue<int, queue_capacity> queue;
    
    auto pool = std::make_unique<thread_pool::WorkStealingThreadPool>(2);
    
    std::atomic<int> processed_count{0};
    
    // Producer: enqueue work items
    std::thread producer([&queue]() {
        for (int i = 0; i < 50; ++i) {
            while (!queue.push(i * i)) {
                std::this_thread::yield();
            }
            std::cout << "Produced: " << i * i << "\n";
        }
    });
    
    // Consumer: process items via thread pool
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 20; ++i) {  // submit 20 consumer tasks
        auto fut = pool->submit([&queue, &processed_count]() {
            int item;
            while (queue.pop(item)) {
                processed_count++;
                std::cout << "  Consumed: " << item << " (total: " 
                          << processed_count << ")\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        futures.push_back(std::move(fut));
    }
    
    producer.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    for (auto& f : futures) {
        f.get();
    }
    
    std::cout << "✓ Processed " << processed_count << " items\n";
}

// ============================================================================
// Example 2: Worker Pool Processing Tasks from Queue
// ============================================================================

struct WorkItem {
    int id;
    int value;
    
    // Simulate processing
    int compute() const {
        int result = 0;
        for (int i = 0; i < value; ++i) {
            result += i;
        }
        return result;
    }
};

void example_worker_pool() {
    std::cout << "\n=== Example 2: Worker Pool Processing ===\n";
    
    constexpr int queue_capacity = 64;
    lock_free::SPSCQueue<WorkItem, queue_capacity> work_queue;
    
    auto pool = std::make_unique<thread_pool::WorkStealingThreadPool>(4);
    
    std::atomic<int> work_completed{0};
    std::vector<int> results(100);
    
    // Enqueue work items
    for (int i = 0; i < 100; ++i) {
        WorkItem w{i, 1000 + i};
        while (!work_queue.push(w)) {
            std::this_thread::yield();
        }
    }
    
    // Workers process items
    std::vector<std::future<void>> futures;
    for (int w = 0; w < 4; ++w) {
        auto fut = pool->submit([&work_queue, &work_completed, &results]() {
            WorkItem item;
            while (work_queue.pop(item)) {
                int result = item.compute();
                results[item.id] = result;
                work_completed++;
                
                if (work_completed % 25 == 0) {
                    std::cout << "  Completed: " << work_completed 
                              << " items\n";
                }
            }
        });
        futures.push_back(std::move(fut));
    }
    
    // Wait for all work to complete
    for (auto& f : futures) {
        f.get();
    }
    
    std::cout << "✓ Completed " << work_completed << " work items\n";
    std::cout << "  Sample results: [" << results[0] << ", " 
              << results[50] << ", " << results[99] << "]\n";
}

// ============================================================================
// Example 3: Pipeline - Queue Output Feeds Thread Pool
// ============================================================================

void example_pipeline() {
    std::cout << "\n=== Example 3: Pipeline Processing ===\n";
    
    // Stage 1: Initial data generation
    constexpr int pipeline_capacity = 32;
    lock_free::SPSCQueue<int, pipeline_capacity> stage1_queue;
    lock_free::SPSCQueue<int, pipeline_capacity> stage2_queue;
    
    auto pool = std::make_unique<thread_pool::WorkStealingThreadPool>(3);
    
    // Stage 1: Generator -> stage1_queue
    std::thread generator([&stage1_queue]() {
        for (int i = 0; i < 30; ++i) {
            int data = i * i;
            while (!stage1_queue.push(data)) {
                std::this_thread::yield();
            }
        }
    });
    
    // Stage 2: Process from stage1 -> stage2
    auto stage2_worker = pool->submit([&stage1_queue, &stage2_queue]() {
        int data;
        while (stage1_queue.pop(data)) {
            int processed = data * 2;  // simple transformation
            while (!stage2_queue.push(processed)) {
                std::this_thread::yield();
            }
        }
    });
    
    // Stage 3: Process from stage2 (final)
    auto stage3_worker = pool->submit([&stage2_queue]() {
        int data;
        int count = 0;
        while (stage2_queue.pop(data)) {
            count++;
            if (count % 10 == 0) {
                std::cout << "  Pipeline processed: " << count << " items\n";
            }
        }
        std::cout << "  Pipeline final: processed " << count << " items total\n";
    });
    
    generator.join();
    stage2_worker.get();
    stage3_worker.get();
    
    std::cout << "✓ Pipeline complete\n";
}

// ============================================================================
// Example 4: Demonstrating Wait-Free Properties
// ============================================================================

void example_wait_free_demo() {
    std::cout << "\n=== Example 4: Wait-Free Queue Demo ===\n";
    
    constexpr int queue_capacity = 1000;
    lock_free::SPSCQueue<int, queue_capacity> queue;
    
    std::atomic<int> total_pushed{0};
    std::atomic<int> total_popped{0};
    
    // Producer thread
    std::thread producer([&queue, &total_pushed]() {
        for (int i = 0; i < 500; ++i) {
            if (queue.push(i)) {
                total_pushed++;
            }
        }
    });
    
    // Consumer thread
    std::thread consumer([&queue, &total_popped]() {
        int item;
        while (total_popped < 500) {
            if (queue.pop(item)) {
                total_popped++;
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "  Pushed: " << total_pushed << "\n";
    std::cout << "  Popped: " << total_popped << "\n";
    std::cout << "✓ Lock-free operation complete (no blocking, no locks)\n";
}

// ============================================================================
// Example 5: Real-world Scenario - Task Batch Processing
// ============================================================================

struct Task {
    int task_id;
    std::string description;
    int complexity;
    
    int execute() const {
        std::cout << "  [Task " << task_id << "] " << description 
                  << " (complexity: " << complexity << ")\n";
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return task_id;
    }
};

void example_batch_processing() {
    std::cout << "\n=== Example 5: Real-World Batch Processing ===\n";
    
    constexpr int queue_capacity = 128;
    lock_free::SPSCQueue<Task, queue_capacity> task_queue;
    
    auto pool = std::make_unique<thread_pool::WorkStealingThreadPool>(4);
    
    // Enqueue batch of tasks
    std::vector<Task> tasks = {
        {1, "Data validation", 2},
        {2, "Transform JSON", 3},
        {3, "Compute hash", 1},
        {4, "Compress data", 4},
        {5, "Send to API", 2},
        {6, "Log results", 1},
        {7, "Update cache", 2},
        {8, "Notify users", 3},
    };
    
    for (const auto& t : tasks) {
        while (!task_queue.push(t)) {
            std::this_thread::yield();
        }
    }
    
    // Submit worker that processes task queue
    auto worker = pool->submit([&task_queue]() {
        Task t;
        while (task_queue.pop(t)) {
            t.execute();
        }
    });
    
    worker.get();
    
    std::cout << "✓ Batch of " << tasks.size() << " tasks processed\n";
}

int main() {
    std::cout << "\n" << std::string(70, '=');
    std::cout << "\nThread Pool + Lock-Free Queue Examples\n";
    std::cout << std::string(70, '=') << "\n";
    
    try {
        example_basic_producer_consumer();
        example_worker_pool();
        example_pipeline();
        example_wait_free_demo();
        example_batch_processing();
        
        std::cout << "\n" << std::string(70, '=');
        std::cout << "\n✅ All examples completed successfully\n";
        std::cout << std::string(70, '=') << "\n\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << "\n\n";
        return 1;
    }
}
