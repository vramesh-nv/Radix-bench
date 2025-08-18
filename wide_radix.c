#include "wide_radix.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

// Extract 8-bit byte from 64-bit key at given level (0-7)
static inline uint8_t extract_byte(NvU64 key, int level) {
    return (key >> ((7 - level) * 8)) & 0xFF;
}

// Create a new node
static wide_radix_node_t* create_node(bool is_leaf) {
    wide_radix_node_t* node = (wide_radix_node_t*)calloc(1, sizeof(wide_radix_node_t));
    if (node) {
        node->is_leaf = is_leaf;
        node->value = 0;
        // child_mask is already zeroed by calloc
        // children array is already NULLed by calloc
    }
    return node;
}

// Free a node and all its children recursively
static void free_node(wide_radix_node_t* node) {
    if (!node) return;
    
    if (!node->is_leaf) {
        for (int i = 0; i < 256; i++) {
            if (mask_get_bit(&node->child_mask, i) && node->children[i]) {
                free_node(node->children[i]);
            }
        }
    }
    free(node);
}

void wide_radix_init(wide_radix_tree_t* tree, NvU32 key_bits) {
    assert(tree != NULL);
    tree->root = create_node(false);  // Create internal root node
    tree->key_bits = key_bits;
}

bool wide_radix_insert(wide_radix_tree_t* tree, NvU64 key, NvU64 value) {
    assert(tree != NULL && tree->root != NULL);
    
    wide_radix_node_t* current = tree->root;
    
    // Navigate through the tree levels (0-7 for 64-bit keys)
    for (int level = 0; level < 7; level++) {
        uint8_t byte = extract_byte(key, level);
        
        if (mask_get_bit(&current->child_mask, byte)) {
            // Child exists, continue to next level
            current = current->children[byte];
            assert(current != NULL);
        } else {
            // Child doesn't exist, create internal node
            wide_radix_node_t* new_node = create_node(false);
            if (!new_node) return false;
            
            mask_set_bit(&current->child_mask, byte);
            current->children[byte] = new_node;
            current = new_node;
        }
    }
    
    // At the final level (level 7), create or update leaf
    uint8_t final_byte = extract_byte(key, 7);
    
    if (mask_get_bit(&current->child_mask, final_byte)) {
        // Leaf already exists, update value
        wide_radix_node_t* leaf = current->children[final_byte];
        assert(leaf->is_leaf);
        leaf->value = value;
    } else {
        // Create new leaf
        wide_radix_node_t* leaf = create_node(true);
        if (!leaf) return false;
        
        leaf->value = value;
        mask_set_bit(&current->child_mask, final_byte);
        current->children[final_byte] = leaf;
    }
    
    return true;
}

bool wide_radix_delete(wide_radix_tree_t* tree, NvU64 key) {
    assert(tree != NULL && tree->root != NULL);
    
    wide_radix_node_t* current = tree->root;
    wide_radix_node_t* parent = NULL;
    uint8_t parent_byte = 0;
    
    // Navigate to the leaf
    for (int level = 0; level < 7; level++) {
        uint8_t byte = extract_byte(key, level);
        
        if (!mask_get_bit(&current->child_mask, byte)) {
            return false;  // Key doesn't exist
        }
        
        parent = current;
        parent_byte = byte;
        current = current->children[byte];
        assert(current != NULL);
    }
    
    // Check final level
    uint8_t final_byte = extract_byte(key, 7);
    if (!mask_get_bit(&current->child_mask, final_byte)) {
        return false;  // Key doesn't exist
    }
    
    wide_radix_node_t* leaf = current->children[final_byte];
    assert(leaf->is_leaf);
    
    // Remove the leaf
    free(leaf);
    current->children[final_byte] = NULL;
    mask_clear_bit(&current->child_mask, final_byte);
    
    // Clean up empty internal nodes (optional optimization)
    // For now, we'll leave empty internal nodes to avoid complexity
    
    return true;
}

NvU64* wide_radix_lookup(wide_radix_tree_t* tree, NvU64 key) {
    assert(tree != NULL && tree->root != NULL);
    
    wide_radix_node_t* current = tree->root;
    
    // Navigate through all levels
    for (int level = 0; level < 8; level++) {
        uint8_t byte = extract_byte(key, level);
        if (!mask_get_bit(&current->child_mask, byte)) {
            return NULL;  // Key doesn't exist
        }
        
        current = current->children[byte];
        assert(current != NULL);
        
        if (level == 7) {
            // We've reached the leaf level
            assert(current->is_leaf);
            return &current->value;
        }
    }
    
    return NULL;  // Should never reach here
}

NvU64* wide_radix_find_geq(wide_radix_tree_t* tree, NvU64 key) {
    assert(tree != NULL && tree->root != NULL);
    
    // For simplicity, we'll implement a basic version that finds the next key
    // A full implementation would need to traverse the tree more carefully
    
    wide_radix_node_t* current = tree->root;
    NvU64 current_key = 0;
    
    // Navigate through levels, building the key as we go
    for (int level = 0; level < 8; level++) {
        uint8_t byte = extract_byte(key, level);
        
        if (mask_get_bit(&current->child_mask, byte)) {
            // Exact match at this level, continue
            current = current->children[byte];
            assert(current != NULL);
            current_key |= ((NvU64)byte << ((7 - level) * 8));
        } else {
            // Find the next available child at this level
            for (int next_byte = byte + 1; next_byte < 256; next_byte++) {
                if (mask_get_bit(&current->child_mask, next_byte)) {
                    // Found a larger key, navigate to it
                    current = current->children[next_byte];
                    assert(current != NULL);
                    current_key |= ((NvU64)next_byte << ((7 - level) * 8));
                    
                    // Navigate to the leftmost leaf in this subtree
                    for (int remaining_level = level + 1; remaining_level < 8; remaining_level++) {
                        // Find the smallest child
                        for (int child_byte = 0; child_byte < 256; child_byte++) {
                            if (mask_get_bit(&current->child_mask, child_byte)) {
                                current = current->children[child_byte];
                                assert(current != NULL);
                                current_key |= ((NvU64)child_byte << ((7 - remaining_level) * 8));
                                break;
                            }
                        }
                    }
                    
                    if (current->is_leaf) {
                        return &current->value;
                    }
                    return NULL;
                }
            }
            // No larger key found at this level
            return NULL;
        }
        
        if (level == 7) {
            // We've reached the leaf level
            assert(current->is_leaf);
            return &current->value;
        }
    }
    
    return NULL;
}

bool wide_radix_empty(wide_radix_tree_t* tree) {
    assert(tree != NULL);
    if (!tree->root) return true;
    
    // Check if root has any children
    for (int i = 0; i < 256; i++) {
        if (mask_get_bit(&tree->root->child_mask, i)) {
            return false;
        }
    }
    return true;
}

void wide_radix_destroy(wide_radix_tree_t* tree) {
    assert(tree != NULL);
    if (tree->root) {
        free_node(tree->root);
        tree->root = NULL;
    }
}
