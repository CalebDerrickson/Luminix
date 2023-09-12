#include "lmemory.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "core/lstring.h"

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
    struct memory_stats stats;
    u64 alloc_count;
} memory_system_state;

static memory_system_state* state_ptr;

void initialize_memory(u64* memory_requirement, void* state)
{
    *memory_requirement = sizeof(memory_system_state);
    if(state == 0) {
        return;
    }
    state_ptr = state;
    state_ptr->alloc_count = 0;
    platform_zero_memory(&state_ptr->stats, sizeof(state_ptr->stats));
}

void shutdown_memory()
{
    state_ptr = 0;
}

void* lallocate(u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN){
        LWARN("lallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    if(state_ptr) {
        state_ptr->stats.total_allocated += size;
        state_ptr->stats.tagged_allocations[tag] += size;
        state_ptr->alloc_count++;
    }
    
    // TODO: memory alignment
    void* block = platform_allocate(size, false);
    platform_zero_memory(block, size);
    return block;
}

void lfree(void* block, u64 size, memory_tag tag) 
{
    if (tag == MEMORY_TAG_UNKNOWN){
        LWARN("lfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    // TODO: memory alignment
    if(state_ptr) {
    state_ptr->stats.total_allocated -= size;
    state_ptr->stats.tagged_allocations[tag] -= size;
    }
    platform_free(block, false);
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

    for (u32 it = MEMORY_TAG_UNKNOWN; it != MEMORY_TAG_MAX_TAGS; it++){

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