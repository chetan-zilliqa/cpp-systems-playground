cmake_minimum_required(VERSION 3.16)

add_library(kv_store_lib
    src/kv_store.cpp
)

target_include_directories(kv_store_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(kv_store_lib
    PUBLIC
        common
        memory_pool_lib
)

add_executable(kv_store_demo
    src/main.cpp
)

target_link_libraries(kv_store_demo
    PRIVATE
        kv_store_lib
        common
)

# Unit tests
add_executable(kv_store_tests
    tests/kv_store_tests.cpp
)

target_link_libraries(kv_store_tests
    PRIVATE
        kv_store_lib
        common
)

add_test(NAME kv_store_tests COMMAND kv_store_tests)