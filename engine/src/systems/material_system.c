#include "material_system.h"

#include "core/logger.h"
#include "core/lstring.h"
#include "containers/hashtable.h"
#include "math/lmath.h"
#include "renderer/renderer_frontend.h"
#include "systems/texture_system.h"

// TODO: temp: resource system
#include "platform/filesystem.h"
// TODO: end temp

typedef struct material_system_state {
    material_system_config config;

    material default_material;

    // Array of registered materials.
    material* registered_materials;

    // Hashtable for material lookups.
    hashtable registered_material_table;
} material_system_state;

typedef struct material_reference {
    u64 reference_count;
    u32 handle;
    b8 auto_release;
} material_reference;

static material_system_state* state_ptr = 0;

//--------------------------------
// PRIVATE METHOD DECLARATIONS
//-------------------------------- 

b8 create_default_material(material_system_state* state);
b8 load_material(material_config config, material* m);
void destroy_material(material* m);
b8 load_configuration_file(const char* path, material_config* out_config);

//--------------------------------
// PUBLIC METHOD DEFINITONS
//-------------------------------- 

b8 material_system_initialize(u64* memory_requirement, void* state, material_system_config config)
{
    if (config.max_material_count == 0) {
        LFATAL("material_system_initialize - config.mat_material_count must be > 0.");
        return false;
    }

    // Block of memory will contain state structure, then block for array, then block for hashtable.
    u64 struct_requirement = sizeof(material_system_state);
    u64 array_requirement = sizeof(material) * config.max_material_count;
    u64 hashtable_requirement = sizeof(material_reference) * config.max_material_count;
    *memory_requirement = struct_requirement + array_requirement + hashtable_requirement;

    if (!state) {
        return true;
    }

    state_ptr = state;
    state_ptr->config = config;

    // The array block is after the state. Already allocated, so just set the pointer.
    void* array_block = state + struct_requirement;
    state_ptr->registered_materials = array_block;

    // Hashtable block is after array.
    void* hashtable_block = array_block + array_requirement;

    // Create a hashtable for the required material lookups.
    hashtable_create(
        sizeof(material_reference),
        config.max_material_count,
        hashtable_block,
        false,
        &state_ptr->registered_material_table
    );

    // Fill the hashtable with invalid references to use as a default.
    material_reference invalid_ref;
    invalid_ref.auto_release = false;
    invalid_ref.handle = INVALID_ID;  // Primary reason for needing default values.
    invalid_ref.reference_count = 0;
    hashtable_fill(&state_ptr->registered_material_table, &invalid_ref);

    // Invalidate all materials in the array.
    u32 count = state_ptr->config.max_material_count;
    for (u32 i = 0; i < count; ++i) {
        state_ptr->registered_materials[i].id = INVALID_ID;
        state_ptr->registered_materials[i].generation = INVALID_ID;
        state_ptr->registered_materials[i].internal_id = INVALID_ID;
    }

    if (!create_default_material(state_ptr)) {
        LFATAL("Failed to create default material. Application cannot continue.");
        return false;
    }

    return true;
}

void material_system_shutdown(void* state)
{
    material_system_state* s = (material_system_state*)state;

    if (!s) {
        state_ptr = 0;
        return;
    }

    // Invalidate all materials in the array.
    u32 count = s->config.max_material_count;
    for (u32 i = 0; i < count; i++) {
        if (s->registered_materials[i].id != INVALID_ID) {
            destroy_material(&s->registered_materials[i]);
        }
    }

    // Destroy the default material.
    destroy_material(&s->default_material);
    state_ptr = 0;
}

material* material_system_acquire(const char* name)
{
    // Load the given material configuration from disk.
    material_config config;

    // Load file from disk.
    // TODO: Should be able to be located anywhere.
    char* format_string = "assets/materials/%s.%s";
    char full_file_path[512];

    // TODO: Try different extensions.
    string_format(full_file_path, format_string, name, "lmt");
    if (!load_configuration_file(full_file_path, &config)) {
        LERROR("Failed to load material file '%s'. Null pointer will be returned.", full_file_path);
        return 0;
    }

    // Now acquire from loaded config.
    return material_system_acquire_from_config(config);
}

