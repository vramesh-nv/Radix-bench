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

# Create static library for libart (reference implementation)
add_library(libart STATIC
    art.c
    art.h
)

# Test executable
add_executable(test_radix test_radix.c)
target_link_libraries(test_radix radix_tree)

# Benchmark executable
add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark radix_tree libart)

# Set default build type to Release for better performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif() 