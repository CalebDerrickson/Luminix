#pragma once

#include "defines.h"

#include "core/asserts.h"
#include "renderer/renderer_types.inl"
#include "containers/freelist.h"

#include <vulkan/vulkan.h>


#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 1
#define MATERIAL_SHADER_STAGE_COUNT 2

#define UI_SHADER_STAGE_COUNT 2
#define VULKAN_UI_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_UI_SHADER_SAMPLER_COUNT 1

// Max number of ui control instances
// TODO: make configurable
#define VULKAN_MAX_UI_COUNT 1024

// Max number of material instances
// TODO: Make this configurable
#define VULKAN_MAX_MATERIAL_COUNT 1024

// Max number of simultaneously uploaded geometries
// TODO: Make this configurable
#define VULKAN_MAX_GEOMETRY_COUNT 4096

// Checks the given expresion's return type value agains VK_SUCCESS
#define VK_CHECK(expr)              \
    {                               \
        LASSERT(expr == VK_SUCCESS);\
    }                               \

/**
 * @brief Internal buffer data for geometry. This data gets loaded directly into a buffer.
 */
typedef struct vulkan_geometry_data {
    /** @brief The identification of the geometry. */
    u32 id;
    /** @brief The geometry generation. Incremented every the game updates. */
    u32 generation;
    /** @brief The vertex count.*/
    u32 vertex_count;
    /** @brief The size of each vertex.*/
    u32 vertex_element_size;
    /** @brief The offset in bytes of the vertex buffer.*/
    u64 vertex_buffer_offset;
    /** @brief The index count.*/
    u32 index_count;
    /** @brief The size of each index.*/
    u32 index_element_size;
    /** @brief The offset in bytes in the index buffer.*/
    u64 index_buffer_offset;
} vulkan_geometry_data;

typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vulkan_swapchain_support_info;

typedef struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
} vulkan_pipeline;

/** 
 * @brief Represents a Vulkan-specific buffer.
 * Used to load data onto the GPU. 
 * 
 * */
typedef struct vulkan_buffer {
    /** @brief The total size of the buffer.*/
    u64 total_size;
    /** @brief The handle to the internal buffer.*/
    VkBuffer handle;
    /** @brief The usage flags.*/
    VkBufferUsageFlagBits usage;
    /** @brief Indicates if the buffer's memory is currently locked.*/
    b8 is_locked;
    /** @brief The memory used by the buffer.*/
    VkDeviceMemory memory;
    /** @brief The index of the memory used by the buffer.*/
    i32 memory_index;
    /** @brief The property flags for the memory used by the buffer.*/
    u32 memory_property_flags;
    /** @brief The amount of memory required for the freelist.*/
    u64 freelist_memory_requirement;
    /** @brief The memory block used by the internal freelist.*/
    void* freelist_block;
    /** @brief A freelist used to track allocations.*/
    freelist buffer_freelist;
}vulkan_buffer;


typedef struct vulkan_device {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

    VkCommandPool graphics_command_pool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depth_format;

    b8 supports_device_local_host_visible;
}   vulkan_device;

typedef struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

