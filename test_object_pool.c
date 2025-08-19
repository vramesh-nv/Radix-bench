#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "radix_new.h"

int main() {
    printf("Testing ObjectPool Implementation\n");
    printf("================================\n");
    
    // Test ObjectPool
    ObjectPool pool;
    if (objectPoolInit(&pool, sizeof(uint64_t), 100) != 0) {
        printf("Failed to initialize ObjectPool\n");
        return -1;
    }
    printf("ObjectPool initialized successfully\n");
    
    // Test allocation
    void* result;
    void* obj = objectPoolAlloc(&pool, &result);
    if (obj == NULL || result == NULL) {
        printf("Failed to allocate from ObjectPool\n");
        return -1;
    }
    printf("Successfully allocated object from pool\n");
    
    // Test multiple allocations
    void* results[10];
    for (int i = 0; i < 10; i++) {
        void* obj = objectPoolAlloc(&pool, &results[i]);
        if (obj == NULL || results[i] == NULL) {
            printf("Failed to allocate object %d from pool\n", i);
            return -1;
        }
        // Write some data to the allocated object
        *(uint64_t*)results[i] = i + 1000;
    }
    printf("Successfully allocated 10 objects from pool\n");
    
    // Test freeing
    for (int i = 0; i < 10; i++) {
        objectPoolFree(&pool, results[i]);
    }
    printf("Successfully freed 10 objects\n");
    
    // Test reallocation
    void* new_results[10];
    for (int i = 0; i < 10; i++) {
        void* obj = objectPoolAlloc(&pool, &new_results[i]);
        if (obj == NULL || new_results[i] == NULL) {
            printf("Failed to reallocate object %d from pool\n", i);
            return -1;
        }
    }
    printf("Successfully reallocated 10 objects from pool\n");
    
    // Cleanup
    objectPoolDestroy(&pool);
    printf("ObjectPool destroyed successfully\n");
    
    printf("\nAll ObjectPool tests completed!\n");
    return 0;
}
