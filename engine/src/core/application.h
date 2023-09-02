#pragma once 

#include "defines.h"

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


KAPI b8 application_create(application_config* config);

KAPI b8 application_run();