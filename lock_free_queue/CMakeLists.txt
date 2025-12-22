cmake_minimum_required(VERSION 3.16)

add_library(spsc_queue INTERFACE)

target_include_directories(spsc_queue
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(spsc_queue INTERFACE cxx_std_20)

# Demo
add_executable(spsc_queue_demo
    src/spsc_queue_demo.cpp
)

target_link_libraries(spsc_queue_demo
    PRIVATE spsc_queue
)

# Tests (simple assert-based)
add_executable(spsc_queue_tests
    tests/spsc_queue_tests.cpp
)

target_link_libraries(spsc_queue_tests
    PRIVATE spsc_queue
)

add_test(NAME spsc_queue_tests COMMAND spsc_queue_tests)

