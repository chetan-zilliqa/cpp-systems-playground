cmake_minimum_required(VERSION 3.16)

add_library(memory_pool_lib INTERFACE)

target_include_directories(memory_pool_lib
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(memory_pool_lib
    INTERFACE
        common
)

add_executable(memory_pool_demo
    src/main.cpp
)

target_link_libraries(memory_pool_demo
    PRIVATE
        memory_pool_lib
        common
)

