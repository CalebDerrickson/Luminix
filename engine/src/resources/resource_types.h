#pragma once

#include "math/math_types.h"
#define MAX_TEXTURE_NAME_LENGTH 512
#define MAX_MATERIAL_NAME_LENGTH 512

typedef struct texture {
    u32 id;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    u32 generation;
    char name[MAX_TEXTURE_NAME_LENGTH];
    void* internal_data;
} texture;

typedef enum texture_use {
    TEXTURE_USE_UNKNOWN = 0x00,
    TEXTURE_USE_MAP_DIFFUSE = 0x01,
} texture_use;

typedef struct texture_map {
    texture* texture;
    texture_use use;
} texture_map;

typedef struct material {
    u32 id;
    u32 generation;
    u32 internal_id;
    char name[MAX_MATERIAL_NAME_LENGTH];
    vec4 diffuse_color;
    texture_map diffuse_map;
} material;