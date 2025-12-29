cmake_minimum_required(VERSION 3.16)

add_executable(thread_pool_lockfree_example
    thread_pool_lockfree_example.cpp
)

target_link_libraries(thread_pool_lockfree_example
    PRIVATE
        thread_pool
        spsc_queue
        common
)
