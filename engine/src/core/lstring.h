#pragma once 

#include "defines.h"

LAPI char* string_duplicate(const char* str);

LAPI u64 string_length(const char* str);

// Case sensitive string comparison, true if same, false otherwise
LAPI b8 strings_equal(const char* str1, const char* str2);

// Performs string formatting to dest given format string and parameters
LAPI i32 string_format(char* dest, const char* format, ...);

/**
 * @brief Performs variable string formatting to dest given format string and va_list.
 * 
 * @param dest The destination for the formatted string.
 * @param format The string to be formatted.
 * @param va_listp The variable argument list.
 * @return The size of the data written. 
 */
LAPI i32 string_format_v(char* dest, const char* format, void* va_listp);