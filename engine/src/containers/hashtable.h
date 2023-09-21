#pragma once

#include "defines.h"

/**
 * @brief Represents a simple hashtable. Members of this structure
 * should not be modified outside the functions associated with it.
 * 
 * For non-pointer types, table retains a copy of the value. For
 * pointer types, make sure to use the _ptr setter and getter. Table
 * does not take ownership of pointers or associated memory allocations,
 * and should be managed externally.
 */
typedef struct hashtable {
    u64 element_size;
    u32 element_count;
    b8 is_pointer_type;
    void* memory;
} hashtable;

/**
 * @brief Creates a hashtable and stores it in out_hashtable 
 * 
 * @param element_size The size of each element in bytes.
 * @param element_count The maximum number of elements. Cannot be resized.
 * @param memory A block of memory to be used. Should equal in size to element_size * element_count.
 * @param is_pointer_type Indicates if this hashtable will hold pointer types.
 * @param out_hashtable A pointer to a hashtable in which to hold relevant data.
 */
LAPI void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, hashtable* out_hashtable);

/**
 * @brief Destroys the provided hashtable. Does not release memory for pointer types
 * 
 * @param table A pointer to the table to be destroyed.
 */
LAPI void hashtable_destroy(hashtable* table);

/**
 * @brief Stores a copy of the data in value in the provided hashtable.
 * Only use for tables which were 'NOT' created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value The value to be set. Required.
 * @return True, or false if a null pointer is passed.
 */
LAPI b8 hashtable_set(hashtable* table, const char* name, void* value);

/**
 * @brief Stores a pointer of the data in value in the provided hashtable.
 * Only use for tables which were created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value The value to be set. Can pass 0 to 'unset' an entry.
 * @return True, or false if a null pointer is passed or if the entry is 0.
 */
LAPI b8 hashtable_set_ptr(hashtable* table, const char* name, void** value);

/**
 * @brief Obtains a copy of the data present in the provided hashtable.
 * Only use for tables which were 'NOT' created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param out_value The value to be set. Required.
 * @return True, or false if a null pointer is passed.
 */
LAPI b8 hashtable_get(hashtable* table, const char* name, void* out_value);

/**
 * @brief Obtains a copy of the data present in the provided hashtable.
 * Only use for tables which were created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value The value to be set. Can pass 0 to 'unset' an entry.
 * @return True, or false if a null pointer is passed or if the entry is 0.
 */
LAPI b8 hashtable_get_ptr(hashtable* table, const char* name, void** out_value);

/**
 * @brief Fills all entries in the hashtable with the given value.
 * Useful when non-existent names should return some default value.
 * Should not be used with pointer table types.
 * 
 * @param table A pointer to the table to be filled. Required.
 * @param value The value with which to fill the table. Required.
 * @return True if successful; false otherwise
 */
LAPI b8 hashtable_fill(hashtable* table, void* value);