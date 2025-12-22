cmake_minimum_required(VERSION 3.16)

# Library: in_memory_redis
add_library(in_memory_redis
    src/redis.cpp
)

target_include_directories(in_memory_redis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# Threads (for std::thread)
find_package(Threads REQUIRED)
target_link_libraries(in_memory_redis PUBLIC Threads::Threads)

# Demo executable
add_executable(in_memory_redis_demo
    src/main.cpp
)

target_link_libraries(in_memory_redis_demo
    PRIVATE in_memory_redis
)

