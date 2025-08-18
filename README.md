# Radix Tree Benchmark

Comparing CUDA driver internal radix tree performance with standard data structures in the C++ std library, reference ART implementations, wide radix tree, and AVL tree.

## Results (Per Operation)

### Insertion Performance
- **libart (Standard ART)**: 0.063 us/op ⭐ **FASTEST**
- **AVL Tree**: 0.119 us/op (89% slower)
- **CUDA Radix Tree**: 0.102 us/op (62% slower)
- **std::multiset**: 0.130 us/op (106% slower)
- **std::set**: 0.155 us/op (146% slower)
- **Wide Radix Tree**: 0.789 us/op (1152% slower)

### Lookup Performance  
- **Wide Radix Tree**: 0.050 us/op ⭐ **FASTEST**
- **libart (Standard ART)**: 0.050 us/op (0% slower)
- **AVL Tree**: 0.267 us/op (434% slower)
- **CUDA Radix Tree**: 0.262 us/op (424% slower)
- **std::multiset**: 0.293 us/op (486% slower)
- **std::set**: 0.321 us/op (542% slower)

### Range Query Performance
- **std::set**: 0.046 us/op ⭐ **FASTEST**
- **CUDA Radix Tree**: 0.130 us/op (183% slower)

## Key Findings

- **libart (Standard ART)** and **Wide Radix Tree** tie for fastest lookup performance
- **libart (Standard ART)** has the fastest insertion performance
- **AVL Tree** performs competitively for insertions (89% slower than libart) but is slower for lookups
- **Wide Radix Tree** has excellent lookup performance but slower insertion due to memory allocation overhead
- **CUDA Radix Tree** performs competitively for insertions but has optimization opportunities for lookups

## Building

```bash
mkdir -p build
cd build
cmake ..
make
```

## Running

```bash
# Simple test
./test_radix

# Performance comparison
./benchmark
``` 