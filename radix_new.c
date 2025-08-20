#include "radix_new.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ObjectPool implementation
int objectPoolInit(ObjectPool* pool, size_t objectSize, size_t initialCapacity) {
    if (!pool || objectSize == 0 || initialCapacity == 0) {
        return -1;
    }
    
    pool->objectSize = objectSize;
    pool->initialCapacity = initialCapacity;
    pool->pools = NULL;
    pool->currentPool = NULL;
    
    // Create the first pool
    ObjectPoolBlock* firstPool = (ObjectPoolBlock*)calloc(1, sizeof(ObjectPoolBlock));
    if (!firstPool) {
        return -1;
    }
    
    // Allocate the main pool
    firstPool->pool = calloc(1, objectSize * initialCapacity);
    if (!firstPool->pool) {
        free(firstPool);
        return -1;
    }
    
    // Allocate the free list
    firstPool->freeList = calloc(1, sizeof(void*) * initialCapacity);
    if (!firstPool->freeList) {
        free(firstPool->pool);
        free(firstPool);
        return -1;
    }
    
    firstPool->objectSize = objectSize;
    firstPool->capacity = initialCapacity;
    firstPool->used = 0;
    firstPool->freeListTop = 0;
    firstPool->next = NULL;
    
    // Initialize free list with all objects
    for (size_t i = 0; i < initialCapacity; i++) {
        firstPool->freeList[i] = (char*)firstPool->pool + (i * objectSize);
    }
    firstPool->freeListTop = initialCapacity;
    
    pool->pools = firstPool;
    pool->currentPool = firstPool;
    
    return 0;
}

void objectPoolDestroy(ObjectPool* pool) {
    if (pool) {
        ObjectPoolBlock* current = pool->pools;
        while (current) {
            ObjectPoolBlock* next = current->next;
            if (current->pool) {
                free(current->pool);
            }
            if (current->freeList) {
                free(current->freeList);
            }
            free(current);
            current = next;
        }
        memset(pool, 0, sizeof(*pool));
    }
}

// Helper function to create a new pool
static ObjectPoolBlock* createNewPool(size_t objectSize, size_t capacity) {
    ObjectPoolBlock* newPool = (ObjectPoolBlock*)calloc(1, sizeof(ObjectPoolBlock));
    if (!newPool) {
        return NULL;
    }
    
    // Allocate the main pool
    newPool->pool = calloc(1, objectSize * capacity);
    if (!newPool->pool) {
        free(newPool);
        return NULL;
    }
    
    // Allocate the free list
    newPool->freeList = calloc(1, sizeof(void*) * capacity);
    if (!newPool->freeList) {
        free(newPool->pool);
        free(newPool);
        return NULL;
    }
    
    newPool->objectSize = objectSize;
    newPool->capacity = capacity;
    newPool->used = 0;
    newPool->freeListTop = 0;
    newPool->next = NULL;
    
    // Initialize free list with all objects
    for (size_t i = 0; i < capacity; i++) {
        newPool->freeList[i] = (char*)newPool->pool + (i * objectSize);
    }
    newPool->freeListTop = capacity;
    
    return newPool;
}

void* objectPoolAlloc(ObjectPool* pool, void** result) {
    if (!pool || !result) {
        return NULL;
    }
    
    // Try to find a pool with available objects
    ObjectPoolBlock* poolToUse = NULL;
    
    // First, check if current pool has available objects
    if (pool->currentPool && pool->currentPool->freeListTop > 0) {
        poolToUse = pool->currentPool;
    } else {
        // Search through all pools for available objects
        ObjectPoolBlock* current = pool->pools;
        while (current) {
            if (current->freeListTop > 0) {
                poolToUse = current;
                pool->currentPool = current;
                break;
            }
            current = current->next;
        }
    }
    
    // If no pool has available objects, create a new one
    if (!poolToUse) {
        size_t newCapacity = pool->initialCapacity * 2;  // Double the capacity for new pools
        ObjectPoolBlock* newPool = createNewPool(pool->objectSize, newCapacity);
        if (!newPool) {
            return NULL;
        }
        
        // Add to the end of the pool list
        if (pool->pools == NULL) {
            pool->pools = newPool;
        } else {
            ObjectPoolBlock* last = pool->pools;
            while (last->next) {
                last = last->next;
            }
            last->next = newPool;
        }
        
        pool->currentPool = newPool;
        poolToUse = newPool;
    }
    
    // Get object from the selected pool
    if (poolToUse->freeListTop == 0) {
        return NULL;  // Safety check
    }
    
    poolToUse->freeListTop--;
    void* obj = poolToUse->freeList[poolToUse->freeListTop];
    
    // Validate the object pointer
    if (obj < poolToUse->pool || 
        (void*)((char*)poolToUse->pool + (poolToUse->capacity * poolToUse->objectSize)) <= obj) {
        return NULL;  // Invalid object pointer
    }
    
    *result = obj;
    poolToUse->used++;
    
    return obj;
}

