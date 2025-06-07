# Radix Tree Benchmark

Comparing CUDA driver internal radix tree performance with standard data structures in the C++ std library and reference ART implementations.

## Results (Per Operation)

### Insertion Performance
- **CUDA Radix Tree**: 0.102 us/op ⭐ **FASTEST**
- **libart (Standard ART)**: 0.116 us/op (14% slower)
- **std::multiset**: 0.130 us/op (27% slower)
- **std::set**: 0.159 us/op (56% slower)

### Lookup Performance  
- **libart (Standard ART)**: 0.056 us/op ⭐ **FASTEST**
- **CUDA Radix Tree**: 0.215 us/op (284% slower)
- **std::set**: 0.278 us/op (396% slower)
- **std::multiset**: 0.282 us/op (404% slower)

### Range Query Performance
- **std::set**: 0.044 us/op ⭐ **FASTEST**
- **CUDA Radix Tree**: 0.132 us/op (200% slower)

## Key Findings

The CUDA radix tree excels at **insertion-heavy workloads** but has optimization opportunities for lookups compared to standard ART implementations.

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