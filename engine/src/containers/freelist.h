/**
 * @file freelist.h
 * @author name
 * @brief Definition of freelist, used for custom memory allocation tracking.
 * @version 0.1
 * @date 2024-03-14 
 * 
 */

#pragma once
#include "defines.h"

/**
 * @brief A data structure to be used alongside an allocator for dynamic memory
 * allocation. Tracks free ranges of memory.
 * 
 */
typedef struct freelist {
    /** @brief The internal state of the freelist. */
    void* memory;
} freelist;

/**
 * @brief Creates a new freelist or obtains the memory requirement for one. Should call twice (in line with vulkan style).
 * Should be first called, passing 0 to memory, to obtain memory requirement.
 * Should then be called, passing an allocated block to memory.
 * 
 * @param total_size The total size in bytes that the freelist should track.
 * @param memory_requirement A pointer to hold memory requirement to the freelist itself (the internal state!).
 * @param memory 0; or a pre-allocated block of memory for the free list to use (the intenal state!).
 * @param out_list  A pointer to hold the created free list.
 */
LAPI void freelist_create(u64 total_size, u64* memory_requirement, void* memory, freelist* out_list);


/**
 * @brief Destroys the provided list.
 * 
 * @param list The list to be destroyed.
 */
LAPI void freelist_destroy(freelist* list);

/**
 * @brief Attempts to find a free block of memory of the given size. Done by first fit.
 * 
 * @param list A pointer to the list to search.
 * @param size The size to allocate.
 * @param out_offset A pointer to hold the offset to the allocated memory.
 * @return b8 True if a block of memory was found and allocated; otherwise false
 */
LAPI b8 freelist_allocate_block(freelist* list, u64 size, u64* out_offset);

/**
 * @brief Attempts to find a free block of memory of the given size. Done by best fit.
 * TODO: Implement this
 * 
 * @param list A pointer to the list to search.
 * @param size The size to allocate.
 * @param out_offset A pointer to hold the offset to the allocated memory.
 * @return b8 True if a block of memory was found and allocated; otherwise false
 */
LAPI b8 freelist_allocate_block_best(freelist* list, u64 size, u64* out_offset);



/**
 * @brief Attempts to free a block of memory at the given offset, and of the given size. 
 * Can fail if invalid data is passed. 
 * 
 * @param list A pointer to the list from which to free.
 * @param size The size to be freed.
 * @param offset The offset to free at.
 * @return b8 True if successful; otherwise false. False should be treated as an error.
 */
LAPI b8 freelist_free_block(freelist* list, u64 size, u64 offset);

/**
 * @brief Clears the freelist
 * 
 * @param list The freelist to be cleared.
 */
LAPI void freelist_clear(freelist* list);

/**
 * @brief Returns the amount of free space in this list. 
 * NOTE: Since this has to iterate over the enitre internal list, 
 * this can be an expensive operation. Use sparingly
 * 
 * @param list A pointer tot he list from which to obtain.
 * @return The amount of free space in bytes
 */
LAPI u64 freelist_free_space(freelist* list);