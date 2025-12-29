#include <iostream>
#include <chrono>
#include <iomanip>
#include <random>
#include "hash_map/hash_map.hpp"

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

void benchmark_hashmap_insert() {
    std::cout << "\n--- HashMap Insert Benchmark ---\n";
    
    const int sizes[] = {1000, 10000, 100000, 1000000};
    
    for (int size : sizes) {
        hash_map::HashMap<int, int> map;
        
        Timer timer;
        for (int i = 0; i < size; ++i) {
            map.insert_or_assign(i, i * 2);
        }
        double elapsed = timer.elapsed_ms();
        
        double ops_per_sec = (size / elapsed) * 1000.0;
        std::cout << "Insert " << std::setw(7) << size << " items: "
                  << std::fixed << std::setprecision(2)
                  << elapsed << " ms | " << ops_per_sec << " ops/sec\n";
    }
}

void benchmark_hashmap_lookup() {
    std::cout << "\n--- HashMap Lookup Benchmark ---\n";
    
    const int sizes[] = {1000, 10000, 100000, 1000000};
    
    for (int size : sizes) {
        hash_map::HashMap<int, int> map;
        
        // Populate
        for (int i = 0; i < size; ++i) {
            map.insert_or_assign(i, i * 2);
        }
        
        // Benchmark lookups (random access)
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, size - 1);
        
        Timer timer;
        int lookups = size * 10;
        for (int i = 0; i < lookups; ++i) {
            int key = dist(rng);
            auto val = map.get(key);
            (void)val;
        }
        double elapsed = timer.elapsed_ms();
        
        double ops_per_sec = (lookups / elapsed) * 1000.0;
        std::cout << "Lookup " << std::setw(7) << lookups << " times: "
                  << std::fixed << std::setprecision(2)
                  << elapsed << " ms | " << ops_per_sec << " ops/sec\n";
    }
}

void benchmark_hashmap_mixed() {
    std::cout << "\n--- HashMap Mixed Operations Benchmark ---\n";
    
    const int sizes[] = {1000, 10000, 100000};
    
    for (int size : sizes) {
        hash_map::HashMap<int, int> map;
        
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, size - 1);
        std::uniform_int_distribution<int> op_type(0, 99);
        
        Timer timer;
        int total_ops = size * 5;
        
        for (int i = 0; i < total_ops; ++i) {
            int op = op_type(rng);
            int key = dist(rng);
            
            if (op < 40) {  // 40% inserts
                map.insert_or_assign(key, key * 2);
            } else if (op < 80) {  // 40% lookups
                auto val = map.get(key);
                (void)val;
            } else {  // 20% erases
                map.erase(key);
            }
        }
        double elapsed = timer.elapsed_ms();
        
        double ops_per_sec = (total_ops / elapsed) * 1000.0;
        std::cout << "Mixed ops (" << std::setw(7) << total_ops << " total): "
                  << std::fixed << std::setprecision(2)
                  << elapsed << " ms | " << ops_per_sec << " ops/sec\n";
    }
}

void benchmark_load_factor() {
    std::cout << "\n--- Load Factor Impact on Insertion ---\n";
    
    double load_factors[] = {0.5, 0.75, 0.9};
    int size = 100000;
    
    for (double lf : load_factors) {
        hash_map::HashMap<int, int> map(16);
        map.max_load_factor(lf);
        
        Timer timer;
        for (int i = 0; i < size; ++i) {
            map.insert_or_assign(i, i * 2);
        }
        double elapsed = timer.elapsed_ms();
        
        std::cout << "Load factor " << std::fixed << std::setprecision(2) << lf << ": "
                  << elapsed << " ms (bucket count: " << map.bucket_count() << ")\n";
    }
}

int main() {
    std::cout << "\n========================================\n";
    std::cout << "  C++ Systems Playground - Benchmarks\n";
    std::cout << "========================================\n";
    
    benchmark_hashmap_insert();
    benchmark_hashmap_lookup();
    benchmark_hashmap_mixed();
    benchmark_load_factor();
    
    std::cout << "\n========================================\n";
    std::cout << "  Benchmark Complete\n";
    std::cout << "========================================\n\n";
    
    return 0;
}
