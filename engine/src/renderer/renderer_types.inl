#pragma once
#include "defines.h"

#include "math/math_types.h"

#include "resources/resource_types.h"

typedef enum renderer_backend_type {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

typedef struct global_uniform_object {
    // For the purposes of aligning to NVIDIA cards, uniform objects need to be 256 bytes.
    mat4 projection;        // 64 bytes
    mat4 view;              // 64 bytes
    mat4 m_reserved0;       // 64 bytes
    mat4 m_reserved1;       // 64 bytes
} global_uniform_object;

typedef struct material_uniform_object {
    vec4 diffuse_color;     // 16 bytes
    vec4 v_reserved0;       // 16 bytes
    vec4 v_reserved1;       // 16 bytes
    vec4 v_reserved2;       // 16 bytes
} material_uniform_object;

typedef struct geometry_render_data {
    mat4 model;
    material* material;
} geometry_render_data;

typedef struct renderer_backend {
    struct platform_state* plat_state;
    u64 frame_number;

    b8 (*initialize)(struct renderer_backend* backend, const char* application_name);

    void (*shutdown)(struct renderer_backend* backend);

    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, i32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);
    
    void (*update_object)(geometry_render_data data);

    void (*create_texture)(const u8* pixels, struct texture* out_texture);
    
    void (*destroy_texture)(struct texture* texture);

    b8 (*create_material)(struct material* material);

    void (*destroy_material)(struct material* material);

} renderer_backend;

typedef struct render_packet{
    f32 delta_time;
} render_packet;