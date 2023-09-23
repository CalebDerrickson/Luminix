#include "resource_system.h"

#include "core/logger.h"
#include "core/lstring.h"

// Known Resource Loaders
#include "resources/loaders/image_loader.h"


typedef struct resource_system_state {
    resource_system_config config;
    resource_loader* registered_loaders;
} resource_system_state;

static resource_system_state* state_ptr = 0;

//--------------------------------
// PRIVATE METHOD DECLARATIONS
//-------------------------------- 

b8 load(const char* name, resource_loader* loader, resource* out_resource);

//--------------------------------
// PUBLIC METHOD DEFINITONS
//-------------------------------- 

b8 resource_system_initialize(u64* memory_requirement, void* state, resource_system_config config)
{
    if (config.max_loader_count == 0) {
        LFATAL("resource_system_initialize - config.max_loader_count must be > 0");
        return false;
    }

    // Block of memory will contain state structure, then block for array.
    u64 struct_requirement = sizeof(resource_system_state);
    u64 array_requirement = sizeof(resource) * config.max_loader_count;
    *memory_requirement = struct_requirement + array_requirement;

    if (!state) {
        return true;
    }

    state_ptr = state;
    state_ptr->config = config;

    // The array block is after the state. Already allocated, so just set the pointer.
    void* array_block = state + struct_requirement;
    state_ptr->registered_loaders = array_block;

    // Invalidate all loaders in the array.
    u32 count = state_ptr->config.max_loader_count;
    for (u32 i = 0; i < count; i++) {
        state_ptr->registered_loaders[i].id = INVALID_ID;
    }

    // NOTE: Auto-register known loader types here.
    resource_system_register_loader(image_resource_loader_create());

    LINFO("resource system initialized with base path '%s'", config.asset_base_path);

    return true;
}

void resource_system_shutdown(void* state) 
{
    if(!state_ptr) {
        return;
    }

    state_ptr = 0;
}

b8 resource_system_register_loader(resource_loader loader) 
{
    if(!state_ptr) {
        return false;
    }

    u32 count = state_ptr->config.max_loader_count;

    // Ensure no loaders for the given type already exist
    for(u32 i = 0; i < count; i++) {
        resource_loader* l = &state_ptr->registered_loaders[i];
        if(l->id == INVALID_ID) {
            continue;
        } 
        if(l->type == loader.type) {
            LERROR("resource_system_register_loader - Loader of type %d already exists and will not be registered.", loader.type);
            return false;
        }
        else if(loader.custom_type && string_length(loader.custom_type) > 0 && strings_equali(l->custom_type, loader.custom_type)) {
            LERROR("resource_system_register_loader - Loader of type %s already exists and will not be registered.", loader.custom_type);
            return false;
        }
    }

    // Register loader
    for(u32 i = 0; i < count; i++) {
        if(state_ptr->registered_loaders[i].id != INVALID_ID) {
            continue;
        }
        state_ptr->registered_loaders[i] = loader;
        state_ptr->registered_loaders[i].id = i;
        LTRACE("Loader registered.");
        return true;
    }

    LERROR("Unable to obtain free slot for loader. Adjust resource configuration to allow more space.");
    return false;
}

b8 resource_system_load(const char* name, resource_type type, resource* out_resource)
{
    if(!state_ptr || type == RESOURCE_TYPE_CUSTOM) {
        out_resource->loader_id = INVALID_ID;
        LERROR("resource_system_load - No loader for type %d was found.", type);
        return false;
    }

    // Select loader
    u32 count = state_ptr->config.max_loader_count;
    for(u32 i = 0; i < count; i++) {
        resource_loader* l = &state_ptr->registered_loaders[i];
        if(l->id != INVALID_ID && l->type == type) {
            return load(name, l, out_resource);
        }
    }

    LERROR("resource_system_load - could not find loader for type %d.", type);
    return false;
}

b8 resource_system_load_custom(const char* name, const char* custom_type, resource* out_resource)
{
    if(!state_ptr || !custom_type || string_length(custom_type) <= 0) {
        out_resource->loader_id = INVALID_ID;
        LERROR("resource_system_load_custom - No loader for custom_type %d was found.", custom_type);
        return false;
    }

    // Select loader
    u32 count = state_ptr->config.max_loader_count;
    for(u32 i = 0; i < count; i++) {
        resource_loader* l = &state_ptr->registered_loaders[i];
        if(l->id != INVALID_ID && l->type == RESOURCE_TYPE_CUSTOM && strings_equali(l->custom_type, custom_type)) {
            return load(name, l, out_resource);
        }
    }

    LERROR("resource_system_load_custom - could not find loader for custom_type %d.", custom_type);
    return false;
}

void resource_system_unload(resource* resource)
{
    if(!state_ptr || !resource || resource->loader_id == INVALID_ID) {
        return;
    }

    resource_loader* l = &state_ptr->registered_loaders[resource->loader_id];
    if(l->id != INVALID_ID && l->unload) {
        l->unload(l, resource);
    }
}

const char* resource_system_base_path()
{
    if(!state_ptr) {
        LERROR("resource_system_base_path called before initialization, returning empty string.");
        return "";
    }

    return state_ptr->config.asset_base_path;
}

//--------------------------------
// PRIVATE METHOD DEFINITONS
//-------------------------------- 

b8 load(const char* name, resource_loader* loader, resource* out_resource)
{
    if(!name || !loader || !loader->load || !out_resource) {
        out_resource->loader_id = INVALID_ID;
        return false;
    }

    out_resource->loader_id = loader->id;
    return loader->load(loader, name, out_resource);
}