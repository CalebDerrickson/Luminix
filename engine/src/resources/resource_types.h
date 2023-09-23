#pragma once

#include "math/math_types.h"
#define MAX_TEXTURE_NAME_LENGTH 512
#define MAX_MATERIAL_NAME_LENGTH 512
#define MAX_GEOMETRY_NAME_LENGTH 256

// Pre-defined resource types
typedef enum resource_type {
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_STATIC_MESH,
    RESOURCE_TYPE_CUSTOM
} resource_type;

typedef struct resource {
    u32 loader_id;
    const char* name;
    char* full_path;
    u64 data_size;
    void* data;
} resource;

typedef struct image_resource_data {
    u8 channel_count;
    u32 width;
    u32 height;
    u8* pixels;
} image_resource_data;

typedef struct material_config {
    char name[MAX_MATERIAL_NAME_LENGTH];
    b8 auto_release;
    vec4 diffuse_color;
    char diffuse_map_name[MAX_TEXTURE_NAME_LENGTH];
} material_config;

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

/**
 * @brief Represents actual geometry in the world. 
 * Typically (but not always, depending on use) paired with a material.
 */
typedef struct geometry {
    u32 id;
    u32 internal_id;
    u32 generation;
    char name[MAX_GEOMETRY_NAME_LENGTH];
    material* material;
} geometry;