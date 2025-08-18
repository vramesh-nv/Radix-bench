#ifndef __WIDE_RADIX_H__
#define __WIDE_RADIX_H__

#include <stdint.h>
#include <stdbool.h>
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Export macro
#define WIDE_RADIX_EXPORT

// 256-bit bitmask for child presence
typedef struct {
    uint64_t bits[4];  // 4 * 64 = 256 bits
} wide_radix_mask_t;

// Wide radix tree node (256 children)
typedef struct wide_radix_node_st {
    wide_radix_mask_t child_mask;           // 256-bit mask indicating which children exist
    struct wide_radix_node_st* children[256]; // 256 child pointers
    NvU64 value;                            // Value stored at leaf nodes (0 for internal nodes)
    bool is_leaf;                           // True if this is a leaf node
} wide_radix_node_t;

// Wide radix tree structure
typedef struct wide_radix_tree_st {
    wide_radix_node_t* root;
    NvU32 key_bits;                         // Number of bits in key (64 for NvU64)
} wide_radix_tree_t;

// Function declarations
WIDE_RADIX_EXPORT void wide_radix_init(wide_radix_tree_t* tree, NvU32 key_bits);
WIDE_RADIX_EXPORT bool wide_radix_insert(wide_radix_tree_t* tree, NvU64 key, NvU64 value);
WIDE_RADIX_EXPORT bool wide_radix_delete(wide_radix_tree_t* tree, NvU64 key);
WIDE_RADIX_EXPORT NvU64* wide_radix_lookup(wide_radix_tree_t* tree, NvU64 key);
WIDE_RADIX_EXPORT NvU64* wide_radix_find_geq(wide_radix_tree_t* tree, NvU64 key);
WIDE_RADIX_EXPORT bool wide_radix_empty(wide_radix_tree_t* tree);
WIDE_RADIX_EXPORT void wide_radix_destroy(wide_radix_tree_t* tree);

// Utility functions for bit manipulation
static inline bool mask_get_bit(const wide_radix_mask_t* mask, int bit) {
    return (mask->bits[bit / 64] & (1ULL << (bit % 64))) != 0;
}

static inline void mask_set_bit(wide_radix_mask_t* mask, int bit) {
    mask->bits[bit / 64] |= (1ULL << (bit % 64));
}

static inline void mask_clear_bit(wide_radix_mask_t* mask, int bit) {
    mask->bits[bit / 64] &= ~(1ULL << (bit % 64));
}

#ifdef __cplusplus
}
#endif

#endif // __WIDE_RADIX_H__
