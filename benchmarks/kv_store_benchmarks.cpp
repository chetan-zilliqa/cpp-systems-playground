#include <iostream>
#include <chrono>
#include <iomanip>
#include <random>
#include "kv_store_chaining/kv_store_chaining.hpp"
#include "kv_store_linear/kvstore_linear.hpp"

class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}
    
    double elapsed_ms() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
        return static_cast<double>(duration.count());
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_;
};

void benchmark_kv_stores() {
    std::cout << "\n--- KV Store Comparison (Chaining vs Linear) ---\n";
    
    const int sizes[] = {1000, 10000, 100000};
    
    for (int size : sizes) {
        // Benchmark chaining store
        {
            kv_store_chaining::KVStoreChainingLib<int, std::string> store;
            
            Timer timer;
            for (int i = 0; i < size; ++i) {
                store.put(i, "value_" + std::to_string(i));
            }
            double insert_time = timer.elapsed_ms();
            
            std::mt19937 rng(42);
            std::uniform_int_distribution<int> dist(0, size - 1);
            
            Timer lookup_timer;
            for (int i = 0; i < size; ++i) {
                int key = dist(rng);
                auto val = store.get(key);
                (void)val;
            }
            double lookup_time = lookup_timer.elapsed_ms();
            
            std::cout << "Chaining  " << std::setw(7) << size << " items: "
                      << "Insert " << std::fixed << std::setprecision(2)
                      << std::setw(8) << insert_time << " ms, "
                      << "Lookup " << std::setw(8) << lookup_time << " ms\n";
        }
        
        // Benchmark linear store
        {
            kv_store_linear::KVStoreLinear<int, std::string> store(size);
            
            Timer timer;
            for (int i = 0; i < size; ++i) {
                store.insert(i, "value_" + std::to_string(i));
            }
            double insert_time = timer.elapsed_ms();
            
            std::mt19937 rng(42);
            std::uniform_int_distribution<int> dist(0, size - 1);
            
            Timer lookup_timer;
            for (int i = 0; i < size; ++i) {
                int key = dist(rng);
                auto val = store.find(key);
                (void)val;
            }
            double lookup_time = lookup_timer.elapsed_ms();
            
            std::cout << "Linear    " << std::setw(7) << size << " items: "
                      << "Insert " << std::fixed << std::setprecision(2)
                      << std::setw(8) << insert_time << " ms, "
                      << "Lookup " << std::setw(8) << lookup_time << " ms\n";
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "\n========================================\n";
    std::cout << "  KV Store Benchmarks\n";
    std::cout << "========================================\n";
    
    benchmark_kv_stores();
    
    std::cout << "\n========================================\n";
    std::cout << "  Benchmark Complete\n";
    std::cout << "========================================\n\n";
    
    return 0;
}
