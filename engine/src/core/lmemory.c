#include "lmemory.h"

#include "core/logger.h"
#include "platform/platform.h"
#include "core/lstring.h"
#include "memory/dynamic_allocator.h"

#include <stdio.h>

struct memory_stats{
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

// TODO: Simple for now, make a better implementation in the future
static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = 
{
    "UNKNOWN            ",
    "ARRAY              ",
    "LINEAR_ALLOCATOR   ",
    "DARRAY             ",
    "DICT               ",
    "RING_QUEUE         ",
    "BST                ",
    "STRING             ",
    "APPLICATION        ",
    "JOB                ",
    "TEXTURE            ",
    "MAT_INST           ",
    "RENDERER           ",
    "GAME               ",
    "TRANSFORM          ",
    "ENTITY             ",
    "ENTITY_MODE        ",
    "SCENE              "
};


typedef struct memory_system_state {
    memory_system_configuration config;
    struct memory_stats stats;
    u64 alloc_count;
    u64 allocator_memory_requirement;
    dynamic_allocator allocator;
    void* allocator_block;
} memory_system_state;

static memory_system_state* state_ptr;

b8 memory_system_initialize(memory_system_configuration config)
{
    // Amount needed to hold the state.
    u64 state_memory_requirement = sizeof(memory_system_state);

    // Calcuate space needed for the dynamic allocator.
    u64 alloc_requirement = 0;
    dynamic_allocator_create(config.total_alloc_size, &alloc_requirement, 0, 0);

    // Call the platform to get the memory for the whole systme, including state.
    // TODO: Memory alignment.
    void* block = platform_allocate(state_memory_requirement + alloc_requirement, false);

    if (!block) {
        LFATAL("Memory system allocation failed and the system cannot continue.");
        return false;
    }

    // The state is in the first part of the memory block.
    state_ptr = (memory_system_state*)block;
    state_ptr->config = config;
    state_ptr->alloc_count = 0;
    state_ptr->allocator_memory_requirement = alloc_requirement;
    platform_zero_memory(&state_ptr->stats, sizeof(state_ptr->stats));
    
    // Allocator block is after the system block
    state_ptr->allocator_block = ((void*)block + state_memory_requirement); 

    if (!dynamic_allocator_create(
        config.total_alloc_size,
        &state_ptr->allocator_memory_requirement,
        state_ptr->allocator_block,
        &state_ptr->allocator
    )) {
        LFATAL("Memory system is unable to setup internal allocator. Application cannot continue.");
        return false;
    }

    LINFO("Memory system successfully allocated %llu bytes.", config.total_alloc_size);
    return true;
}

void memory_system_shutdown()
{
    if (!state_ptr) {
        state_ptr = 0;
        return;
    } 

    dynamic_allocator_destroy(&state_ptr->allocator);

    // Free the block.
    platform_free(state_ptr, state_ptr->allocator_memory_requirement + sizeof(memory_system_state));
    state_ptr = 0;
}

void* lallocate(u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN){
        LWARN("lallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    // Check whether or not the memory system has been initialized. 
    // Will change how our allocation happens.
    void* block = 0;
    if (state_ptr) {
        state_ptr->stats.total_allocated += size;
        state_ptr->stats.tagged_allocations[tag] += size;
        state_ptr->alloc_count++;

        block = dynamic_allocator_allocate(&state_ptr->allocator, size);
    } else {
        // If the system is not up yet, warn about it for now.
        LWARN("lallocate called before the memroy system is initialized.");
        // TODO: Memory alignment;
        block = platform_allocate(size, false);
    }
    
    if (block) {
        platform_zero_memory(block, size);
        return block;
    }

    LFATAL("lallocate failed to allocate successfully.");
    return 0;
}

void lfree(void* block, u64 size, memory_tag tag) 
{
    if (tag == MEMORY_TAG_UNKNOWN){
        LWARN("lfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    // TODO: memory alignment
    if (state_ptr) {
        state_ptr->stats.total_allocated -= size;
        state_ptr->stats.tagged_allocations[tag] -= size;

        b8 res = dynamic_allocator_free(&state_ptr->allocator, block, size);

        // If the free fialed, it is possible that an allocation was made
        // before the system was statrted. Since this is an exception, try to
        // free it on a platform level. If this fails... dont want this to fail.

        if (!res) {
            // TODO: Memory alignment
            platform_free(block, false);
        } 
    } else {
        // TODO: Memory alignment
        platform_free(block, false);
    }

}

void* lzero_memory(void* block, u64 size)
{
    return platform_zero_memory(block, size);
}

void* lcopy_memory(void* dest, const void* source, u64 size)
{
    return platform_copy_memory(dest, source, size);
}

void* lset_memory(void* dest, i32 value, u64 size)
{
    return platform_set_memory(dest, value, size);
}

char* get_memory_usage_str()
{
    const u64 kib = 1024;
    const u64 mib = kib * kib;
    const u64 gib = mib * kib;

    char buffer[8000] = "System memory use (tagged): \n";
    u64 offset = string_length(buffer);

    for (u32 it = MEMORY_TAG_UNKNOWN; it != MEMORY_TAG_MAX_TAGS; ++it){

        char unit[4] = "XiB";
        f32 amount = 1.0f;
        if (state_ptr->stats.tagged_allocations[it] >= gib) {
            unit[0] = 'G';
            amount = state_ptr->stats.tagged_allocations[it] / (f32) gib;
        }
        else if (state_ptr->stats.tagged_allocations[it] >= mib) {
            unit[0] = 'M';
            amount = state_ptr->stats.tagged_allocations[it] / (f32) mib;    
        }
        else if (state_ptr->stats.tagged_allocations[it] >= kib) {
            unit[0] = 'K';
            amount = state_ptr->stats.tagged_allocations[it] / (f32) kib;    
        }
        else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)state_ptr->stats.tagged_allocations[it];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memory_tag_strings[it], amount, unit);   
        offset += length;
    }

    char* out_string = string_duplicate(buffer);
    return out_string;
}

u64 get_memory_alloc_count()
{
    return (state_ptr) ? state_ptr->alloc_count : 0;
}