void objectPoolFree(ObjectPool* pool, void* obj) {
    if (!pool || !obj) {
        return;
    }
    
    // Find which pool this object belongs to
    ObjectPoolBlock* current = pool->pools;
    while (current) {
        if (obj >= current->pool && 
            (void*)((char*)current->pool + (current->capacity * current->objectSize)) > obj) {
            
            // Add back to this pool's free list
            if (current->freeListTop < current->capacity) {
                current->freeList[current->freeListTop] = obj;
                current->freeListTop++;
                current->used--;
            }
            return;
        }
        current = current->next;
    }
}

struct WideRadixNode {
    uint64_t bits[4];
    void* children[4];
};

struct WideRadixTree {
    WideRadixNode root;
    uint8_t numLevels;
    ObjectPool nonLeafPool;
    ObjectPool leafPool;
};

void treeInit(WideRadixTree *tree, uint8_t log2Max, uint8_t log2Align) {
    memset(&tree->root, 0, sizeof(tree->root));
    tree->numLevels = ((log2Max - log2Align) + 7) >> 3;
    
    // Use smaller initial pool sizes since we can now grow dynamically
    size_t nonLeafPoolSize = 64 * sizeof(WideRadixNode);
    objectPoolInit(&tree->nonLeafPool, nonLeafPoolSize, 100);  // Start with 100 non-leaf nodes
    
    size_t leafPoolSize = 64 * sizeof(uint64_t);
    objectPoolInit(&tree->leafPool, leafPoolSize, 1000);  // Start with 1000 leaf values
}

uint64_t treeRemove(WideRadixTree *tree, uint64_t key);

int treeInsertOrReturnExisting(WideRadixTree *tree, uint64_t key, uint64_t value, uint64_t *existing) {
    if (!tree || !existing) {
        return -1;
    }
    
    WideRadixNode *node = &tree->root;
    *existing = 0;
    for (uint8_t level = 0; level < tree->numLevels; level++) {
        uint8_t keyLevelBits = (uint8_t)((key >> ((tree->numLevels - level - 1) << 3)) & 0xFF);
        uint8_t keyLevelIdx = keyLevelBits >> 6;
        uint8_t keyLevelChild = keyLevelBits & 0x3F;
        bool isLastLevel = (level == (tree->numLevels - 1));
        
        if (keyLevelIdx >= 4) {
            return -1;  // Safety check
        }
        
        if (node->children[keyLevelIdx] == NULL) {
            void* result = objectPoolAlloc(isLastLevel ? &tree->leafPool : &tree->nonLeafPool, &node->children[keyLevelIdx]);
            if (result == NULL || node->children[keyLevelIdx] == NULL) {
                return -1;
            }
        }
        
        // Ensure the child is allocated before proceeding
        if (node->children[keyLevelIdx] == NULL) {
            return -1;
        }
        
        node->bits[keyLevelIdx] |= 1ULL << keyLevelChild;
        if (isLastLevel) {
            // Additional safety check before accessing the array
            if (keyLevelChild >= 64) {
                return -1;  // Safety check
            }
            *existing = ((uint64_t *)node->children[keyLevelIdx])[keyLevelChild];
            if (*existing == 0) {
                ((uint64_t *)node->children[keyLevelIdx])[keyLevelChild] = value;
            }
            return 0;
        }
        node = &((WideRadixNode*)node->children[keyLevelIdx])[keyLevelChild];
    }
    return 0;
}


#if 1
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
#else
static inline uint64_t getFirstSetBit(uint64_t val) {
    return val ? __builtin_ctzll(val) : 64;
}
#endif

static inline uint64_t getFirstSetBitInRange(uint64_t *vals, uint64_t startBit, uint64_t endBit) {
    uint64_t startIdx = startBit >> 6, endIdx = endBit >> 6, retval = 0;
    for (uint64_t idx = startIdx; idx <= endIdx; idx++) {
        uint64_t mask = ~0ULL;
        if (idx == startIdx) {
            mask &= (~0ULL) << (startBit & 0x3F);
        }
        if (idx == endIdx) {
            mask &= (~0ULL) >> (63 - (endBit & 0x3F));
        }
        retval = getFirstSetBit(vals[idx] & mask);
        if (retval < 64) {
            return (retval | (idx << 6));
        }
    }
    return endBit+1;
}

static inline void getKeyLevelBits(uint64_t key, uint8_t numLevels, uint8_t idx[8], uint8_t child[8]) {
    for (int8_t level = (int8_t)numLevels - 1; level >= 0; level--) {
        idx[level] = (key & 0xC0) >> 6;
        child[level] = key & 0x3F;
        key >>= 8;
    }
}

