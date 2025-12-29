cmake_minimum_required(VERSION 3.16)

# Integration tests - cross-module scenarios

add_executable(integration_tests
    integration_tests.cpp
)

target_link_libraries(integration_tests
    PRIVATE
        hash_map
        memory_pool_lib
        kv_store_chaining_lib
        lru_cache
        thread_pool
        spsc_queue
        common
)

add_test(NAME integration_tests COMMAND integration_tests)
