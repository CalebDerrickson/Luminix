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
    "UNKNOWN    ",
    "ARRAY      ",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_MODE",
    "SCENE      "
};

static struct memory_stats stats;

void initialize_memory()
{
    platform_zero_memory(&stats, sizeof(stats));
}

void shutdown_memory()
{
    // Empty for now, in place for memory stats
}

void* lallocate(u64 size, memory_tag tag)
{
    if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("lallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    // TODO: memory alignment
    void* block = platform_allocate(size, FALSE);
    platform_zero_memory(block, size);
    return block;
}

void lfree(void* block, u64 size, memory_tag tag) 
{
    if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("lfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    // TODO: memory alignment
    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;
    platform_free(block, FALSE);
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

    for(u32 it = MEMORY_TAG_UNKNOWN; it != MEMORY_TAG_MAX_TAGS; it++){

        char unit[4] = "XiB";
        f32 amount = 1.0f;
        if(stats.tagged_allocations[it] >= gib) {
            unit[0] = 'G';
            amount = stats.tagged_allocations[it] / (f32) gib;
        }
        else if (stats.tagged_allocations[it] >= mib) {
            unit[0] = 'M';
            amount = stats.tagged_allocations[it] / (f32) mib;    
        }
        else if (stats.tagged_allocations[it] >= kib) {
            unit[0] = 'K';
            amount = stats.tagged_allocations[it] / (f32) kib;    
        }
        else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)stats.tagged_allocations[it];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memory_tag_strings[it], amount, unit);   
        offset += length;
    }

    char* out_string = string_duplicate(buffer);
    return out_string;
}