uint64_t treeFind(WideRadixTree *tree, uint64_t key) {
    WideRadixNode *node = &tree->root;
    uint8_t lastLevel = tree->numLevels - 1;
    for (uint8_t level = 0; level < tree->numLevels; level++) {
        uint8_t keyLevelBits = (uint8_t)((key >> ((lastLevel - level) << 3)) & 0xFF);
        uint8_t keyLevelIdx = keyLevelBits >> 6;
        uint8_t keyLevelChild = keyLevelBits & 0x3F;
        
        if (keyLevelIdx >= 4) {
            return 0;  // Safety check
        }
        
        if (!(node->bits[keyLevelIdx] & (1ULL << keyLevelChild))) {
            return 0;
        }
        
        if (node->children[keyLevelIdx] == NULL) {
            return 0;  // Safety check
        }
        
        if (level == lastLevel) {
            return ((uint64_t*)node->children[keyLevelIdx])[keyLevelChild];
        }
        node = &((WideRadixNode*)node->children[keyLevelIdx])[keyLevelChild];
    }
    return 0;
}

uint64_t treeFindGEQ(WideRadixTree *tree, uint64_t key) {
    uint8_t keyLevel[8], idx, child;
    WideRadixNode *nodes[8];
    uint8_t lastLevel = tree->numLevels - 1, level;

    nodes[0] = &tree->root;
    for (level = 0; level < tree->numLevels; level++) {
        keyLevel[level] = (key >> ((lastLevel - level) << 3)) & 0xFF;
        idx = keyLevel[level] >> 6, child = keyLevel[level] & 0x3F;
        
        if (idx >= 4) {
            return 0;  // Safety check
        }
        
        if (!(nodes[level]->bits[idx] & (1ULL << child))) {
            break;
        }
        
        if (nodes[level]->children[idx] == NULL) {
            return 0;  // Safety check
        }
        
        if (level == lastLevel) {
            return ((uint64_t*)nodes[level]->children[idx])[child];
        }
        nodes[level + 1] = &((WideRadixNode*)nodes[level]->children[idx])[child];
    }

    uint64_t firstSetBit;
    while ((firstSetBit = getFirstSetBitInRange(nodes[level]->bits, keyLevel[level]+(uint64_t)(1), 255)) > 255) {
        if (level == 0) {
            return 0;
        }
        level--;
    };

    for (; level < tree->numLevels; level++) {
        idx = firstSetBit >> 6, child = firstSetBit & 0x3F;
        if (level == lastLevel) {
            return ((uint64_t*)nodes[level]->children[idx])[child];
        }
        nodes[level + 1] = &((WideRadixNode*)nodes[level]->children[idx])[child];
        firstSetBit = getFirstSetBitInRange(nodes[level+1]->bits, 0, 255);
    }

    return 0;
}

uint64_t treeRemove(WideRadixTree *tree, uint64_t key) {
    uint8_t keyLevelIdx[8], keyLevelChild[8];
    WideRadixNode *nodes[8];
    uint8_t lastLevel = tree->numLevels - 1;
    uint64_t value = 0;

    getKeyLevelBits(key, tree->numLevels, keyLevelIdx, keyLevelChild);

    nodes[0] = &tree->root;
    for (uint8_t level = 0; level < lastLevel; level++) {
        if (keyLevelIdx[level] >= 4) {
            return 0;  // Safety check
        }
        
        if (nodes[level]->children[keyLevelIdx[level]] == NULL) {
            lastLevel = level;
            break;
        }
        nodes[level+1] = &((WideRadixNode*)nodes[level]->children[keyLevelIdx[level]])[keyLevelChild[level]];
    }

    if ((lastLevel == (tree->numLevels-1)) && (nodes[lastLevel]->children[keyLevelIdx[lastLevel]])) {
        if (keyLevelIdx[lastLevel] >= 4) {
            return 0;  // Safety check
        }
        value = ((uint64_t*)nodes[lastLevel]->children[keyLevelIdx[lastLevel]])[keyLevelChild[lastLevel]];
        ((uint64_t*)nodes[lastLevel]->children[keyLevelIdx[lastLevel]])[keyLevelChild[lastLevel]] = 0;
    }

    for (uint8_t level = lastLevel; level > 0; level--) {
        if (keyLevelIdx[level] >= 4) {
            continue;  // Safety check
        }
        
        nodes[level]->bits[keyLevelIdx[level]] &= ~(1ULL << keyLevelChild[level]);
        if (!nodes[level]->bits[keyLevelIdx[level]] && nodes[level]->children[keyLevelIdx[level]]) {
            objectPoolFree((level == (tree->numLevels - 1)) ? &tree->leafPool : &tree->nonLeafPool, nodes[level]->children[keyLevelIdx[level]]);
            nodes[level]->children[keyLevelIdx[level]] = NULL;
        }
        if (nodes[level]->bits[0] || nodes[level]->bits[1] || nodes[level]->bits[2] || nodes[level]->bits[3]) {
            break;
        }
    }

    return value;
}

void treeDestroy(WideRadixTree *tree) {
    if (tree) {
        objectPoolDestroy(&tree->nonLeafPool);
        objectPoolDestroy(&tree->leafPool);
        memset(tree, 0, sizeof(*tree));
    }
}