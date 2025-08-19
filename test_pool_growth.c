#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "radix_new.h"

int main() {
    printf("Testing ObjectPool Dynamic Growth\n");
    printf("=================================\n");
    
    // Test ObjectPool with small initial capacity to force growth
    ObjectPool pool;
    if (objectPoolInit(&pool, sizeof(uint64_t), 5) != 0) {  // Start with only 5 objects
        printf("Failed to initialize ObjectPool\n");
        return -1;
    }
    printf("ObjectPool initialized with capacity 5\n");
    
    // Allocate more objects than the initial capacity to trigger pool growth
    void* results[20];
    for (int i = 0; i < 20; i++) {
        void* obj = objectPoolAlloc(&pool, &results[i]);
        if (obj == NULL || results[i] == NULL) {
            printf("Failed to allocate object %d from pool\n", i);
            return -1;
        }
        // Write some data to the allocated object
        *(uint64_t*)results[i] = i + 1000;
        printf("Allocated object %d with value %lu\n", i, *(uint64_t*)results[i]);
    }
    printf("Successfully allocated 20 objects (initial capacity was only 5)\n");
    
    // Verify all objects have correct values
    printf("\nVerifying object values:\n");
    for (int i = 0; i < 20; i++) {
        if (*(uint64_t*)results[i] != (uint64_t)(i + 1000)) {
            printf("Object %d has wrong value: expected %lu, got %lu\n", 
                   i, (uint64_t)(i + 1000), *(uint64_t*)results[i]);
            return -1;
        }
        printf("Object %d: %lu ✓\n", i, *(uint64_t*)results[i]);
    }
    
    // Free some objects and reallocate to test the free list
    printf("\nFreeing objects 5-14 and reallocating:\n");
    for (int i = 5; i < 15; i++) {
        objectPoolFree(&pool, results[i]);
        printf("Freed object %d\n", i);
    }
    
    // Reallocate from the freed objects
    void* new_results[10];
    for (int i = 0; i < 10; i++) {
        void* obj = objectPoolAlloc(&pool, &new_results[i]);
        if (obj == NULL || new_results[i] == NULL) {
            printf("Failed to reallocate object %d from pool\n", i);
            return -1;
        }
        *(uint64_t*)new_results[i] = i + 2000;
        printf("Reallocated object %d with value %lu\n", i, *(uint64_t*)new_results[i]);
    }
    
    // Cleanup
    objectPoolDestroy(&pool);
    printf("\nObjectPool destroyed successfully\n");
    
    printf("\nAll pool growth tests completed!\n");
    return 0;
}
