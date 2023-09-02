#pragma once

#include "core/application.h"

/**
*   Represents the basic game state in a game.
*   Called for by creation by the application.
*   This is what a game looks like to the engine.
**/

typedef struct game {

    // The application configuration.
    application_config application_config;

    // Function pointer to game's initialize function.
    b8 (*initialize)(struct game* game_inst);

    // Function pointer to game's update funciton.
    b8 (*update)(struct game* game_inst, f32 delta_time);

    // Function pointer to game's render function.
    b8 (*render)(struct game* game_inst, f32 delta_time);

    // Function pointer to handle resizes, if applicable.
    void (*on_resize)(struct game* game_inst, u32 width, u32 height);

    // Game specific game state. Created and managed by the game.
    void* state;
} game;