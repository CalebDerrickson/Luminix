#include "renderer/renderer_frontend.h"
#include "renderer/renderer_backend.h"

#include "core/logger.h"
#include "core/lmemory.h"
#include "math/lmath.h"

#include "resources/resource_types.h"

#include "systems/texture_system.h"
#include "systems/material_system.h"

// TODO: temporary
#include "core/lstring.h"
#include "core/event.h"
// TODO: end temporary

typedef struct renderer_system_state {
    renderer_backend backend;
    mat4 world_projection;
    mat4 world_view;
    mat4 ui_projection;
    mat4 ui_view;
    f32 near_clip;
    f32 far_clip; 
} renderer_system_state;

static renderer_system_state* state_ptr;

b8 renderer_system_initialize(u64* memory_requirement, void* state, const char* application_name) 
{
    *memory_requirement = sizeof(renderer_system_state);    
    if (state == 0) {
        return false;
    }
    state_ptr = state;

    // TODO: Make this configurable
    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &state_ptr->backend);
    state_ptr->backend.frame_number = 0;

    if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
        LFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }
    
    // World projection/view
    state_ptr->near_clip = 0.1f;
    state_ptr->far_clip = 1000.0f;
    state_ptr->world_projection = mat4_perspective(deg_to_rad(45.0f), 1280/720.0f, state_ptr->near_clip, state_ptr->far_clip);
    // TODO: configurable camera starting position
    state_ptr->world_view = mat4_translation((vec3){0, 0, -30.0f});
    state_ptr->world_view = mat4_inverse(state_ptr->world_view);

    // UI projection/view
    state_ptr->ui_projection = mat4_orthographic(0, 1280.0f, 720.0f, 0, -100.0f, 100.0f);   // Intentionally flipped on y-axis
    state_ptr->ui_view = mat4_inverse(mat4_identity());

    return true;
}

void renderer_system_shutdown(void* state)
{
    if (!state_ptr) {
        return;
    }

    state_ptr->backend.shutdown(&state_ptr->backend);
    state_ptr = 0;
}


void renderer_on_resized(u16 width, u16 height)
{
    if (state_ptr) { 
        state_ptr->world_projection = mat4_perspective(deg_to_rad(45.0f), width / (f32) height, state_ptr->near_clip, state_ptr->far_clip); 
        state_ptr->ui_projection = mat4_orthographic(0, (f32)width, (f32)height, 0, -100.0f, 100.0f);
        state_ptr->backend.resized(&state_ptr->backend, width, height);
    } else {
        LWARN("renderer backend does not accept resize: %i, %i", width, height);
    }
}

b8 renderer_draw_frame(render_packet* packet)
{
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (!state_ptr->backend.begin_frame(&state_ptr->backend, packet->delta_time)) {
        return true;
    }
    
    // World renderpass 
    if (!state_ptr->backend.begin_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_WORLD)) {
        LERROR("backend.begin_renderpass -> BUILTIN_RENDERPASS_WORLD failed. Application shutting down...");
        return false;
    }

    state_ptr->backend.update_global_world_state(state_ptr->world_projection, state_ptr->world_view, vec3_set(0), vec4_set(1.0f), 0);
    
    // Draw geometries
    u32 count = packet->geometry_count;
    for (u32 i = 0; i < count; i++) {
        state_ptr->backend.draw_geometry(packet->geometries[i]);
    }

    // End world renderpass
    if (!state_ptr->backend.end_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_WORLD)) {
        LERROR("backend.end_renderpass -> BUILTIN_RENDERPASS_WORLD failed. Application shutting down...");
        return false;
    }

    // UI renderpass
    if (!state_ptr->backend.begin_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_UI)) {
        LERROR("backend.begin_renderpass -> BUILTIN_RENDERPASS_UI failed. Application shutting down...");
        return false;
    }

    // Update UI global state
    state_ptr->backend.update_global_ui_state(state_ptr->ui_projection, state_ptr->ui_view, 0);

    // Draw geometries
    count = packet->ui_geometry_count;
    for (u32 i = 0; i < count; i++) {
        state_ptr->backend.draw_geometry(packet->ui_geometries[i]);
    }

    // End world renderpass
    if (!state_ptr->backend.end_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_UI)) {
        LERROR("backend.end_renderpass -> BUILTIN_RENDERPASS_UI failed. Application shutting down...");
        return false;
    }


    // End the frame. If this fails, likely unrecoverable.
    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, packet->delta_time);
    state_ptr->backend.frame_number++;

    if (!result){
        LERROR("renderer_end_frame failes. Application shutting down...");
        return false;
    }

    return true;
}

void renderer_set_view(mat4 view)
{
    state_ptr->world_view = view;
}

void renderer_create_texture(const u8* pixels, struct texture* texture)
{
    state_ptr->backend.create_texture(pixels, texture);
}

void renderer_destroy_texture(struct texture* texture)
{
    state_ptr->backend.destroy_texture(texture);
}

b8 renderer_create_material(struct material* material)
{
   return state_ptr->backend.create_material(material);
}

void renderer_destroy_material(struct material* material)
{
    state_ptr->backend.destroy_material(material);
}

b8 renderer_create_geometry(geometry* geometry, u32 vertex_count, const vertex_3d* vertices, u32 index_count, const u32* indices)
{   
    return state_ptr->backend.create_geometry(geometry, vertex_count, vertices, index_count, indices);
}

void renderer_destroy_geometry(geometry* geometry)
{
    state_ptr->backend.destroy_geometry(geometry);
}
