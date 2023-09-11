#pragma once

#include "defines.h"
#include "math_types.h"

#define L_PI 3.14159265358979323846f
#define L_2_PI 2.0f * L_PI
#define L_PI_2 0.5f * L_PI
#define L_PI_4 0.25f * L_PI 
#define L_PI_INV 1.0f / L_PI
#define L_2_PI_INV 1.0f / L_2_PI
#define L_SQRT_2 1.41421356237309504880f
#define L_SQRT_3 1.73205080756887729352f
#define L_SQRT_2_INV 0.70710678118654752440f
#define L_SQRT_3_INV 0.57735026918962576450f
#define L_DEG2RAD_FACTOR L_PI / 180.0f
#define L_RAD2DEG_FACTOR 180.0f / L_PI

// The factor to convert seconds to milliseconds
#define L_SEC_TO_MS_FACTOR 1000.0f

// The factor to convert milliseconds to seconds
#define L_MS_TO_SEC_FACTOR 0.001f

// A huge number to mimic infinity
#define L_INF 1e30f

// Smallest positive number where 1.0 + FLOAT_EPS != 1
#define L_FLOAT_EPS 1.192092896e-07f

// --------------------------------------
// General Math functions
// Use Taylor Expansions where possible
// --------------------------------------

LAPI f32 lsin(f32 x);
LAPI f32 lcos(f32 x);
LAPI f32 ltan(f32 x);
LAPI f32 lacos(f32 x);
LAPI f32 lsqrt(f32 x);
LAPI f32 labs(f32 x);

/**
 * @brief Indicates if the value is a power of 2. 0 is _not_ a power of 2.
 * @param value The value to be considered.
 * @returns True if value is a power of 2; False otherwise  
 */
LINLINE b8 is_power_of_2(u64 value) 
{
    return (value != 0) && ((value & (value - 1)) == 0);
}

LAPI i32 lrandom();
LAPI i32 lrandom_in_range(i32 min, i32 max);

LAPI f32 flrandom();
LAPI f32 flrandom_in_range(f32 min, f32 max);

// --------------------------------------
// Vector 2 
// --------------------------------------

/**
 * @brief Creates and retuns a new 2-element vector
 * using the supplied values.
 * 
 * @param x The x value.
 * @param y The y value.
 * @returns A new vec2.
 */
LINLINE vec2 make_vec2(f32 x, f32 y)
{
    vec2 out_vec;
    out_vec.x = x;
    out_vec.y = y;
    return out_vec;
}

/**
 * @brief Creates and returns a 2-component 0-vector
 * 
 */
LINLINE vec2 vec2_zero()
{
    return (vec2){0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 2-component vector populated with ones.
 * 
 */
LINLINE vec2 vec2_one() 
{
    return (vec2){1.0f, 1.0f};
}

/**
 * @brief Creates and returns a 2-component vector populated with the given value.
 * 
 */
LINLINE vec2 vec2_fill(f32 value)
{
    return (vec2){value, value};
}