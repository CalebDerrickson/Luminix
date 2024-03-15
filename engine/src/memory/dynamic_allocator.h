/**
 * @file dynamic_allocator.h
 * 
 * @brief Contains the implementations of the dynamic allocator. 
 * @version 0.1
 * @date 2024-03-14
 * 
 */

#pragma once
#include "defines.h"

/** @brief The dynamic allocator stucture.*/
typedef struct dynamic_allocator
{
    /** @brief The allocated memory block for this allocator to use.*/
    void* memory;
} dynamic_allocator; 

/**
 * @brief Creates a new dynamic allocator. Should be called twice; once to obtain the desired memory 
 * amount (passing 0 for memory), then a second time to set the allocated block.
 * 
 * @param total_size The total size in bytes the allocator should hold. Note that this size does not include the size of the internal state.
 * @param memory_requirement A pointer to hold the required memory for the internam state PLUS total_size.
 * @param memory An allocated block of memory.
 * @param out_allocator A pointer to hold the allocator.
 * @return True if success; false otherwise.
 */
LAPI b8 dynamic_allocator_create(u64 total_size, u64* memory_requirement, void* memory, dynamic_allocator* out_allocator);

/**
 * @brief Destroys the given allocator.
 * 
 * @param allocator A pointer to the allocator to be destroyed. 
 * @return True if success; false otherwise.
 */
LAPI b8 dynamic_allocator_destroy(dynamic_allocator* allocator);

/**
 * @brief Allocates the given amount of memory from the provided allocator.
 * 
 * @param allocator A pointer to the allocator to allocate from.
 * @param size The amount in bytes to be allocated.
 * @return The allocated block of memory unless it fails; false otherwise.
 */
LAPI void* dynamic_allocator_allocate(dynamic_allocator* allocator, u64 size);

/**
 * @brief Frees the given block of memory.
 * 
 * @param allocator A pointer to the allocatro to free from.
 * @param block The block to be freed. Must have been allocated by the provided allocator.
 * @param size The size of the block.
 * @return True if success; false otherwise.
 */
LAPI b8 dynamic_allocator_free(dynamic_allocator* allocator, void* block, u64 size);

/**
 * @brief Obtains the amount of free space left in the provided allocator.
 * 
 * @param allocator A pointer to the allocator to be examined. 
 * @return The amount of free space in bytes.
 */
LAPI u64 dynamic_allocator_free_space(dynamic_allocator* allocator);