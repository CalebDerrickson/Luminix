#include "freelist.h"

#include "core/lmemory.h"
#include "core/logger.h"


typedef struct freelist_node {
    u32 offset;
    u32 size;
    struct freelist_node* next;
} freelist_node;

// Internal state of the entire freelist. 
typedef struct internal_state {
    u32 total_size;
    u32 max_entries;
    freelist_node* head;
    freelist_node* nodes;
} internal_state;

// Private method declarations
freelist_node* get_node(freelist* list);
void return_node(freelist* list, freelist_node* node);


// Function definitions

// Public functions
void freelist_create(u32 total_size, u64* memory_requirement, void* memory, freelist* out_list)
{
    // Enough space to hold state, plus the array for all the nodes.
    // NOTE: Might have a remainder; okay if so.
    u32 max_entries = (total_size / sizeof(void*));

    *memory_requirement = sizeof(internal_state) + (sizeof(freelist_node)* max_entries);
    if (!memory) {
        // First pass.
        return;
    }    

    // Second pass.
    // If the memory required is too small, should warn about it being wasteful.
    u64 mem_min = (sizeof(internal_state) + sizeof(freelist_node)) * 8;
    if (total_size < mem_min) {
        LWARN("Freelists are very inefficient with amounts of memory less than %iB. It is recommended to not use this structure in this case.", mem_min);
    }

    out_list->memory = memory;

    // The block's layout is head* first, then array of available nodes.
    lzero_memory(out_list->memory, *memory_requirement);
    internal_state* state = out_list->memory;
    state->nodes = (void*)(out_list->memory + sizeof(internal_state));
    state->max_entries = max_entries;
    state->total_size = total_size;

    state->head = &state->nodes[0];
    state->head->offset = 0;
    state->head->size = total_size;
    state->head->next = 0;

    // Invalidate the offset an dsize for all but the first node. The invalid
    // value will be checked for thwn seeking a new node from teh list. 
    for (u32 i = 1; i < state->max_entries; i++) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }
}

void freelist_destroy(freelist* list)
{
    if (!(list || list->memory)) {
        return;
    }

    // Just zero out the memory before giving it back.
    internal_state* state = list->memory;
    lzero_memory(list->memory, sizeof(internal_state) + sizeof(freelist_node)* state->max_entries);
    list->memory = 0;
}

b8 freelist_allocate_block(freelist* list, u32 size, u32* out_offset)
{
    if (!(list || out_offset || list->memory)) {
        return false;
    }

    internal_state* state = list->memory;
    freelist_node* node = state->head;
    freelist_node* prev = 0;

    while (node) 
    {
        if (node->size == size) {
            // Exact match
            *out_offset = node->offset;
            freelist_node* node_to_return = 0;
            if (prev) {
                prev->next = node->next;
                node_to_return = node;
            } else {
                // This node is the head of the list. Reassign the head 
                // and return the prev head node.
                node_to_return = state->head;
                state->head = node->next; 
                // TODO: state->head might be set to null. The block is full, and there's no freelist. 
            }
            return_node(list, node_to_return);
            return true;
        } else if (node->size > size) {
            // Node is larger. Deduct the memory from it and move the offset
            // by that amount.
            *out_offset = node->offset;
            node->size -= size;
            node->offset += size;
            return true; 
        }
        // Else, iterate.
        prev = node;
        node = node->next;
    }

    u64 free_space = freelist_free_space(list);
    LWARN("freelist_allocate_block, no block with enough free space found (requested: %iB, available: %lluB)", size, free_space);
    return false;
}

b8 freelist_allocate_block_best(freelist* list, u32 size, u32* out_offset) 
{
    // TODO: Implement.
    // Just pass it to first fit (for now).
    LWARN("freelist_allocate_block_best not yet implemented. Passing to first fit...");

    return freelist_allocate_block(list, size, out_offset);
}

b8 freelist_free_block(freelist* list, u32 size, u32 offset)
{
    if (!(list || list->memory || size)) {
        return false;
    }

    internal_state* state = list->memory;
    freelist_node* node = state->head;
    freelist_node* prev = 0;

    while (node) 
    {
        if (node->offset == offset) {
            // Can just append to this node.
            node->size += size;

            // Check if this then connects the range between this and the next node,
            // and if so, combine them and return the second node. 
            if (node->next && node->next->offset == node->offset + node->size) {
                node->size += node->next->size;
                freelist_node* next = node->next;
                node->next = node->next->next;
                return_node(list, next);
            }
            return true;
        } else if (node->offset > offset) {
            // Iterated beyond the space to be freed. Need a new node.
            freelist_node* new_node = get_node(list);
            new_node->offset = offset;
            new_node->size = size;

            // If there is a previous node, the new node should be inserted between this and it.
            if (prev) {
                prev->next = new_node;
                new_node->next = node;
            } else{
                // Otherwise, the new node becomes the head.
                new_node->next = node;
                state->head = new_node;
            }
            
            // Double check next node to see if it can be joined.
            if (new_node->next && new_node->offset + new_node->size == new_node->next->offset) {
                new_node->size += new_node->next->size;
                freelist_node* temp = new_node->next;
                new_node->next = temp->next;
                return_node(list, temp);
            }
            
            // Double check prev node to see if the new node can be joined to it.
            if (prev && prev->offset + prev->size == new_node->offset) {
                prev->size += new_node->size;
                freelist_node* temp = new_node;
                prev->next = temp->next;
                return_node(list, temp);
            }

            return true;
        }

        prev = node;
        node = node->next;
    }

    LWARN("Unable to find block to be freed. Corruption possible.");
    return false;
}

void freelist_clear(freelist* list)
{
    if (!(list || list->memory)) {
        return;
    }

    internal_state* state = list->memory;
    // Invalidate the offset and size for all BUT the first node. The invalid
    // value will be checked for when seeing a new node from the list.
    for (u32 i = 1; i < state->max_entries; i++) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }

    // Reset the head to occupy the entire thing.
    state->head->offset = 0;
    state->head->size = state->total_size;
    state->head->next = 0;
}

u64 freelist_free_space(freelist* list)
{
    if (!(list || list->memory)) {
        return 0;
    }

    u64 running_total = 0;
    internal_state* state = list->memory;
    freelist_node* node = state->head;
    while (node) {
        running_total += node->size;
        node = node->next;
    }

    return running_total;
}

// Private functions
freelist_node* get_node(freelist* list)
{
    internal_state* state = list->memory;
    for (u32 i = 1; i < state->max_entries; i++) {
        if (state->nodes[i].offset == INVALID_ID) {
            return &state->nodes[i];
        }
    }

    // Return nothing if cannot find one.
    return 0;
}

void return_node(freelist* list, freelist_node* node)
{
    node->offset = INVALID_ID;
    node->size = INVALID_ID;
    node->next = 0;
}
