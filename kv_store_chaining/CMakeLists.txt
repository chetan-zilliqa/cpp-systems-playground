cmake_minimum_required(VERSION 3.16)

add_library(kv_store_chaining_lib
    src/kv_store_chaining.cpp
)

target_include_directories(kv_store_chaining_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(kv_store_chaining_lib
    PUBLIC
        common
        memory_pool_lib
)

add_executable(kv_store_chaining_demo
    src/kv_store_chaining_demo.cpp
)

target_link_libraries(kv_store_chaining_demo
    PRIVATE
        kv_store_chaining_lib
        common
)

# Unit tests
add_executable(kv_store_chaining_tests
    tests/kv_store_chaining_tests.cpp
)

target_link_libraries(kv_store_chaining_tests
    PRIVATE
        kv_store_chaining_lib
        common
)

add_test(NAME kv_store_chaining_tests COMMAND kv_store_chaining_tests)
