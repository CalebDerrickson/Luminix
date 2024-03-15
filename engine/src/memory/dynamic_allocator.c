#include "dynamic_allocator.h"

#include "core/lmemory.h"
#include "core/logger.h"
#include "containers/freelist.h"

typedef struct dynamic_allocator_state {
    u64 total_size;
    freelist list;
    void* freelist_block;
    void* memory_block;
} dynamic_allocator_state;


// Public function definitions


b8 dynamic_allocator_create(u64 total_size, u64* memory_requirement, void* memory, dynamic_allocator* out_allocator)
{
    if (total_size < 1) {
        LERROR("dynamic_allocator_create cannot have a total_size of 0. Create failed.");
        return false;
    }

    if(!memory_requirement) {
        LERROR("dynamic_allocator_create requires memory_requirement to exist. Create failed.");
        return false;
    }

    u64 freelist_requirement = 0;
    // Grad the memory requirement for the free list.
    freelist_create(total_size, &freelist_requirement, 0, 0);

    *memory_requirement = freelist_requirement + sizeof(dynamic_allocator_state) + total_size; 

    if (!memory) {
        // First pass.
        return true;
    }

    // Second pass. 
    
    // Memory layout:
    // state
    // freelist block
    // memory block

    out_allocator->memory = memory;

    // The cold cast is fine, as this is what we're actually pointing to.
    dynamic_allocator_state* state = out_allocator->memory;
    state->freelist_block = (void*) (out_allocator->memory + sizeof(dynamic_allocator_state));
    state->memory_block = (void*)(state->freelist_block + freelist_requirement);

    // Create the freelist.
    freelist_create(total_size, &freelist_requirement, state->freelist_block, &state->list);

    lzero_memory(state->memory_block, total_size);
    return true;
}

b8 dynamic_allocator_destroy(dynamic_allocator* allocator)
{
    if (!allocator) {
        LWARN("dynamic_allocator_destroy requires a pointer to an allocator. Destroy failed.");
        return false;
    }
    
    dynamic_allocator_state* state = allocator->memory;
    freelist_destroy(&state->list);
    lzero_memory(state->memory_block, state->total_size);
    state->total_size = 0;
    allocator->memory = 0;
    return true;
}

void* dynamic_allocator_allocate(dynamic_allocator* allocator, u64 size)
{
    if(!allocator || !size) {
        return 0;
    }

    dynamic_allocator_state* state = allocator->memory;
    u64 offset = 0;
    if(!freelist_allocate_block(&state->list, size, &offset)) {
        LERROR("dynamic_allocator_allocate no blocks of memory large enough to allocate!");
        u64 available = freelist_free_space(&state->list);
        LERROR("Requested size: %llu B, total space available: %llu B", size, available);
        // TODO: Report fragmentation? Should be on the job of the freelist.
        return 0;
    }

    // Use that offset against the base memory block to get the block.
    void* block = (void*)(state->memory_block + offset);
    return block;
}

b8 dynamic_allocator_free(dynamic_allocator* allocator, void* block, u64 size)
{
    if(!allocator || !block || !size) {
        LERROR("dynamic_allocator_free requires both a valid allocator (0x%p) and a block (0x%p) to be freed, as well as a nonzero size.", 
        allocator, block);
        return false;
    }

    dynamic_allocator_state* state = allocator->memory;
    if (block < state->memory_block || block > state->memory_block + state->total_size) {
        void* end_of_block = (void*)(state->memory_block + state->total_size);
        LERROR("dynamic_allocator_free trying to release block (0x%p) outside the allocator range: (0x%p) - (0x%p)", 
        block, state->memory_block, end_of_block);
        return false;
    }

    u64 offset = (block - state->memory_block);
    if(!freelist_free_block(&state->list, size, offset)) {
        LERROR("dynamic_allocator_free failed");
        return false;
    }

    return true;
}

u64 dynamic_allocator_free_space(dynamic_allocator* allocator)
{
    dynamic_allocator_state* state = allocator->memory;
    return freelist_free_space(&state->list);
}

