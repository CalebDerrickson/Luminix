#include "core/lstring.h"
#include "core/lmemory.h"
#include "lstring.h"

#ifndef _MSC_VER
#include <strings.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

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

b8 strings_equal(const char* str1, const char* str2)
{
    return strcmp(str1, str2) == 0;
}

b8 strings_equali(const char* str1, const char* str2)
{
#if defined(__GNUC__)
    return strcasecmp(str1, str2) == 0;
#elif (defined _MSC_VER)
    return _strcmpi(str1, str2);
#endif
}

i32 string_format(char *dest, const char *format, ...) 
{
    if (!dest) {
        return -1;
    }

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, format);
    i32 written = string_format_v(dest, format, arg_ptr);
    va_end(arg_ptr);
    return written;
}

i32 string_format_v(char *dest, const char *format, void *va_listp) 
{
    if (!dest) {
        return -1;
    }

    // Big, but can fit on stack.
    char buffer[32000];
    i32 written = vsnprintf(buffer, 32000, format, va_listp);
    buffer[written] = 0;
    lcopy_memory(dest, buffer, written + 1);

    return written;
}