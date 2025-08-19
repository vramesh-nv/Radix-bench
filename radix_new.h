#ifndef _RADIX_NEW_H
#define _RADIX_NEW_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Individual pool structure
typedef struct ObjectPoolBlock_st {
    void* pool;           // Pointer to the allocated memory pool
    size_t objectSize;    // Size of each object in the pool
    size_t capacity;      // Total number of objects that can be allocated
    size_t used;          // Number of currently allocated objects
    void** freeList;      // Stack of free object pointers
    size_t freeListTop;   // Top of the free list stack
    struct ObjectPoolBlock_st* next;  // Next pool in the chain
} ObjectPoolBlock;

// ObjectPool structure for efficient object allocation
typedef struct ObjectPool_st {
    size_t objectSize;    // Size of each object in the pool
    size_t initialCapacity; // Initial capacity for new pools
    ObjectPoolBlock* pools; // Linked list of pools
    ObjectPoolBlock* currentPool; // Current pool being used for allocations
} ObjectPool;

// Function declarations for ObjectPool
int objectPoolInit(ObjectPool* pool, size_t objectSize, size_t initialCapacity);
void objectPoolDestroy(ObjectPool* pool);
void* objectPoolAlloc(ObjectPool* pool, void** result);
void objectPoolFree(ObjectPool* pool, void* obj);

typedef struct WideRadixNode_st {
    uint64_t bits[4];
    void* children[4];
} WideRadixNode;

typedef struct WideRadixTree_st {
    WideRadixNode root;
    uint8_t numLevels;
    ObjectPool nonLeafPool;
    ObjectPool leafPool;
} WideRadixTree;

// Function declarations for the radix tree
void treeInit(WideRadixTree *tree, uint8_t log2Max, uint8_t log2Align);
int treeInsertOrReturnExisting(WideRadixTree *tree, uint64_t key, uint64_t value, uint64_t *existing);
uint64_t treeFind(WideRadixTree *tree, uint64_t key);
uint64_t treeFindGEQ(WideRadixTree *tree, uint64_t key);
uint64_t treeRemove(WideRadixTree *tree, uint64_t key);
void treeDestroy(WideRadixTree *tree);

#endif