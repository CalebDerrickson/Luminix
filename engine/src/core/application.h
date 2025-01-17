#pragma once 

#include "defines.h"

struct game;

// Application Struct
typedef struct application_config {

    // Window starting x position, if applicable.
    i16 start_pos_x;

    // Window starting y position, if applicable.
    i16 start_pos_y;

    // Window starting height, if applicable.
    i16 start_height;

    // Window starting width, if applicable;
    i16 start_width;

    // The Application name used in windowing, if applicable.
    char* name;

} application_config;


LAPI b8 application_create(struct game* game_inst);

LAPI b8 application_run();

void application_get_framebuffer_size(u32* width, u32* height);