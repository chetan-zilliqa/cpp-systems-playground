cmake_minimum_required(VERSION 3.16)

add_library(common INTERFACE)

target_include_directories(common
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Unit tests
add_executable(common_tests
    tests/common_logging_tests.cpp
)

target_link_libraries(common_tests
    PRIVATE
        common
)

add_test(NAME common_tests COMMAND common_tests)
