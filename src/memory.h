#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

// Allocates an array on the heap.
#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, 0, sizeof(type) * count)

// Resizes a allocation down to zero bytes.
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// Calcs a new capacity based on given current capacity. It grows in factor of two because it's efficient and typical. 1.5x it's another obtion.
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount)      \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

// This one frees the memory by passing in zero for the new size.
#define FREE_ARRAY(type, pointer, oldCount) \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), 0)

// Used for all dynamic memory management, allocatig memory, freeing it, and changing the size of an existing allocation.
// 0, Non-zero - Allowcate a new block.
// Non-zero, 0 - Free Allocation.
// Non‑zero, Smaller than oldSize - Shrink existing allocation.
// Non‑zero, Larger than oldSize - Grow existing allocation.
void *reallocate(void *pointer, size_t oldSize, size_t newSize);
void collectGarbage();
void freeObjects();

#endif