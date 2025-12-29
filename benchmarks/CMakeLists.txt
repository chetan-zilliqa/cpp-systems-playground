cmake_minimum_required(VERSION 3.16)

add_executable(benchmarks
    hash_map_benchmarks.cpp
    kv_store_benchmarks.cpp
)

target_link_libraries(benchmarks
    PRIVATE
        hash_map
        kv_store_chaining_lib
        kv_store_linear
        memory_pool_lib
        common
)

# Optional: Could add Google Benchmark dependency if desired
# For now, we use simple chrono-based timing
