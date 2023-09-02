#include <core/logger.h>
#include <core/asserts.h>

// TODO: Test
#include <core/application.h>

int main(void) 
{
    // Application Struct
    application_config config;

    config.start_pos_x = 100;
    config.start_pos_y = 100;
    config.start_height = 720;
    config.start_width = 1200;
    config.name = "Luminix Engine Testbed";

    application_create(&config);
    application_run();
    
    return 0;
}