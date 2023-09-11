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
#define L_EPS 1.192092896e-07f

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
LINLINE vec2 vec2_make(f32 x, f32 y)
{
    return (vec2){x, y};
}

/**
 * @brief Creates and returns a 2-component vector populated with the given value. 
 */
LINLINE vec2 vec2_set(f32 value)
{
    return (vec2){value, value};
}

/**
 * @brief Creates and returns a 2-component vector pointing up. 
 */
LINLINE vec2 vec2_up()
{
    return (vec2){0.0f, 1.0f};
}

/**
 * @brief Creates and returns a 2-component vector pointing down. 
 */
LINLINE vec2 vec2_down()
{
    return (vec2){0.0f, -1.0f};
}

/**
 * @brief Creates and returns a 2-component vector pointing left. 
 */
LINLINE vec2 vec2_left()
{
    return (vec2){-1.0f, 0.0f};
}

/**
 * @brief Creates and returns a 2-component vector pointing right. 
 */
LINLINE vec2 vec2_right()
{
    return (vec2){1.0f, 0.0f};
}

/**
 * @brief Adds vector_1 and vector_2 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec2 vec2_add(vec2 vector_1, vec2 vector_2)
{
    return (vec2){
        vector_1.x + vector_2.x,
        vector_1.y + vector_2.y
    };
}

/**
 * @brief Subtracts vector_2 from vector_1 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec2 vec2_subtract(vec2 vector_1, vec2 vector_2)
{
    return (vec2){
        vector_1.x - vector_2.x,
        vector_1.y - vector_2.y
    };
}

/**
 * @brief Multiplies vector_1 and vector_2 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec2 vec2_mult(vec2 vector_1, vec2 vector_2)
{
    return (vec2){
        vector_1.x * vector_2.x,
        vector_1.y * vector_2.y
    };
}

/**
 * @brief Divides vector_2 from vector_1 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec2 vec2_div(vec2 vector_1, vec2 vector_2)
{
    return (vec2){
        vector_1.x / vector_2.x,
        vector_1.y / vector_2.y
    };
}

/**
 * @brief Dots vector_1 and vector_2 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The scalar product.
 */
LINLINE f32 vec2_dot(vec2 vector_1, vec2 vector_2)
{
    return vector_1.x * vector_2.x + 
           vector_1.y * vector_2.y;
}

/**
 * @brief Crosses vector_2 to vector_1 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec3 vec2_cross(vec2 vector_1, vec2 vector_2)
{
    f32 term1 = vector_1.x * vector_2.y;
    f32 term2 = vector_2.x * vector_1.y;
    return (vec3){0.0f, 0.0f, term1 - term2};
}

/**
 * @brief Returns the length of the vector
 * @param vector The given vector
 * @returns Its length squared
 */
LINLINE f32 vec2_length(vec2 vector)
{
    return lsqrt(vec2_dot(vector, vector));
}

/**
 * @brief Normalizes the provided vector
 * @param vector A pointer to the vector to be normalized.
 * @return The normalized vector
 */
LINLINE void vec2_normalize(vec2* vector)
{
    const f32 length = vec2_dot(*vector, *vector);
    vector->x /= length;
    vector->y /= length;
}

/**
 * @brief Returns a normalized copy of the provided vector
 * @param vector A pointer to the vector to be normalized.
 * @return A normalized copy of the provided vector.
 */
LINLINE vec2 vec2_normalized(vec2 vector)
{
    vec2_normalize(&vector);
    return vector;
}

/**
 * @brief Compares vector_1 and vector_2 and ensures the difference is less than tolerance
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @param tolerance The difference tolerance. Typically L_EPS or similar
 * @return True if within tolerance; false otherwise
 */
