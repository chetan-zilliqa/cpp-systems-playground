cmake_minimum_required(VERSION 3.16)
project(cpp_systems_playground CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(common)
add_subdirectory(memory_pool)
add_subdirectory(kv_store_chaining)
add_subdirectory(kv_store_linear)
add_subdirectory(in_memory_redis)
add_subdirectory(hash_map)
add_subdirectory(lru_cache)
add_subdirectory(smart_pointers)
add_subdirectory(lock_free_queue)
