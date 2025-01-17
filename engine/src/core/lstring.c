#include "core/lstring.h"
#include "core/lmemory.h"
#include "lstring.h"

#ifndef _MSC_VER
#include <strings.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>  //isspace

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
    return _strcmpi(str1, str2) == 0;
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

char* string_empty(char* str)
{
    if (str) {
        str[0] = 0;
    }
    return str;
}

char* string_copy(char* dest, const char* source)
{
    return strcpy(dest, source);

}

char* string_ncopy(char* dest, const char* source, i64 length)
{
    return strncpy(dest, source, length);

}

char* string_trim(char* str)
{
    while(isspace((unsigned char)*str)) {
        str++;
    }

    if (*str) {
        char* p = str;
        while(*p) {
            p++;
        }
        while(isspace((unsigned char)*(--p)))
            ;
        p[1] = '\0';
    }

    return str;
}

void string_mid(char* dest, const char* source, i32 start, i32 length)
{
    if (length == 0) {
        return;
    }
    u64 src_length = string_length(source);
    if (start >= src_length) {
        dest[0] = 0;
        return;
    }
    if (length > 0) {
        for (u64 i = start, j = 0; j < length && source[i]; ++i, ++j){
            dest[j] = source[i];
        }
        dest[start + length] = 0;
    } else {
        // If a negative value is passed, proceed to the end of the string.
        u64 j = 0;
        for (u64 i = start; source[i]; ++i, ++j) {
            dest[j] = source[i];
        }
        dest[start + j] = 0;
    }
}

i32 string_index_of(char* str, char c)
{
    if (!str) {
        return -1;
    }
    u32 length = string_length(str);
    if (length > 0) {
        for (u32 i = 0; i < length; ++i) {
            if (str[i] == c) {
                return i;
            } 
        }
    }

    return -1;
}

b8 string_to_vec4(char* str, vec4* out_vector)
{
    if (!str) {
        return false;
    }

    lzero_memory(out_vector, sizeof(vec4));
    i32 result = sscanf(str, "%f %f %f %f", &out_vector->x, &out_vector->y, &out_vector->z, &out_vector->w);
    return result != -1;
}

b8 string_to_vec3(char* str, vec3* out_vector)
{
    if (!str) {
        return false;
    }

    lzero_memory(out_vector, sizeof(vec3));
    i32 result = sscanf(str, "%f %f %f", &out_vector->x, &out_vector->y, &out_vector->z);
    return result != -1;
}

b8 string_to_vec2(char* str, vec2* out_vector)
{
    if (!str) {
        return false;
    }

    lzero_memory(out_vector, sizeof(vec2));
    i32 result = sscanf(str, "%f %f", &out_vector->x, &out_vector->y);
    return result != -1;
}

b8 string_to_f32(char* str, f32* f)
{
    if (!str) {
        return false;
    }

    *f = 0;
    i32 result = sscanf(str, "%f", f);
    return result != -1;
}

b8 string_to_f64(char* str, f64* f)
{
    if (!str) {
        return false;
    }

    *f = 0;
    i32 result = sscanf(str, "%lf", f);
    return result != -1;
}

b8 string_to_i8(char* str, i8* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%hhi", i);
    return result != -1;
}

b8 string_to_i16(char* str, i16* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%hi", i);
    return result != -1;
}

b8 string_to_i32(char* str, i32* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%i", i);
    return result != -1;
}

b8 string_to_i64(char* str, i64* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%lli", i);
    return result != -1;
}

b8 string_to_u8(char* str, u8* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%hhu", i);
    return result != -1;
}

b8 string_to_u16(char* str, u16* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%hu", i);
    return result != -1;
}

b8 string_to_u32(char* str, u32* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%u", i);
    return result != -1;
}

b8 string_to_u64(char* str, u64* i)
{
    if (!str) {
        return false;
    }

    *i = 0;
    i32 result = sscanf(str, "%llu", i);
    return result != -1;
}

b8 string_to_bool(char* str, b8* b)
{
    if (!str) {
        return false;
    }

    return strings_equal(str, "i") || strings_equali(str, "true");
}
