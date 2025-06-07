#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "radix.h"

int main() {
    CUradixTree tree;
    CUradixNode nodes[5];
    NvU64 keys[] = {10, 5, 20, 15, 8};
    int num_keys = sizeof(keys) / sizeof(keys[0]);
    
    printf("Radix Tree Test Program\n");
    printf("=======================\n\n");
    
    // Initialize the tree with 8-bit keys
    radixTreeInit(&tree, 63);
    printf("Initialized radix tree with 8-bit keys\n");
    
    // Check if tree is empty
    printf("Tree is empty: %s\n\n", radixTreeEmpty(&tree) ? "true" : "false");
    
    // Insert keys
    printf("Inserting keys: ");
    for (int i = 0; i < num_keys; i++) {
        printf("%lu ", keys[i]);
        radixTreeInsert(&tree, &nodes[i], keys[i]);
    }
    printf("\n\n");
    
    // Check if tree is empty after insertion
    printf("Tree is empty after insertion: %s\n\n", radixTreeEmpty(&tree) ? "true" : "false");
    
    // Test finding keys
    printf("Testing radixTreeFindGEQ:\n");
    NvU64 search_keys[] = {3, 5, 7, 10, 12, 15, 18, 25};
    int num_search = sizeof(search_keys) / sizeof(search_keys[0]);
    
    for (int i = 0; i < num_search; i++) {
        CUradixNode *found = radixTreeFindGEQ(&tree, search_keys[i]);
        if (found) {
            printf("  Search for %lu: Found node with key %lu\n", search_keys[i], found->key);
        } else {
            printf("  Search for %lu: No node found\n", search_keys[i]);
        }
    }
    
    printf("\nTest completed successfully!\n");
    return 0;
} 