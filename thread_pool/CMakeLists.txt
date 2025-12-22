cmake_minimum_required(VERSION 3.16)

add_library(thread_pool INTERFACE)

target_include_directories(thread_pool
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(thread_pool INTERFACE cxx_std_20)

# Demo
add_executable(thread_pool_demo
    src/work_stealing_thread_pool_demo.cpp
)

target_link_libraries(thread_pool_demo
    PRIVATE thread_pool
)

# Tests
add_executable(thread_pool_tests
    tests/work_stealing_thread_pool_tests.cpp
)

target_link_libraries(thread_pool_tests
    PRIVATE thread_pool
)

add_test(NAME thread_pool_tests COMMAND thread_pool_tests)

