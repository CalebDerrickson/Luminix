#include "game.h"
#include <core/logger.h>
#include <core/input.h>
#include <core/lmemory.h>

#include <math/lmath.h>

// HACK: This should not be available outside the engine!
#include <renderer/renderer_frontend.h>

void recalculate_view_matrix(game_state* state);
void camera_yaw(game_state* state, f32 amount);
void camera_pitch(game_state* state, f32 amount);
void camera_roll(game_state* state, f32 amount);

b8 game_initialize(game* game_inst)
{
    LDEBUG("Game initialize was called.");
 
    game_state* state = (game_state*)game_inst->state;

    state->camera_position = (vec3){0, 0, 30.0f}; 
    state->camera_euler = vec3_set(0); 

    state->view = mat4_translation(state->camera_position);
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = true;

    return true;
}

b8 game_update(game* game_inst, f32 delta_time)
{

    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();

    // Here just to make the code look better
    game_state* state = (game_state*)game_inst->state;

    if (input_is_key_up('M') && input_was_key_down('M')) {
        LDEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }



    // HACK: temp key to move around
    if (input_is_key_down('A') || input_is_key_down(KEY_LEFT)) {
        camera_yaw(state, -1.0f * delta_time);
    }
    
    if (input_is_key_down('D') || input_is_key_down(KEY_RIGHT)) {
        camera_yaw(state, 1.0f * delta_time);
    }
    
    if (input_is_key_down(KEY_UP)) {
        camera_pitch(state, -1.0f * delta_time);
    }
    
    if (input_is_key_down(KEY_DOWN)) {
        camera_pitch(state, 1.0f * delta_time);
    }

    f32 temp_move_speed = 15.0f;
    vec3 velocity = vec3_set(0);
    
    if (input_is_key_down('W')) {
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }
    
    if (input_is_key_down('S')) {
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }

    if (input_is_key_down('Q')) {
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }
    
    if (input_is_key_down('E')) {
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }

    if (input_is_key_down(KEY_SPACE)) {
        velocity.y += 1.0f;
    }

    if (input_is_key_down('X')) {
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_set(0);
    if (!vec3_compare(z, velocity, 0.0002f)) {
        // Be sure to normalize the velocity before applying speed.
        vec3_normalize(&velocity);
        state->camera_position.x += velocity.x * temp_move_speed * delta_time;
        state->camera_position.y += velocity.y * temp_move_speed * delta_time;
        state->camera_position.z += velocity.z * temp_move_speed * delta_time;
        state->camera_view_dirty = true;
    }

    recalculate_view_matrix(state);

    // HACK: This should not be available outside the engine!
    renderer_set_view(state->view);

    return true;
}

b8 game_render(game* game_inst, f32 delta_time)
{

    return true;
}

void game_on_resize(game* game_inst, u32 width, u32 height)
{


}

void recalculate_view_matrix(game_state* state)
{
    if (!state->camera_view_dirty) {
        return;
    }

    mat4 rotation = mat4_euler_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
    mat4 translation = mat4_translation(state->camera_position);
    state->view = mat4_mul(rotation, translation);
    state->view = mat4_inverse(state->view);

    state->camera_view_dirty = false;
}

void camera_pitch(game_state* state, f32 amount)
{
    state->camera_euler.x += amount;

    // Clamp to aviod Gimball lock.
    f32 limit = deg_to_rad(89.0f);
    state->camera_euler.x = LCLAMP(state->camera_euler.x, -limit, limit);

    state->camera_view_dirty = true;
}

void camera_yaw(game_state* state, f32 amount)
{
    state->camera_euler.y += amount;
    state->camera_view_dirty = true;
}

void camera_roll(game_state* state, f32 amount)
{
    state->camera_euler.z += amount;
    state->camera_view_dirty = true;
}
