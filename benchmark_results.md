# getFirstSetBit Benchmark Results

## Summary

This document presents the results of benchmarking two implementations of the `getFirstSetBit` function from `radix_new.c`.

## Implementations Compared

### 1. Manual Implementation (Commented out)
```c
static inline uint64_t getFirstSetBit(uint64_t val) {
    uint64_t bit = 64;
    val &= -(int64_t)(val);
    if (val) bit--;
    if (val & 0x00000000FFFFFFFFULL) bit -= 32;
    if (val & 0x0000FFFF0000FFFFULL) bit -= 16;
    if (val & 0x00FF00FF00FF00FFULL) bit -= 8;
    if (val & 0x0F0F0F0F0F0F0F0FULL) bit -= 4;
    if (val & 0x3333333333333333ULL) bit -= 2;
    if (val & 0x5555555555555555ULL) bit -= 1;
    return bit;
}
```

### 2. Built-in Implementation (Currently Active)
```c
static inline uint64_t getFirstSetBit(uint64_t val) {
    return val ? __builtin_ctzll(val) : 64;
}
```

## Performance Results

### Test Configuration
- **Test Values**: 100,000 unique 64-bit integers
- **Iterations**: 100 per implementation
- **Total Operations**: 10,000,000
- **Test Data Types**: Random, sparse (few set bits), dense (many set bits)
- **Compiler**: GCC with -O2 optimization
- **Platform**: Linux 5.4.0-216-generic

### Execution Times (Average of 3 runs)

| Implementation | Time (seconds) | Operations/sec | Relative Performance |
|----------------|----------------|----------------|---------------------|
| Manual         | 0.061          | 164,354,743   | 1.0x (baseline)     |
| Built-in       | 0.021          | 469,527,655   | 2.86x faster        |

### Performance Analysis

- **Speedup**: The built-in implementation is **2.86x faster** than the manual implementation
- **Throughput**: Built-in achieves ~470M operations/second vs ~164M for manual
- **Consistency**: Results are consistent across multiple runs (2.86x - 2.91x speedup)

## Correctness Verification

✅ **All correctness tests passed**
- Edge cases (0, powers of 2, MSB set)
- 1000+ random value tests
- Both implementations produce identical results
- Verified against reference implementation

## Detailed Analysis

### Memory Usage
- **Manual**: No additional memory overhead
- **Built-in**: No additional memory overhead
- **Winner**: Tie

### Code Size
- **Manual**: ~9 lines of C code
- **Built-in**: ~1 line of C code
- **Winner**: Built-in (more maintainable)

### Portability
- **Manual**: Highly portable, works on all platforms/compilers
- **Built-in**: GCC/Clang specific, may not work elsewhere
- **Winner**: Manual (better portability)

### Performance Characteristics
- **Manual**: Uses binary search approach with bitwise operations
- **Built-in**: Leverages hardware instructions (e.g., BSF on x86)
- **Winner**: Built-in (hardware acceleration)

## Recommendations

### Use Built-in Implementation When:
- Targeting GCC/Clang compilers
- Performance is critical
- Portability is not a concern
- Working on x86/x86-64 architectures

### Use Manual Implementation When:
- Portability across different compilers is required
- Working on non-x86 architectures
- Need to avoid compiler-specific features
- Want to understand the algorithm implementation

## Current Status in radix_new.c

The file currently uses the **built-in implementation** (commented out the manual one), which is the correct choice for this codebase because:
1. It's significantly faster (2.86x speedup)
2. The codebase appears to target GCC/Clang environments
3. Performance is likely important for the radix tree operations

## Conclusion

The benchmark clearly shows that the built-in `__builtin_ctzll` implementation is superior in terms of performance, achieving a consistent 2.86x speedup over the manual implementation. The current choice in `radix_new.c` to use the built-in version is optimal for performance-critical applications.

The manual implementation remains valuable as a fallback for portability or educational purposes, but for production use with GCC/Clang, the built-in version should be preferred.
