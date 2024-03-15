#pragma once

#include "defines.h"

typedef enum memory_tag{
    // For temporary use. Should be assigned one of the below or have a new tag created
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NONE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
} memory_tag;

/** @brief The configuration for the memory system.*/
typedef struct memory_system_configuration {
    /** @brief The total memory size in bytes used by the internal allocator for this system. */
    u64 total_alloc_size;
} memory_system_configuration;


/**
 * @brief Initializes the memory system.
 * 
 * @param config The configuration for this system.
 * @return True if successful; false otherwise
 */
LAPI b8 memory_system_initialize(memory_system_configuration config);

/**
 * @brief Shuts down the memory system
 * 
 */
LAPI void memory_system_shutdown();

// These functions are designed to be called 
// out of the engine, so LAPI is intended here.

LAPI void* lallocate(u64 size, memory_tag tag);

LAPI void lfree(void* block, u64 size, memory_tag tag);

LAPI void* lzero_memory(void* black, u64 size);

LAPI void* lcopy_memory(void* dest, const void* source, u64 size);

LAPI void* lset_memory(void* dest, i32 value, u64 size);

// Print out usage statistics to the console.
// should be freed after use
LAPI char* get_memory_usage_str();

LAPI u64 get_memory_alloc_count();