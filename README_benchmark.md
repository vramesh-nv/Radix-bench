# getFirstSetBit Benchmark

This benchmark compares two implementations of the `getFirstSetBit` function found in `radix_new.c`:

## Implementations

### 1. Manual Implementation (Commented out in radix_new.c)
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

**Characteristics:**
- Uses manual bit manipulation with bitwise operations
- Implements a binary search approach to find the first set bit
- Highly portable across all platforms and compilers
- ~9 lines of C code

### 2. Built-in Implementation (Currently active in radix_new.c)
```c
static inline uint64_t getFirstSetBit(uint64_t val) {
    return val ? __builtin_ctzll(val) : 64;
}
```

**Characteristics:**
- Uses GCC/Clang built-in function `__builtin_ctzll`
- Leverages hardware instructions when available (e.g., BSF on x86)
- Very concise (1 line of C code)
- Compiler-specific, may not work on other compilers

## What the Function Does

The `getFirstSetBit` function returns the position of the least significant set bit (1) in a 64-bit unsigned integer. It returns:
- The 0-based position of the first set bit (0-63)
- 64 if the input is 0 (no set bits)

**Examples:**
- `getFirstSetBit(0x0000000000000001)` → `0` (bit 0 is set)
- `getFirstSetBit(0x0000000000000002)` → `1` (bit 1 is set)
- `getFirstSetBit(0x8000000000000000)` → `63` (bit 63 is set)
- `getFirstSetBit(0x0000000000000000)` → `64` (no bits set)

## Building and Running

### Prerequisites
- GCC compiler (tested with GCC 7.0+)
- Standard C library
- Linux/Unix environment

### Build
```bash
make
```

### Run
```bash
make run
```

### Clean
```bash
make clean
```

## Benchmark Details

The benchmark performs:

1. **Correctness Verification**
   - Tests known edge cases
   - Tests 1000 random values
   - Compares both implementations against a reference implementation

2. **Performance Testing**
   - 100,000 test values per iteration
   - 100 iterations for statistical significance
   - Three types of test data:
     - Random values
     - Sparse values (few set bits)
     - Dense values (many set bits)

3. **Analysis**
   - Execution time comparison
   - Speedup calculation
   - Operations per second
   - Memory usage analysis
   - Code size comparison
   - Portability assessment

## Expected Results

Typically, the built-in implementation (`__builtin_ctzll`) will be faster because:
- It can use specialized CPU instructions (e.g., BSF on x86)
- The compiler can optimize it more aggressively
- It's implemented in highly optimized assembly

The manual implementation may be competitive or faster in some edge cases, especially when the compiler can't optimize the built-in function effectively.

## Customization

You can modify the benchmark by:
- Changing `numTests` and `iterations` in the source code
- Adding new test data generation functions
- Testing with different optimization levels
- Running on different hardware architectures

## Notes

- The benchmark uses a fixed random seed (42) for reproducible results
- Results may vary significantly between different hardware and compiler versions
- The manual implementation is included for educational purposes and portability
- Consider your target platform and compiler when choosing between implementations
