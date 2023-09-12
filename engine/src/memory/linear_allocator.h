#pragma once

#include "defines.h"

/*
Advantages of a linear memory allocator:

    Sequential Allocation: Allocates memory in a continuous, 
    linear fashion, which makes it easy to manage and track memory usage.

    Fast Allocation and Deallocation: Allocation and deallocation operations
    are typically fast since they involve simple pointer manipulation.

    Low Fragmentation: Linear allocators tend to have low memory fragmentation 
    because they allocate memory sequentially, reducing the chances of small 
    gaps between allocated blocks.

Disadvantages of a linear memory allocator:

    Limited Flexibility: Due to its sequential nature, it may not be suitable 
    for data structures that require frequent insertions or deletions, as it can 
    lead to inefficient memory usage.

    Fixed Size: The size of the allocated memory must be determined in advance, 
    which can be a limitation when dealing with varying data sizes.

    Cannot Reclaim Individual Blocks: Once a block of memory is allocated, it 
    cannot be deallocated individually; you must reset the entire allocator to 
    free all memory.

    Not Suitable for Complex Data Structures: Linear allocators are not ideal for 
    data structures with complex memory access patterns or dynamic resizing needs.
 
 */

typedef struct linear_allocator {
    u64 total_size;
    u64 allocated;
    void* memory;
    b8 owns_memory;
} linear_allocator;

LAPI void linear_allocator_create(u64 total_size, void* memory, linear_allocator* out_allocator);
LAPI void linear_allocator_destroy(linear_allocator* allocator);

LAPI void* linear_allocator_allocate(linear_allocator* allocator, u64 size);
LAPI void linear_allocator_free_all(linear_allocator* allocator);
