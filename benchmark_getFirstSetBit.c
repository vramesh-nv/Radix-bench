#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// First implementation: Manual bit manipulation
static inline uint64_t getFirstSetBit_manual(uint64_t val) {
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

// Second implementation: Using built-in function
static inline uint64_t getFirstSetBit_builtin(uint64_t val) {
    return val ? __builtin_ctzll(val) : 64;
}

// Reference implementation for correctness verification
static inline uint64_t getFirstSetBit_reference(uint64_t val) {
    if (val == 0) return 64;
    uint64_t bit = 0;
    while ((val & 1) == 0) {
        val >>= 1;
        bit++;
    }
    return bit;
}

// Test data generation functions
uint64_t generateRandomValue() {
    // Generate random 64-bit values with various bit patterns
    uint64_t val = 0;
    for (int i = 0; i < 8; i++) {
        val = (val << 8) | (rand() & 0xFF);
    }
    return val;
}

uint64_t generateSparseValue() {
    // Generate values with few set bits
    uint64_t val = 0;
    int numBits = rand() % 5; // 0-4 set bits
    for (int i = 0; i < numBits; i++) {
        int pos = rand() % 64;
        val |= (1ULL << pos);
    }
    return val;
}

uint64_t generateDenseValue() {
    // Generate values with many set bits
    uint64_t val = 0;
    int numBits = 60 + (rand() % 5); // 60-64 set bits
    for (int i = 0; i < numBits; i++) {
        int pos = rand() % 64;
        val |= (1ULL << pos);
    }
    return val;
}

// Benchmark function
double benchmark_function(uint64_t (*func)(uint64_t), uint64_t* testData, int numTests, int iterations) {
    clock_t start = clock();
    
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < numTests; i++) {
            volatile uint64_t result = func(testData[i]);
            (void)result; // Prevent compiler from optimizing away
        }
    }
    
    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

// Correctness verification
int verify_correctness() {
    printf("Verifying correctness...\n");
    
    // Test cases with known results
    uint64_t testCases[] = {
        0x0000000000000001ULL,  // Should return 0
        0x0000000000000002ULL,  // Should return 1
        0x0000000000000004ULL,  // Should return 2
        0x0000000000000008ULL,  // Should return 3
        0x8000000000000000ULL,  // Should return 63
        0x0000000000000000ULL,  // Should return 64
        0x0000000000000003ULL,  // Should return 0
        0x0000000000000006ULL,  // Should return 1
        0x000000000000000CULL,  // Should return 2
    };
    
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    int errors = 0;
    
    for (int i = 0; i < numTestCases; i++) {
        uint64_t expected = getFirstSetBit_reference(testCases[i]);
        uint64_t result1 = getFirstSetBit_manual(testCases[i]);
        uint64_t result2 = getFirstSetBit_builtin(testCases[i]);
        
        if (result1 != expected) {
                        printf("ERROR: Manual implementation failed for 0x%016lx: expected %lu, got %lu\n",
                   testCases[i], expected, result1);
            errors++;
        }
        
        if (result2 != expected) {
                        printf("ERROR: Built-in implementation failed for 0x%016lx: expected %lu, got %lu\n",
                   testCases[i], expected, result2);
            errors++;
        }
    }
    
    // Test random values
    srand(42); // Fixed seed for reproducibility
    for (int i = 0; i < 1000; i++) {
        uint64_t val = generateRandomValue();
        uint64_t expected = getFirstSetBit_reference(val);
        uint64_t result1 = getFirstSetBit_manual(val);
        uint64_t result2 = getFirstSetBit_builtin(val);
        
        if (result1 != expected || result2 != expected) {
                        printf("ERROR: Random test failed for 0x%016lx: expected %lu, manual %lu, builtin %lu\n",
                   val, expected, result1, result2);
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("✓ All correctness tests passed!\n");
        return 1;
    } else {
        printf("✗ %d correctness tests failed!\n", errors);
        return 0;
    }
}

int main() {
    printf("=== getFirstSetBit Benchmark ===\n\n");
    
    // Verify correctness first
    if (!verify_correctness()) {
        printf("Correctness verification failed. Aborting benchmark.\n");
        return 1;
    }
    
    printf("\nRunning performance benchmarks...\n\n");
    
    const int numTests = 100000;
    const int iterations = 100;
    
    // Generate test data
    uint64_t* testData = malloc(numTests * sizeof(uint64_t));
    if (!testData) {
        printf("Failed to allocate test data\n");
        return 1;
    }
    
    srand(42); // Fixed seed for reproducibility
    
    // Generate different types of test data
    for (int i = 0; i < numTests; i++) {
        if (i < numTests / 3) {
            testData[i] = generateRandomValue();
        } else if (i < 2 * numTests / 3) {
            testData[i] = generateSparseValue();
        } else {
            testData[i] = generateDenseValue();
        }
    }
    
    // Warm up
    printf("Warming up...\n");
    for (int i = 0; i < 1000; i++) {
        getFirstSetBit_manual(testData[i % numTests]);
        getFirstSetBit_builtin(testData[i % numTests]);
    }
    
    // Benchmark manual implementation
    printf("Benchmarking manual implementation...\n");
    double time_manual = benchmark_function(getFirstSetBit_manual, testData, numTests, iterations);
    
    // Benchmark built-in implementation
    printf("Benchmarking built-in implementation...\n");
    double time_builtin = benchmark_function(getFirstSetBit_builtin, testData, numTests, iterations);
    
    // Results
    printf("\n=== Results ===\n");
    printf("Manual implementation:  %.6f seconds\n", time_manual);
    printf("Built-in implementation: %.6f seconds\n", time_builtin);
    printf("Speedup: %.2fx\n", time_manual / time_builtin);
    
    if (time_manual < time_builtin) {
        printf("Manual implementation is %.2fx faster\n", time_builtin / time_manual);
    } else {
        printf("Built-in implementation is %.2fx faster\n", time_manual / time_builtin);
    }
    
    // Detailed analysis
    printf("\n=== Detailed Analysis ===\n");
    printf("Total operations: %d\n", numTests * iterations);
    printf("Operations per second (manual): %.0f\n", (numTests * iterations) / time_manual);
    printf("Operations per second (built-in): %.0f\n", (numTests * iterations) / time_builtin);
    
    // Memory usage analysis
    printf("\n=== Memory Analysis ===\n");
    printf("Manual implementation: No additional memory overhead\n");
    printf("Built-in implementation: No additional memory overhead\n");
    
    // Code size analysis
    printf("\n=== Code Size Analysis ===\n");
    printf("Manual implementation: ~9 lines of C code\n");
    printf("Built-in implementation: ~1 line of C code\n");
    
    // Portability analysis
    printf("\n=== Portability Analysis ===\n");
    printf("Manual implementation: Highly portable, works on all platforms\n");
    printf("Built-in implementation: GCC/Clang specific, may not work on other compilers\n");
    
    free(testData);
    
    printf("\n=== Benchmark Complete ===\n");
    return 0;
}
