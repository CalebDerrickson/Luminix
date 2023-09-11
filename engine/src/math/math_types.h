#pragma once

#include "defines.h"

typedef union vec2 
{
    // An array of x, y
    f32 elements[2];
    struct
    {
        union 
        {
            // The first element.
            f32 x, r, s, u;
        };
        union 
        {
            // The second element.
            f32 y, g, t, v;
        };
    };
} vec2;

typedef union vec3 
{
    // An array of x, y, z.
    f32 elements[3];
    struct 
    {
        union 
        {
            // The first element
            f32 x, r, s, u;
        };
        union 
        {
            // The second element
            f32 y, g, t, v;
        };
        union 
        {
            // The third element
            f32 z, b, p, w;
        };
    };
} vec3;

typedef union vec4 
#if defined(LUSE_SIMD)
    // Used for SIMD operations
    alignas(16) __m128 data;
#endif
{
    // An array of x, y, z, t.
    alignas(16) f32 elements[4];
    struct 
    {
        union 
        {
            // The first element
            f32 x, r, s;
        };
        union 
        {
            // The second element
            f32 y, g, t;
        };
        union 
        {
            // The third element
            f32 z, b, p;
        };
        union
        {
            // The fourth element
            f32 w, a, q; 
        };
    };
} vec4;

// Quaternion definition
typedef vec4 quat;

typedef union mat4 {
    alignas(16) f32 data[16];

#if defined(LUSE_SIMD)
    // Used for SIMD operations
    alignas(16) vec4 rows[4];
#endif
} mat4;