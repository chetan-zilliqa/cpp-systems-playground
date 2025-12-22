cmake_minimum_required(VERSION 3.16)

add_library(smart_pointers INTERFACE)

target_include_directories(smart_pointers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(smart_pointers INTERFACE cxx_std_20)

# Demo
add_executable(smart_pointers_demo
    src/main.cpp
)

target_link_libraries(smart_pointers_demo
    PRIVATE smart_pointers
)

# Tests
add_executable(smart_pointers_tests
    tests/smart_pointers_tests.cpp
)

target_link_libraries(smart_pointers_tests
    PRIVATE smart_pointers
)

add_test(NAME smart_pointers_tests COMMAND smart_pointers_tests)

