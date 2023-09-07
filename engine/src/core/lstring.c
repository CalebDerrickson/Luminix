#include "core/lstring.h"
#include "core/lmemory.h"

#include <string.h>

char* string_duplicate(const char* str)
{
    u64 length = string_length(str);
    char* copy = lallocate(length + 1, MEMORY_TAG_STRING);
    lcopy_memory(copy, str, length + 1);
    return copy;
}

u64 string_length(const char* str)
{
    return strlen(str);
}