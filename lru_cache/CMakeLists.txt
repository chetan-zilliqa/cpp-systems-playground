cmake_minimum_required(VERSION 3.14)

# Header-only library
add_library(lru_cache INTERFACE)

target_include_directories(lru_cache
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# hash_map is defined in hash_map/CMakeLists.txt
target_link_libraries(lru_cache
    INTERFACE
        hash_map
)

# Demo executable
add_executable(lru_cache_demo
    src/main.cpp
)

target_link_libraries(lru_cache_demo
    PRIVATE lru_cache
)
# Tests
add_executable(lru_cache_tests
    tests/lru_cache_tests.cpp
)

target_link_libraries(lru_cache_tests
    PRIVATE lru_cache
)

add_test(NAME lru_cache_tests COMMAND lru_cache_tests)
