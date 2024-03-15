#pragma once

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types.
typedef float f32;
typedef double f64;

// Boolean types.
typedef int b32;
typedef _Bool b8;

// Define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Ensure all types are of the correct size.
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define true 1
#define false 0

/**
 * @brief Any id set to this should be considered invalid,
 * and not actually pointing to a real object
 */
#define INVALID_ID 4294967295U

// Maximum number of Indices, devices, and queue families
// Limiting the "magic numbers"
#define MAX_NUM_INDICES 32
#define MAX_DEVICE_COUNT 32
#define MAX_QUEUE_FAMILIES 32

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define LPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(_gnu_linux__)
#define LPLATFORM_LINUX 1
#if defined(__ANDROID__)
#define LPLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
#define LPLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
#define LPLATFORM_POSIX 1
#elif __APPLE__
#define LPLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#define LPLATFORM_IOS 1
#define LPLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define TARGET_OS_IPHONE
#define LPLATFORM_IOS 1
#elif TARGET_OS_MAC
#else
#error "Unknown Apple Platform"
#endif
#else
#error "Unknown Platform!"
#endif

#ifdef LEXPORT
// Exports
#ifdef _MSC_VER
#define LAPI __declspec(dllexport)
#else
#define LAPI __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define LAPI __declspec(dllimport)
#else
#define LAPI
#endif
#endif


#define LCLAMP(value, min, max) \
    (value <= min) ? min :      \
    (value >= max) ? max :      \
    value                       

// Inlining
#if defined(__clang__) || defined(__gcc__)
#define LINLINE __attribute__((always_inline)) inline
#define LNOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define LINLINE __forceinline
#define LNOINLINE __declspec(noinline)
#else
#define LINLINE static inline
#define LNOINLINE
#endif

/** @brief Gets the number of bytes from the amount of gibibytes (GiB) (1024*1024*1024)*/
#define GIBIBYTES(amount) amount * 1024 * 1024 * 1024

/** @brief Gets the number of bytes from the amount of mibibytes (MiB) (1024*1024)*/
#define MEBIBYTES(amount) amount * 1024 * 1024 

/** @brief Gets the number of bytes from the amount of kibibytes (KiB) (1024)*/
#define KIBIBYTES(amount) amount * 1024 

/** @brief Gets the number of bytes from the amount of gigabytes (GB) (1000*1000*1000)*/
#define GIGABYTES(amount) amount * 1000 * 1000 * 1000
/** @brief Gets the number of bytes from the amount of megabytes (MB) (1000*1000)*/
#define MEGABYTES(amount) amount * 1000 * 1000 
/** @brief Gets the number of bytes from the amount of kilobytes (KB) (1000)*/
#define KILABYTES(amount) amount * 1000