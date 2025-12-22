cmake_minimum_required(VERSION 3.16)

# Create a header-only library target
add_library(hash_map INTERFACE)

# Expose include directory to dependents
target_include_directories(hash_map
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Require modern C++ standard (you can switch to 23)
target_compile_features(hash_map INTERFACE cxx_std_20)

# ------------- Demo executable -------------
add_executable(hash_map_demo
    src/main.cpp
)

target_link_libraries(hash_map_demo
    PRIVATE hash_map
)

# ------------- Test executable -------------
add_executable(hash_map_tests
    tests/hash_map_tests.cpp
)

target_link_libraries(hash_map_tests
    PRIVATE hash_map
)

# Register the test with CTest
add_test(NAME hash_map_tests COMMAND hash_map_tests)