LINLINE b8 vec2_compare(vec2 vector_1, vec2 vector_2, f32 tolerance)
{
    if(labs(vector_1.x - vector_2.x) > tolerance) {
        return FALSE;
    }
    if(labs(vector_1.y - vector_2.y) > tolerance) {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Returns the distance between vector_1 and vector_2 
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The distance between them
 */
LINLINE f32 vec2_distance(vec2 vector_1, vec2 vector_2)
{
    vec2 d = (vec2){
        vector_1.x - vector_2.x,
        vector_1.y - vector_2.y
    };
    return vec2_length(d);
}

/**
 * @brief Multiplies all elements of vector by a scalar
 * @param vector The vector
 * @param scalar The scalar
 * @returns A copy of the resulting vector.
 */
LINLINE vec2 vec2_mul_scalar(vec2 vector, f32 scalar)
{
    return (vec2){
        vector.x * scalar,
        vector.y * scalar
    };
}


// --------------------------------------
// Vector 3
// --------------------------------------


/**
 * @brief Creates and returns a new 3-element vector using the supplied values.
 * 
 * @param x The x value.
 * @param y The y value.
 * @param z The z value.
 * 
 * @returns A Vector-3 of the provided values.
 */
LINLINE vec3 vec3_make(f32 x, f32 y, f32 z)
{
    return (vec3){x, y, z};
}

/**
 * @brief Creates and returns a new 3-element vector using the supplied value.
 * @param value The provided value.
 * 
 * @returns A Vector-3 of the provided value.
 */
LINLINE vec3 vec3_set(f32 value)
{
    return (vec3){value, value, value};
}

/**
 * @brief Returns a new vec4 using vector as the x, y, and z components and w for w
 * @param vector The 3-component vector.
 * @param w The w component.
 * @returns A new vec4
 */
LINLINE vec4 vec3_to_vec4(vec3 vector, f32 w)
{
    return (vec4){vector.x, vector.y, vector.z, w};
}

/**
 * @brief Creates and returns a 3-component vector pointing up (0, 1, 0). 
 */
LINLINE vec3 vec3_up()
{
    return (vec3){0.0f, 1.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing down (0, -1, 0). 
 */
LINLINE vec3 vec3_down()
{
    return (vec3){0.0f, -1.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing left (-1, 0, 0). 
 */
LINLINE vec3 vec3_left()
{
    return (vec3){-1.0f, 0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing right (1, 0, 0). 
 */
LINLINE vec3 vec3_right()
{
    return (vec3){1.0f, 0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing forward (0, 0, -1). 
 */
LINLINE vec3 vec3_forward()
{
    return (vec3){0.0f, 0.0f, -1.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing backward (0, 0, 1). 
 */
LINLINE vec3 vec3_backward()
{
    return (vec3){0.0f, -1.0f, 0.0f};
}

/**
 * @brief Adds vector_1 and vector_2 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec3 vec3_add(vec3 vector_1, vec3 vector_2)
{
    return (vec3){
        vector_1.x + vector_2.x,
        vector_1.y + vector_2.y,
        vector_1.z + vector_2.z,
    };
}

/**
 * @brief Subtracts vector_2 from vector_1 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec3 vec3_subtract(vec3 vector_1, vec3 vector_2)
{
    return (vec3){
        vector_1.x - vector_2.x,
        vector_1.y - vector_2.y,
        vector_1.z - vector_2.z,
    };
}

/**
 * @brief Multiplies vector_1 and vector_2 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec3 vec3_mult(vec3 vector_1, vec3 vector_2)
{
    return (vec3){
        vector_1.x * vector_2.x,
        vector_1.y * vector_2.y,
        vector_1.z * vector_2.z,
    };
}

/**
 * @brief Divides vector_2 from vector_1 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec3 vec3_div(vec3 vector_1, vec3 vector_2)
{
    return (vec3){
        vector_1.x / vector_2.x,
        vector_1.y / vector_2.y,
        vector_1.z / vector_2.z,
    };
}


/**
 * @brief Returns the cross product of vector_1 and vector_2 
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec3 vec3_cross(vec3 vector_1, vec3 vector_2)
{
    f32 term1 = vector_1.y * vector_2.z - vector_1.z * vector_2.y;
    f32 term2 = vector_1.x * vector_2.z - vector_1.z * vector_2.x;
    f32 term3 = vector_1.x * vector_2.y - vector_1.y * vector_2.x;
    return (vec3){term1, term2, term3};
}

/**
 * @brief Returns the dot product of vector_1 and vector_2 
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting scalar.
 */
LINLINE f32 vec3_dot(vec3 vector_1, vec3 vector_2)
{
    return vector_1.x * vector_2.x +
           vector_1.y * vector_2.y +
           vector_1.z * vector_2.z;
}

/**
 * @brief Returns the length of the vector
 * @param vector The given vector
 * @returns Its length squared
 */
LINLINE f32 vec3_length(vec3 vector)
{
    return lsqrt(vec3_dot(vector, vector));
}

/**
 * @brief Normalizes the provided vector
 * @param vector A pointer to the vector to be normalized.
 * @return The normalized vector
 */
LINLINE void vec3_normalize(vec3* vector)
{
    const f32 length = vec3_dot(*vector, *vector);
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
}

/**
 * @brief Returns a normalized copy of the provided vector
 * @param vector A pointer to the vector to be normalized.
 * @return A normalized copy of the provided vector.
 */
LINLINE vec3 vec3_normalized(vec3 vector)
{
    vec3_normalize(&vector);
    return vector;
}

/**
 * @brief Compares vector_1 and vector_2 and ensures the difference is less than tolerance
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @param tolerance The difference tolerance. Typically L_EPS or similar
 * @return True if within tolerance; false otherwise
 */
LINLINE b8 vec3_compare(vec3 vector_1, vec3 vector_2, f32 tolerance)
{
    if(labs(vector_1.x - vector_2.x) > tolerance) {
        return FALSE;
    }
    if(labs(vector_1.y - vector_2.y) > tolerance) {
        return FALSE;
    }
    if(labs(vector_1.z - vector_2.z) > tolerance) {
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief Returns the distance between vector_1 and vector_2 
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The distance between them
 */
LINLINE f32 vec3_distance(vec3 vector_1, vec3 vector_2)
{
    vec3 d = (vec3){
        vector_1.x - vector_2.x,
        vector_1.y - vector_2.y,
        vector_1.z - vector_2.z
    };
    return vec3_length(d);
}

/**
 * @brief Multiplies all elements of vector by a scalar
 * @param vector The vector
 * @param scalar The scalar
 * @returns A copy of the resulting vector.
 */
LINLINE vec3 vec3_mul_scalar(vec3 vector, f32 scalar)
{
    return (vec3){
        vector.x * scalar,
        vector.y * scalar,
        vector.z * scalar
    };
}

// --------------------------------------
// Vector 4
// --------------------------------------


/**
 * @brief Creates and returns a new 4-element vector using the supplied values.
 * 
 * @param x The x value.
 * @param y The y value.
 * @param z The z value.
 * 
 * @returns A Vector-4 of the provided values.
 */
    
LINLINE vec4 vec4_make(f32 x, f32 y, f32 z, f32 w)
{
    vec4 out_vector;
    
#if defined(LUSE_SIMD)
    out_vector.data = _mm_setr_ps(x, y, z, w);
#else
    out_vector.x = x;
    out_vector.y = y;
    out_vector.z = z;
    out_vector.w = w;
#endif

    return out_vector;
}

/**
 * @brief Creates and returns a new 4-element vector using the supplied value.
 * @param value The provided value.
 * 
 * @returns A Vector-4 of the provided value.
 */
LINLINE vec4 vec4_set(f32 value)
{
    vec4 out_vector;
    
#if defined(LUSE_SIMD)
    out_vector.data = _mm_setr_ps(value, value, value, value);
#else
    out_vector.x = value;
    out_vector.y = value;
    out_vector.z = value;
    out_vector.w = value;
#endif

    return out_vector;
}

/**
 * @brief Returns a new vec3 containing the x, y, and z components of the
 * supplied vec4, essentially dropping the w-component.
 * @param vector The 4-component vector to extract from.
 * @return A new vec3
 */
LINLINE vec3 vec4_to_vec3(vec4 vector)
{
    return (vec3){vector.x, vector.y, vector.z};
}

/**
 * @brief Returns a new vec4 using vector as the x, y, and z components and w for w
 * @param vector The 3-component vector.
 * @param w The w component.
 * @returns A new vec4
 */
LINLINE vec4 vec4_from_vec3(vec3 vector, f32 w)
{
#if defined (LUSE_SIMD)
    vec4 out_vector;
    out_vector.data = _mm_setr_ps(x, y, z, w);
    return out_vector;
#else
    return (vec4){vector.x, vector.y, vector.z, w};
#endif
}

/**
 * @brief Adds vector_1 and vector_2 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec4 vec4_add(vec4 vector_1, vec4 vector_2)
{
    vec4 res;
    for(u32 i = 0; i < 4; i++){
        res.elements[i] = vector_1.elements[i] + vector_2.elements[i];
    }
    return res;
}

/**
 * @brief Subtracts vector_2 from vector_1 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec4 vec4_subtract(vec4 vector_1, vec4 vector_2)
{
    vec4 res;
    for(u32 i = 0; i < 4; i++){
        res.elements[i] = vector_1.elements[i] - vector_2.elements[i];
    }
    return res;
}

/**
 * @brief Multiplies vector_1 and vector_2 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec4 vec4_mult(vec4 vector_1, vec4 vector_2)
{
    vec4 res;
    for(u32 i = 0; i < 4; i++){
        res.elements[i] = vector_1.elements[i] * vector_2.elements[i];
    }
    return res;
}

/**
 * @brief Divides vector_2 from vector_1 and returns a copy of the result
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting vector.
 */
LINLINE vec4 vec4_div(vec4 vector_1, vec4 vector_2)
{
    vec4 res;
    for(u32 i = 0; i < 4; i++){
        res.elements[i] = vector_1.elements[i] / vector_2.elements[i];
    }
    return res;
}

/**
 * @brief Returns the dot product of vector_1 and vector_2 
 * @param vector_1 The first vector.
 * @param vector_2 The second vector. 
 * @return The resulting scalar.
 */
LINLINE f32 vec4_dot(vec4 vector_1, vec4 vector_2)
{
    f32 res = 0;
    for(u32 i = 0; i < 4; i++){
        res += vector_1.elements[i] * vector_2.elements[i];
    }
    return res;
}

/**
 * @brief Returns the length of the vector
 * @param vector The given vector
 * @returns Its length squared
 */
LINLINE f32 vec4_length(vec4 vector)
{
    return lsqrt(vec4_dot(vector, vector));
}

/**
 * @brief Normalizes the provided vector
 * @param vector A pointer to the vector to be normalized.
 * @return The normalized vector
 */
LINLINE void vec4_normalize(vec4* vector)
{
    const f32 length = vec4_dot(*vector, *vector);
    for(u32 i = 0; i < 4; i++){
        vector->elements[i] /= length;
    }
}

/**
 * @brief Returns a normalized copy of the provided vector
 * @param vector A pointer to the vector to be normalized.
 * @return A normalized copy of the provided vector.
 */
LINLINE vec4 vec4_normalized(vec4 vector)
{
    vec4_normalize(&vector);
    return vector;
}

LINLINE f32 vec4_dot_32(
    f32 a0, f32 a1, f32 a2, f32 a3,
    f32 b0, f32 b1, f32 b2, f32 b3
)
{
    f32 p = 0;
    p += a0 * b0;
    p += a1 * b1;
    p += a2 * b2; 
    p += a3 * b3;

    return p;
}