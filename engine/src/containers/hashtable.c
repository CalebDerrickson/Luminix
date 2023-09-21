#include "hashtable.h"

#include "core/lmemory.h"
#include "core/logger.h"

u64 hash_name(const char* name, u32 element_count) 
{
    // A multiplier to use when generating a hash. Prime to hopefully avoid collisions
    static const u64 multiplier = 97;

    unsigned const char* us;
    u64 hash = 0;

    for (us = (unsigned const char*)name; *us; us++) {
        hash = hash * multiplier + *us;
    }

    // Mode it against the size of the table
    hash %= element_count;

    return hash;
}

void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, hashtable* out_hashtable)
{
    if(!memory || !out_hashtable) {
        LERROR("hashtable_create failed! Pointer to memory and out_hashtable are required.");
        return;
    }

    if(!element_count || !element_size) {
        LERROR("element_size and element_count must be a positive non-zero-value.");
        return;
    }

    // TODO: Might want to require an allocator and allocate this memory instead.
    out_hashtable->memory = memory;
    out_hashtable->element_count = element_count;
    out_hashtable->element_size = element_size;
    out_hashtable->is_pointer_type = is_pointer_type;
    lzero_memory(out_hashtable->memory, element_size * element_count);
}

void hashtable_destroy(hashtable* table)
{
    if(table) {
        // TODO: If using allocator above, free memory here.
        lzero_memory(table, sizeof(hashtable));
    }
}

b8 hashtable_set(hashtable* table, const char* name, void* value)
{
    if(!table || !name || !value) {
        LERROR("hashtable_set requires table, name, and value to exist.");
        return false;
    }
    if(table->is_pointer_type) {
        LERROR("hashtable_set should not be used with tables that have pointer types. Use hastable_set_ptr instead.");
        return false;
    }

    u64 hash = hash_name(name, table->element_count);
    lcopy_memory(table->memory + (table->element_size * hash), value, table->element_size);
    return true;
}

b8 hashtable_set_ptr(hashtable* table, const char* name, void** value)
{
    if(!table || !name) {
        LERROR("hashtable_set_ptr requires table and name to exist.");
        return false;
    }
    if(!table->is_pointer_type) {
        LERROR("hashtable_set_ptr should not be used with tables that do not have pointer types. Use hastable_set instead.");
        return false;    
    }
    
    u64 hash = hash_name(name, table->element_count);
    ((void**)table->memory)[hash] = value ? *value : 0;
    return true;
}

b8 hashtable_get(hashtable* table, const char* name, void* out_value)
{
    if(!table || !name || !out_value) {
        LWARN("hashtable_get requires table, name, and out_value to exist.");
        return false;
    }
    if(table->is_pointer_type) {
        LERROR("hashtable_get should not be used with tables that have pointer types. Use hastable_get_ptr instead.");
        return false;
    }

    u64 hash = hash_name(name, table->element_count);
    lcopy_memory(out_value, table->memory + (table->element_size * hash), table->element_size);
    return true;
}

b8 hashtable_get_ptr(hashtable* table, const char* name, void** out_value)
{
    if(!table || !name || !out_value) {
        LWARN("hashtable_get_ptr requires table, name, and out_value to exist.");
        return false;
    }
    if(!table->is_pointer_type) {
        LERROR("hashtable_get_ptr should not be used with tables that do not have pointer types. Use hastable_get instead.");
        return false;
    }

    u64 hash = hash_name(name, table->element_count);
    *out_value = ((void**)table->memory)[hash];
    return *out_value != 0;
}

b8 hashtable_fill(hashtable* table, void* value)
{
    if(!table || !value) {
        LWARN("hashtable_fill requires table and value to exist.");
        return false;
    }
    if(table->is_pointer_type) {
        LERROR("hashtable_fill should not be used with tables that have pointer types.");
        return false;
    }

    for(u32 i = 0; i < table->element_count; i++) {
        lcopy_memory(table->memory + (table->element_size * i), value, table->element_size);
    }

    return true;
}

