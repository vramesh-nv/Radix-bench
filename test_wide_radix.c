#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "wide_radix.h"

int main() {
    printf("Testing Wide Radix Tree Implementation\n");
    printf("=====================================\n");
    
    wide_radix_tree_t tree;
    wide_radix_init(&tree, 64);
    
    // Test basic operations
    printf("1. Testing basic insert/lookup...\n");
    
    // Insert some test keys
    assert(wide_radix_insert(&tree, 0x1234567890ABCDEF, 100));
    printf("   ✓ Inserted key 0x1234567890ABCDEF\n");
    assert(wide_radix_insert(&tree, 0xABCDEF1234567890, 200));
    printf("   ✓ Inserted key 0xABCDEF1234567890\n");
    assert(wide_radix_insert(&tree, 0x1111111111111111, 300));
    printf("   ✓ Inserted key 0x1111111111111111\n");
    assert(wide_radix_insert(&tree, 0x2222222222222222, 400));
    printf("   ✓ Inserted key 0x2222222222222222\n");
    
    // Test lookups
    NvU64* value = wide_radix_lookup(&tree, 0x1234567890ABCDEF);
    assert(value != NULL && *value == 100);
    printf("   ✓ Found key 0x1234567890ABCDEF = %lu\n", *value);
    
    value = wide_radix_lookup(&tree, 0xABCDEF1234567890);
    assert(value != NULL && *value == 200);
    printf("   ✓ Found key 0xABCDEF1234567890 = %lu\n", *value);
    
    value = wide_radix_lookup(&tree, 0x1111111111111111);
    assert(value != NULL && *value == 300);
    printf("   ✓ Found key 0x1111111111111111 = %lu\n", *value);
    
    value = wide_radix_lookup(&tree, 0x2222222222222222);
    assert(value != NULL && *value == 400);
    printf("   ✓ Found key 0x2222222222222222 = %lu\n", *value);
    
    // Test non-existent key
    value = wide_radix_lookup(&tree, 0x9999999999999999);
    assert(value == NULL);
    printf("   ✓ Correctly returned NULL for non-existent key\n");
    
    printf("2. Testing delete operations...\n");
    
    // Delete a key
    assert(wide_radix_delete(&tree, 0x1111111111111111));
    printf("   ✓ Deleted key 0x1111111111111111\n");
    
    // Verify it's gone
    value = wide_radix_lookup(&tree, 0x1111111111111111);
    assert(value == NULL);
    printf("   ✓ Confirmed key is no longer found\n");
    
    // Verify other keys still exist
    value = wide_radix_lookup(&tree, 0x1234567890ABCDEF);
    assert(value != NULL && *value == 100);
    printf("   ✓ Other keys still accessible\n");
    
    printf("3. Testing edge cases...\n");
    
    // Test with zero key
    assert(wide_radix_insert(&tree, 0, 500));
    value = wide_radix_lookup(&tree, 0);
    assert(value != NULL && *value == 500);
    printf("   ✓ Zero key works correctly\n");
    
    // Test with maximum key
    assert(wide_radix_insert(&tree, 0xFFFFFFFFFFFFFFFF, 600));
    value = wide_radix_lookup(&tree, 0xFFFFFFFFFFFFFFFF);
    assert(value != NULL && *value == 600);
    printf("   ✓ Maximum key works correctly\n");
    
    // Test empty tree
    wide_radix_tree_t empty_tree;
    wide_radix_init(&empty_tree, 64);
    assert(wide_radix_empty(&empty_tree));
    printf("   ✓ Empty tree detection works\n");
    
    wide_radix_destroy(&empty_tree);
    wide_radix_destroy(&tree);
    
    printf("\nAll tests passed! ✓\n");
    return 0;
}
