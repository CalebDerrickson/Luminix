#pragma once 

#include "defines.h"

LAPI char* string_duplicate(const char* str);

LAPI u64 string_length(const char* str);

// Case sensitive string comparison, true if same, false otherwise
LAPI b8 strings_equal(const char* str1, const char* str2);