material* material_system_acquire_from_config(material_config config)
{
    // Return default material
    if (strings_equali(config.name, DEFAULT_MATERIAL_NAME)) {
        return &state_ptr->default_material;
    }

    material_reference ref;
    if (!state_ptr || !hashtable_get(&state_ptr->registered_material_table, config.name, &ref)) {
        // NOTE: This should only really happen when something went wrong with the state.
        LERROR("material_system_acquire_from_config failed to acquire material '%s'. Null pointer will be returned.", config.name);
        return 0;
    }

    // This can only be changed the first time a material is loaded.
    if (ref.reference_count == 0) {
        ref.auto_release = config.auto_release;
    }

    ref.reference_count++;
    if (ref.handle != INVALID_ID) {
        LTRACE("Material '%s' already exists, ref count is increased to %i.", config.name, ref.reference_count);

        // Update the entry
        hashtable_set(&state_ptr->registered_material_table, config.name, &ref);
        return &state_ptr->registered_materials[ref.handle];
    }

    // This means no material exists here. find a free index first.
    u32 count = state_ptr->config.max_material_count;
    material* m = 0;
    for (u32 i = 0; i < count; i++) {
        if (state_ptr->registered_materials[i].id == INVALID_ID) {
            // Free spot found.
            ref.handle = i;
            m = &state_ptr->registered_materials[i];
            break;
        }
    }

    // Make sure an empty slot was found.
    if (!m || ref.handle == INVALID_ID) {
        LFATAL("material_system_acquire_from_config - material system cannot hold any more materials. Adjust configuration to allow more.");
        return 0;
    }

    // Create new material
    if (!load_material(config, m)) {
        LERROR("Failed to load material '%s'.", config.name);
        return 0;
    }

    if (m->generation == INVALID_ID) {
        m->generation = 0;
    } else {
        m->generation++;
    }

    // Also use the handle as material id.
    m->id = ref.handle;
    LTRACE("Material '%s' does not yet exist. Created, and ref_count is not %i.", config.name, ref.reference_count);

    // Update the entry
    hashtable_set(&state_ptr->registered_material_table, config.name, &ref);
    return &state_ptr->registered_materials[ref.handle];
}

void material_system_release(const char* name)
{
    // Ignore requests to release the default material.
    if (strings_equali(name, DEFAULT_MATERIAL_NAME)) {
        return;
    }

    material_reference ref;
    if (!state_ptr || hashtable_get(&state_ptr->registered_material_table, name, &ref)) {
        LERROR("material_system_release failed to release material '%s'.", name);
        return;
    }

    if (ref.reference_count == 0) {
        LWARN("Tried to release non-existent material: '%s'.", name);
        return;
    }
    ref.reference_count--;

    if (ref.reference_count || !ref.auto_release) {
        LTRACE("Released material '%s', now has a reference count of '%i' (auto_release = &s).", 
            name, 
            ref.reference_count,
            ref.auto_release ? "true" : "false" 
        );

        // Update the entry.
        hashtable_set(&state_ptr->registered_material_table, name, &ref);
    }

    // Now material has a zero reference count and is set to auto release
    material* m = &state_ptr->registered_materials[ref.handle];

    // Destroy and reset the material.
    destroy_material(m);

    // Reset the reference.
    ref.handle = INVALID_ID;
    ref.auto_release = false;
    LTRACE("Released material '%s'. Material unloaded because reference count = zero and auto_release = true", name);

    // Update the entry.
    hashtable_set(&state_ptr->registered_material_table, name, &ref);
  
}

//--------------------------------
// PRIVATE METHOD DEFINITIONS
//--------------------------------

b8 create_default_material(material_system_state* state)
{
    lzero_memory(&state->default_material, sizeof(material));
    state->default_material.id = INVALID_ID;
    state->default_material.generation = INVALID_ID;
    string_ncopy(state->default_material.name, DEFAULT_MATERIAL_NAME, MAX_MATERIAL_NAME_LENGTH);
    state->default_material.diffuse_color = vec4_set(1.0f); // White
    state->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    state->default_material.diffuse_map.texture = texture_system_get_default_texture();

    if (!renderer_create_material(&state->default_material)) {
        LFATAL("Failed to acquire renderer resources for default texture. Application cannot continue.");
        return false;
    }

    return true;
}