typedef enum vulkan_render_pass_state {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkan_render_pass_state;

typedef struct vulkan_renderpass {
    VkRenderPass handle;
    vec4 render_area;
    vec4 clear_color;

    f32 depth;
    u32 stencil;
    vulkan_render_pass_state state;

    u8 clear_flags;
    b8 has_prev_pass;
    b8 has_next_pass;

} vulkan_renderpass;

typedef struct vulkan_swapchain {
    VkSurfaceFormatKHR image_format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 image_count;
    VkImage* images;
    VkImageView* views;

    vulkan_image depth_attachment;

    // Framebuffers used for on-screen rendering, one per frame
    VkFramebuffer framebuffers[3];

} vulkan_swapchain;

typedef enum vulkan_command_buffer_state {
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;
    vulkan_command_buffer_state state;

} vulkan_command_buffer;


typedef struct vulkan_descriptor_state {
    // One per frame
    u32 generations[3];
    u32 ids[3];
} vulkan_descriptor_state;

typedef struct vulkan_material_shader_instance_state {
    // Per frame
    VkDescriptorSet descriptor_sets[3];

    // Per descriptor
    vulkan_descriptor_state descriptor_states[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
} vulkan_material_shader_instance_state;

typedef struct vulkan_material_shader_global_ubo {
    // For the purposes of aligning to NVIDIA cards, uniform objects need to be 256 bytes.
    mat4 projection;        // 64 bytes
    mat4 view;              // 64 bytes
    mat4 m_reserved0;       // 64 bytes
    mat4 m_reserved1;       // 64 bytes
} vulkan_material_shader_global_ubo;

typedef struct vulkan_material_shader_instance_ubo {
    vec4 diffuse_color;     // 16 bytes
    vec4 v_reserved0;       // 16 bytes
    vec4 v_reserved1;       // 16 bytes
    vec4 v_reserved2;       // 16 bytes
    mat4 m_reserved0;       // 64 bytes, reserved for future use
    mat4 m_reserved1;       // 64 bytes, reserved for future use
    mat4 m_reserved2;       // 64 bytes, reserved for future use
} vulkan_material_shader_instance_ubo;

typedef struct vulkan_material_shader {
    // vertex, fragment
    vulkan_shader_stage stages[MATERIAL_SHADER_STAGE_COUNT];

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;
    
    // One descriptor set per frame - max 3 for triple buffering.
    VkDescriptorSet global_descriptor_sets[3];
    b8 descriptor_updated[3];

    // Global uniform object.
    vulkan_material_shader_global_ubo global_ubo;

    // Global uniform buffer.
    vulkan_buffer global_uniform_buffer;

    VkDescriptorPool object_descriptor_pool;
    VkDescriptorSetLayout object_descriptor_set_layout;

    // Object uniform buffers.
    vulkan_buffer object_uniform_buffer;

    // TODO: Manage a free list of some kind here instead.
    u32 object_uniform_buffer_index;

    texture_use sampler_uses[VULKAN_MATERIAL_SHADER_SAMPLER_COUNT];

    // TODO: Make dynamic
    vulkan_material_shader_instance_state instance_states[VULKAN_MAX_MATERIAL_COUNT];

    vulkan_pipeline pipeline;

} vulkan_material_shader;

typedef struct vulkan_ui_shader_instance_state {
    // Per frame
    VkDescriptorSet descriptor_sets[3];

    // Per descriptor
    vulkan_descriptor_state descriptor_states[VULKAN_UI_SHADER_DESCRIPTOR_COUNT];
} vulkan_ui_shader_instance_state;

/**
 * @brief Vulkan-specific uniform buffer object for the ui shader. 
 */
typedef struct vulkan_ui_shader_global_ubo {
    mat4 projection;   // 64 bytes
    mat4 view;         // 64 bytes
    mat4 m_reserved0;  // 64 bytes, reserved for future use
    mat4 m_reserved1;  // 64 bytes, reserved for future use
} vulkan_ui_shader_global_ubo;

/**
 * @brief Vulkan-specific ui material instance uniform buffer object for the ui shader. 
 */
typedef struct vulkan_ui_shader_instance_ubo {
    vec4 diffuse_color;  // 16 bytes
    vec4 v_reserved0;    // 16 bytes, reserved for future use
    vec4 v_reserved1;    // 16 bytes, reserved for future use
    vec4 v_reserved2;    // 16 bytes, reserved for future use
    mat4 m_reserved0;    // 64 bytes, reserved for future use
    mat4 m_reserved1;    // 64 bytes, reserved for future use
    mat4 m_reserved2;    // 64 bytes, reserved for future use
} vulkan_ui_shader_instance_ubo;

typedef struct vulkan_ui_shader {
    // vertex, fragment
    vulkan_shader_stage stages[UI_SHADER_STAGE_COUNT];

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;

    // One descriptor set per frame - max 3 for triple-buffering.
    VkDescriptorSet global_descriptor_sets[3];

    // Global uniform object.
    vulkan_ui_shader_global_ubo global_ubo;

    // Global uniform buffer.
    vulkan_buffer global_uniform_buffer;

    VkDescriptorPool object_descriptor_pool;
    VkDescriptorSetLayout object_descriptor_set_layout;
    // Object uniform buffers.
    vulkan_buffer object_uniform_buffer;
    // TODO: manage a free list of some kind here instead.
    u32 object_uniform_buffer_index;

    texture_use sampler_uses[VULKAN_UI_SHADER_SAMPLER_COUNT];

    // TODO: make dynamic
    vulkan_ui_shader_instance_state instance_states[VULKAN_MAX_UI_COUNT];

    vulkan_pipeline pipeline;

} vulkan_ui_shader;

typedef struct vulkan_context {

    f32 frame_delta_time;

    // The framebuffer's current width
    u32 framebuffer_width;

    // The framebuffer's current height
    u32 framebuffer_height;

    // Current generation of framebuffer size. It is does not match framebuffer_size_last_generation,
    // a new one should be generated.
    u64 framebuffer_size_generation;

    // The generation of the framebuffer when it was last generated. Set to framebuffer_size_generation
    // when updated.
    u64 framebuffer_size_last_generation; 

    VkInstance instance;
    VkAllocationCallbacks* allocator;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    vulkan_device device;
    VkSurfaceKHR surface;
    vulkan_swapchain swapchain;
    vulkan_renderpass main_renderpass;
    vulkan_renderpass ui_renderpass;

    vulkan_buffer object_vertex_buffer;
    vulkan_buffer object_index_buffer;
    
    // darray of command buffers
    vulkan_command_buffer* graphics_command_buffers;

    // darray
    VkSemaphore* image_available_semaphores;

    // darray 
    VkSemaphore* queue_complete_semaphores;

    u32 in_flight_fence_count;
    VkFence in_flight_fences[2];

    // Holds pointers to fences which exist and are owned elsewhere, one per frame.
    VkFence* images_in_flight[3];

    u32 image_index;
    u32 current_frame;

    b8 recreating_swapchain;

    vulkan_material_shader material_shader;
    vulkan_ui_shader ui_shader;

    i32 (*find_memory_index)(u32 type_filter, u32 property_flags);

    // Framebuffers used for world rendering, one per frame.
    VkFramebuffer world_framebuffers[3];

    // TODO: Make dynamic
    vulkan_geometry_data geometries[VULKAN_MAX_GEOMETRY_COUNT];

} vulkan_context;

typedef struct vulkan_texture_data {
    vulkan_image image;
    VkSampler sampler;
} vulkan_texture_data;