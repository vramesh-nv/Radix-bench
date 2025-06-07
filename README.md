# Radix Tree Benchmark

Comparing CUDA driver internal radix tree performance with standard data structures in the C++ std library and reference ART implementations.

## Results (Per Operation)

### Insertion Performance
- **libart (Standard ART)**: 0.071 us/op ⭐ **FASTEST**
- **CUDA Radix Tree**: 0.102 us/op (44% slower)
- **std::multiset**: 0.131 us/op (85% slower)
- **std::set**: 0.156 us/op (120% slower)

### Lookup Performance  
- **libart (Standard ART)**: 0.051 us/op ⭐ **FASTEST**
- **CUDA Radix Tree**: 0.260 us/op (410% slower)
- **std::multiset**: 0.296 us/op (480% slower)
- **std::set**: 0.321 us/op (530% slower)

### Range Query Performance
- **std::set**: 0.046 us/op ⭐ **FASTEST**
- **CUDA Radix Tree**: 0.130 us/op (183% slower)

## Key Findings

The **libart (Standard ART)** dominates both insertion and lookup performance. The CUDA radix tree performs competitively for insertions but has significant optimization opportunities for lookups compared to standard ART implementations.

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