b8 load_material(material_config config, material* m)
{
    lzero_memory(m, sizeof(material));

    // name
    string_ncopy(m->name, config.name, MAX_MATERIAL_NAME_LENGTH);

    // diffuse color
    m->diffuse_color = config.diffuse_color;

    // diffuse map
    if (string_length(config.diffuse_map_name) > 0) {
        m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
        m->diffuse_map.texture = texture_system_acquire(config.diffuse_map_name, true);
        if (!m->diffuse_map.texture) {
            LWARN("Unable to load texture '%s' for material '%s', using default.", config.diffuse_map_name, m->name);
            m->diffuse_map.texture = texture_system_get_default_texture();
        }
    } else {
        // NOTE: Only set for clarity, as call to lzero memory above does this already.
        m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
        m->diffuse_map.texture = 0;
    }

    // TODO: other maps

    // Send it off to the renderer to acquire resources.
    if (!renderer_create_material(m)) {
        LERROR("Failed to acquire renderer resources for material '%s'.", m->name);
        return false;
    }

    return true;
}

void destroy_material(material* m)
{
    LTRACE("Destroying material '%s'...", m->name);

    // Release texture references.
    if (m->diffuse_map.texture) {
        texture_system_release(m->diffuse_map.texture->name);
    }

    // Release renderer resources.
    renderer_destroy_material(m);

    // zero it out, invalidate ids.
    lzero_memory(m, sizeof(material));
    m->id = INVALID_ID;
    m->generation = INVALID_ID;
    m->internal_id = INVALID_ID;
}

b8 load_configuration_file(const char* path, material_config* out_config)
{
    file_handle f;
    if (!filesystem_open(path, FILE_MODE_READ, false, &f)) {
        LERROR("load_configuration_file - unable to open material for for reading: '%s'.", path);
        return false;
    }

    // Read each line of the file
    char line_buf[512] = "";
    char* p = &line_buf[0];
    u64 line_length = 0;
    u32 line_number = 1;
    while(filesystem_read_line(&f, 511, &p, &line_length)) {
        // Trim the string
        char* trimmed = string_trim(line_buf);

        // Get the trimmed length
        line_length = string_length(trimmed);

        // Skip blank lines and comments.
        if (line_length < 1 || trimmed[0] == '#') {
            line_number++;
            continue;
        } 

        // split into var/value
        i32 equal_index = string_index_of(trimmed, '=');
        if (equal_index == -1) {
            LWARN("Potential formatting issue found in file '%s': token not found. Skipping line %ui.", path, line_number);
            line_number++;
            continue;
        }

        // Assume a max of 64 characters for the variable name.
        char raw_var_name[64];
        lzero_memory(raw_var_name, sizeof(char) * 64);
        string_mid(raw_var_name, trimmed, 0, equal_index);
        char* trimmed_var_name = string_trim(raw_var_name);

        // Assume a max of 511-65 (446) for the max length of the value to account for the variable name and the '='.
        char raw_value[446];
        lzero_memory(raw_value, sizeof(char) * 446);
        string_mid(raw_value, trimmed, equal_index + 1, -1);    // Read the rest of the line
        char* trimmed_value = string_trim(raw_value);

        // Process the variable
        if (strings_equali(trimmed_var_name, "version")) {
            // TODO: Version
        } 
        else if (strings_equali(trimmed_var_name, "name")) {
            string_ncopy(out_config->name, trimmed_value, MAX_MATERIAL_NAME_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_map_name")) {
            string_ncopy(out_config->diffuse_map_name, trimmed_value, MAX_TEXTURE_NAME_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_color")) {
            // Parse the color
            if (!string_to_vec4(trimmed_value, &out_config->diffuse_color)) {
                LWARN("Error parsing diffuse_color in file '%s'. Using default of white instead.", path);
                out_config->diffuse_color = vec4_set(1.0f); // White
            }
        }
        // TODO: more fields

        // Clear the line buffer.
        lzero_memory(line_buf, sizeof(char) * 512);
        line_number++;
    }

    filesystem_close(&f);

    return true;
}

material* material_system_get_default()
{
    if (!state_ptr) {
        LFATAL("material_system_get_default called before system is initialized!");
        return 0;
    }

    return &state_ptr->default_material;
}
