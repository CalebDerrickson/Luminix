#pragma once 

#include "defines.h"
#include "math/math_types.h"

LAPI char* string_duplicate(const char* str);

LAPI u64 string_length(const char* str);

// Case sensitive string comparison, true if same, false otherwise
LAPI b8 strings_equal(const char* str1, const char* str2);

// Case insensitive string comparison, true if same, false otherwise
LAPI b8 strings_equali(const char* str1, const char* str2);

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

/**
 * @brief Empties the provided string by setting the first character to 0. 
 * 
 * @param str The string to be emptied
 * @return A pointer to the string
 */
LAPI char* string_empty(char* str);

LAPI char* string_copy(char* dest, const char* source);

LAPI char* string_ncopy(char* dest, const char* source, i64 length);

// Trims the white spaces off the ends of the given string
LAPI char* string_trim(char* str);

// Gets a substring from the given string
LAPI void string_mid(char* dest, const char* source, i32 start, i32 length);

/**
 * @brief Returns the index of the first occurance of 'c' in str; otherwise -1
 * 
 * @param str The string to be scanned.
 * @param c Teh character to search for.
 * 
 * @returns The index of the first occurance of 'c'; otherwise -1 if not found.
 */
LAPI i32 string_index_of(char* str, char c);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space delimited, ie "1.0 1.1 1.2 1.3"
 * @param out_vector A pointer to the vector to write to
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_vec4(char* str, vec4* out_vector);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space delimited, ie "1.0 1.1 1.2"
 * @param out_vector A pointer to the vector to write to
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_vec3(char* str, vec3* out_vector);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space delimited, ie "1.0 1.1"
 * @param out_vector A pointer to the vector to write to
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_vec2(char* str, vec2* out_vector);

/**
 * @brief Attempts to parse a 32-bit floating-point number from the provided string. 
 * 
 * @param str The string to parse from. Should 'not' be postfixed with 'f'.
 * @param f A pointer to the float to write to.
 * @return True if parsed successfully, false otherwise.
 */
LAPI b8 string_to_f32(char* str, f32* f);


/**
 * @brief Attempts to parse a 64-bit floating-point number from the provided string. 
 * 
 * @param str The string to parse from. Should 'not' be postfixed with 'f'.
 * @param f A pointer to the float to write to.
 * @return True if parsed successfully, false otherwise.
 */
LAPI b8 string_to_f64(char* str, f64* f);

/**
 * @brief Attempts to parse a 8-bit signed integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_i8(char* str, i8* i);

/**
 * @brief Attempts to parse a 16-bit signed integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_i16(char* str, i16* i);


/**
 * @brief Attempts to parse a 32-bit signed integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_i32(char* str, i32* i);


/**
 * @brief Attempts to parse a 64-bit signed integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_i64(char* str, i64* i);

/**
 * @brief Attempts to parse a 8-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_u8(char* str, u8* i);

/**
 * @brief Attempts to parse a 16-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_u16(char* str, u16* i);

/**
 * @brief Attempts to parse a 32-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_u32(char* str, u32* i);


/**
 * @brief Attempts to parse a 64-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully, false otherwise
 */
LAPI b8 string_to_u64(char* str, u64* i);

/**
 * @brief Attempts to parse a boolean from the provided string.
 * "true" or "1" are considered true; anything else is false
 * 
 * @param str The string to parse from. 
 * @param b A pointer to the boolean to write to.
 * @return True if parsed successfully, false otherwise. 
 */
LAPI b8 string_to_bool(char* str, b8* b);

