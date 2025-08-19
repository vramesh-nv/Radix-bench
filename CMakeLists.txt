cmake_minimum_required(VERSION 3.10)
project(RadixTreeBenchmark)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Enable optimization for benchmarks
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Create static library for radix tree
add_library(radix_tree STATIC
    radix.c
    radix.h
)

# Create static library for wide radix tree
add_library(wide_radix_tree STATIC
    wide_radix.c
    wide_radix.h
)


add_library(radix_new_tree STATIC
    radix_new.c
    radix_new.h
)

# Create static library for libart (reference implementation)
add_library(libart STATIC
    art.c
    art.h
)

# Create static library for AVL tree
add_library(avl_tree STATIC
    avl.c
)

# Test executables
add_executable(test_radix test_radix.c)
target_link_libraries(test_radix radix_tree)

add_executable(test_wide_radix test_wide_radix.c)
target_link_libraries(test_wide_radix wide_radix_tree)

add_executable(test_radix_new test_radix_new.c)
target_link_libraries(test_radix_new radix_new_tree)

add_executable(test_object_pool test_object_pool.c)
target_link_libraries(test_object_pool radix_new_tree)

add_executable(test_pool_growth test_pool_growth.c)
target_link_libraries(test_pool_growth radix_new_tree)

# Benchmark executable
add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark radix_tree wide_radix_tree radix_new_tree libart avl_tree)

# Set default build type to Release for better performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif() 