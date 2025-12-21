cmake_minimum_required(VERSION 3.16)

add_library(kv_store_optimized INTERFACE)
target_include_directories(kv_store_optimized 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- tests executable ----
add_executable(kv_store_optimized_tests tests/kvstore_optimized_tests.cpp)
target_link_libraries(kv_store_optimized_tests kv_store_optimized gtest gtest_main)
add_test(NAME kv_store_optimized_tests COMMAND kv_store_optimized_tests)

# ---- demo executable ----
add_executable(kv_store_optimized_demo src/kvstore_optimized_demo.cpp)
target_link_libraries(kv_store_optimized_demo kv_store_optimized)

