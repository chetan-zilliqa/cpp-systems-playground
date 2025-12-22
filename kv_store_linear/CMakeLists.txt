cmake_minimum_required(VERSION 3.16)

add_library(kv_store_linear INTERFACE)
target_include_directories(kv_store_linear
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- tests executable ----
add_executable(kv_store_linear_tests tests/kvstore_linear_tests.cpp)
target_link_libraries(kv_store_linear_tests kv_store_linear gtest gtest_main)
add_test(NAME kv_store_linear_tests COMMAND kv_store_linear_tests)

# ---- demo executable ----
add_executable(kv_store_linear_demo src/kvstore_linear_demo.cpp)
target_link_libraries(kv_store_linear_demo kv_store_linear)

