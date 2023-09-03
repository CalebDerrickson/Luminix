#include "game.h"
#include <entry.h>
#include <core/lmemory.h>

// Define the function to create the game
b8 create_game(game* out_game) 
{

    // Assign Window params
    out_game->application_config.start_pos_x = 100;
    out_game->application_config.start_pos_y = 100;
    out_game->application_config.start_height = 720;
    out_game->application_config.start_width = 1200;
    out_game->application_config.name = "Luminix Engine Testbed";

    // Assign function pointers
    out_game->initialize = game_initialize;
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->on_resize = game_on_resize;

    // Create the game state.
    out_game->state = lallocate(sizeof(game_state), MEMORY_TAG_GAME);
    return TRUE;

}