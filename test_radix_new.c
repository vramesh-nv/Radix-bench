#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "radix_new.h"

int main() {
    printf("Testing Radix New Tree Implementation\n");
    printf("=====================================\n");
    
    // Initialize tree
    WideRadixTree tree;
    treeInit(&tree, 64, 8);
    /*if (treeInit(&tree, 64, 8) != 0) {
        printf("Failed to initialize tree\n");
        return -1;
    }*/
    printf("Tree initialized successfully\n");
    
    // Test insertion
    uint64_t keys[] = {100, 200, 300, 150, 250, 350, 120, 180, 220, 280};
    uint64_t values[] = {1000, 2000, 3000, 1500, 2500, 3500, 1200, 1800, 2200, 2800};
    int num_keys = sizeof(keys) / sizeof(keys[0]);
    
    printf("Inserting %d key-value pairs...\n", num_keys);
    for (int i = 0; i < num_keys; i++) {
        uint64_t existing;
        int result = treeInsertOrReturnExisting(&tree, keys[i], values[i], &existing);
        if (result == 0) {
            printf("Inserted key %lu with value %lu\n", keys[i], values[i]);
        } else {
            printf("Failed to insert key %lu\n", keys[i]);
        }
    }
    
    // Test lookup
    printf("\nTesting lookups...\n");
    for (int i = 0; i < num_keys; i++) {
        uint64_t found = treeFind(&tree, keys[i]);
        if (found == values[i]) {
            printf("Found key %lu with correct value %lu\n", keys[i], found);
        } else {
            printf("Lookup failed for key %lu (expected %lu, got %lu)\n", keys[i], values[i], found);
        }
    }
    
    // Test non-existent key
    uint64_t not_found = treeFind(&tree, 999);
    if (not_found == 0) {
        printf("Correctly returned 0 for non-existent key 999\n");
    } else {
        printf("Unexpected result for non-existent key 999: %lu\n", not_found);
    }
    
    // Test findGEQ
    printf("\nTesting findGEQ...\n");
    uint64_t query_keys[] = {110, 160, 210, 260, 310, 999};
    int num_queries = sizeof(query_keys) / sizeof(query_keys[0]);
    
    for (int i = 0; i < num_queries; i++) {
        uint64_t found = treeFindGEQ(&tree, query_keys[i]);
        if (found != 0) {
            printf("findGEQ(%lu) = %lu\n", query_keys[i], found);
        } else {
            printf("findGEQ(%lu) = not found\n", query_keys[i]);
        }
    }
    
    // Test removal
    printf("\nTesting removal...\n");
    uint64_t removed = treeRemove(&tree, 200);
    if (removed == 2000) {
        printf("Successfully removed key 200 with value %lu\n", removed);
    } else {
        printf("Failed to remove key 200 (expected 2000, got %lu)\n", removed);
    }
    
    // Verify removal
    uint64_t found_after_remove = treeFind(&tree, 200);
    if (found_after_remove == 0) {
        printf("Key 200 correctly not found after removal\n");
    } else {
        printf("Key 200 still found after removal: %lu\n", found_after_remove);
    }
    
    // Cleanup
    treeDestroy(&tree);
    printf("\nTree destroyed successfully\n");
    
    printf("\nAll tests completed!\n");
    return 0;
}
