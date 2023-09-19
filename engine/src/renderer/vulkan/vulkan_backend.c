#include "vulkan_backend.h"

#include "vulkan_types.inl"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

#include "core/logger.h"
#include "core/lstring.h"
#include "core/lmemory.h"
#include "core/application.h"

#include "containers/darray.h"

#include "platform/platform.h"

#include "math/math_types.h"

// Shaders
#include "shaders/vulkan_object_shader.h"

#include "resources/resource_types.h"

// Static Vulkan context
static vulkan_context context;
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

// Forward declaration
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback( 
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);
i32 find_memory_index(u32 type_filter, u32 property_flags);
void create_command_buffers(renderer_backend* backend);
void regenerate_framebuffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_renderpass* renderpass);
b8 recreate_swapchain(renderer_backend* backend);
b8 create_buffers(vulkan_context* context);

// Temporary function to create a temp buffer and load it to another
// Skirts around the overload of coherent memory to local memory 
// for optimization 
// TODO: Move this.
void upload_data_range(vulkan_context* context, VkCommandPool pool, VkFence fence, VkQueue queue, vulkan_buffer* buffer, u64 offset, u64 size, void* data)
{
    // Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer staging;
    vulkan_buffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    // Load the data into the staging buffer
    vulkan_buffer_load_data(context, &staging, 0, size, 0, data);

    // Perform the copy from staging to the device local buffer
    vulkan_buffer_copy_to(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);

    // Clean up the staging buffer
    vulkan_buffer_destroy(context, &staging);
}

b8 vulkan_renderer_backend_initilize(renderer_backend* backend, const char* application_name)
{
    // Function pointers
    context.find_memory_index = find_memory_index;

    // TODO: Custom allocator
    context.allocator = 0;

    application_get_framebuffer_size(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 800;
    context.framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 600;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    // Setup Vulkan instance
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName = "Luminix Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

    // Obtain a list of required extensions
    const char** required_extensions = darray_create(const char*);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);     // Generic surface extension
    platform_get_required_extension_names(&required_extensions);          // Platform-specific extension(s)

#if (_DEBUG)
    darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Debug utilities

    LDEBUG("Required extensions:");
    u32 length = darray_length(required_extensions);
    for (u32 i = 0; i < length; i++) {
        LDEBUG(required_extensions[i]);
    }
#endif

    // Validation layers
    const char** required_validation_layer_names = 0;
    u32 required_validation_layer_count = 0;

#if defined(_DEBUG)
    LINFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    required_validation_layer_names = darray_create(const char*);
    darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");
    required_validation_layer_count = darray_length(required_validation_layer_names);

    // Obtain a list of available vaildation layers
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

    // Verify all required layers are available.
    for (u32 i = 0; i < required_validation_layer_count; i++) {
        LINFO("Searching for layer %s ...", required_validation_layer_names[i]);
        b8 found = false;

        for (u32 j = 0; j < available_layer_count; j++) {
            if (strings_equal(required_validation_layer_names[i], available_layers[j].layerName)) {
                found = true;
                LINFO("Found.");
                break;
            }
        }

        if (!found) {
            LFATAL("Required validationlayer is missing: %s", required_validation_layer_names[i]);
            return false;
        }
    }

    LINFO("All required validation layers are present.");
#endif

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;
    create_info.enabledExtensionCount = darray_length(required_extensions);
    create_info.ppEnabledExtensionNames = required_extensions;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    LINFO("Vulkan Instance created.");
    
    // Debugger
#if defined(_DEBUG)

    LDEBUG("Creating Vulkan debugger...");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;    
                                                                        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
                                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT 
                                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vk_debug_callback;
    debug_create_info.pUserData = 0;

    PFN_vkCreateDebugUtilsMessengerEXT func = 
        (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    LASSERT_MSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
    LDEBUG("Vulkan debugger created.");
#endif

    // Surface
    LDEBUG("Creating Vulkan surface...");
    if (!platform_create_vulkan_surface(&context)) {
        LERROR("Failed to create platform surface!");
        return false;
    }

    // Device creation
    LDEBUG("Creating devices...");
    if (!vulkan_device_create(&context)) {
        LERROR("Failed to create device!");
        return false;
    }

    // Swapchain creation
    LDEBUG("Creating Swapchain...");
    vulkan_swapchain_create(
        &context,
        context.framebuffer_width,
        context.framebuffer_height,
        &context.swapchain
    );

    // Renderpass creation
    LDEBUG("Creating main renderpass...");
    vulkan_renderpass_create(
        &context,
        &context.main_renderpass,
        0, 0, context.framebuffer_width, context.framebuffer_height,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f, 0
    );

    // Create Swapchain framebuffers
    context.swapchain.framebuffers = darray_reserve(vulkan_framebuffer, context.swapchain.image_count);
    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    // Command Buffers creation
    LDEBUG("Creating command buffers...");
    create_command_buffers(backend);

    LDEBUG("Creating sync objects...");
    context.image_available_semaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.queue_complete_seamphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.in_flight_fences = darray_reserve(vulkan_fence, context.swapchain.max_frames_in_flight);

    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logical_device,
            &semaphore_create_info, 
            context.allocator, 
            &context.image_available_semaphores[i]
        );
        vkCreateSemaphore(context.device.logical_device, 
            &semaphore_create_info, 
            context.allocator, 
            &context.queue_complete_seamphores[i]
        );

        // Create the fence in a signaled state, indicating that the first frame has already been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it 
        // cannot be rendered unitl a frame is "rendered" before it.
        vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the intitial state should be 0, and will be 0 when not in use. Actual fences are not owned
    // by this list.
    context.images_in_flight = darray_reserve(vulkan_fence, context.swapchain.image_count);
    for (u32 i = 0; i < context.swapchain.image_count; i++) {
        context.images_in_flight[i] = 0;
    }

    // Create builtin shaders
    if (!vulkan_object_shader_create(&context, &context.object_shader)) {
        LERROR("Error loading built-in basic_lighting shader.");
        return false;
    }

    LDEBUG("Creating standard buffers...");
    if (!create_buffers(&context)) {
        LERROR("Error creating standard buffers.");
        return false;
    }

    // TODO: temporary test code.
    const u32 vert_count = 4;
    vertex_3d verts[vert_count];
    lzero_memory(verts, sizeof(vertex_3d) * vert_count);

    const f32 factor = 10.0f;

    verts[0].position.x = -0.5 * factor;
    verts[0].position.y = -0.5 * factor;
    verts[0].texcoord.x = 0.0f;
    verts[0].texcoord.y = 0.0f;

    verts[1].position.x = 0.5 * factor;
    verts[1].position.y = 0.5 * factor;
    verts[1].texcoord.x = 1.0f;
    verts[1].texcoord.y = 1.0f;

    verts[2].position.x = -0.5 * factor;
    verts[2].position.y = 0.5 * factor;
    verts[2].texcoord.x = 0.0f;
    verts[2].texcoord.y = 1.0f;

    verts[3].position.x = 0.5 * factor;
    verts[3].position.y = -0.5 * factor;
    verts[3].texcoord.x = 1.0f;    
    verts[3].texcoord.y = 0.0f;

    const u32 index_count = 6;
    u32 indices[index_count] = {0, 1, 2, 0, 3, 1};

    upload_data_range(
        &context,
        context.device.graphics_command_pool,
        0,                                      // No Fence
        context.device.graphics_queue,
        &context.object_vertex_buffer,
        0,                                      // No Offset
        sizeof(vertex_3d) * vert_count,
        verts
    );

        upload_data_range(
        &context,
        context.device.graphics_command_pool,
        0,                                      // No Fence
        context.device.graphics_queue,
        &context.object_index_buffer,
        0,                                      // No Offset
        sizeof(u32) * index_count,
        indices
    );

    u32 object_id = 0;
    if (!vulkan_object_shader_acquire_resources(&context, &context.object_shader, &object_id)) {
        LERROR("Failed to acquire shader resources.");
        return false;
    }
    // TODO: End test code

    LINFO("Vulkan Renderer initialized successfully.");

    return true;
}

void vulkan_renderer_backend_shutdown(renderer_backend* backend)
{
    vkDeviceWaitIdle(context.device.logical_device);
    // Destroy in order opposite of creation

    LDEBUG("Destroying buffer objects...");
    vulkan_buffer_destroy(&context, &context.object_vertex_buffer);
    vulkan_buffer_destroy(&context, &context.object_index_buffer);

    LDEBUG("Destroying shader objects...");
    vulkan_object_shader_destroy(&context, &context.object_shader);

    LDEBUG("Destroying sync objects...");
    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
        if (context.image_available_semaphores[i]) {
            vkDestroySemaphore(
                context.device.logical_device,
                context.image_available_semaphores[i],
                context.allocator
            );
            context.image_available_semaphores[i] = 0;
        }
        if (context.queue_complete_seamphores[i]) {
            vkDestroySemaphore(
                context.device.logical_device,
                context.queue_complete_seamphores[i],
                context.allocator
            );
            context.queue_complete_seamphores[i] = 0;
        }
        vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
    }

    darray_destroy(context.image_available_semaphores);
    context.image_available_semaphores = 0;

    darray_destroy(context.queue_complete_seamphores);
    context.queue_complete_seamphores = 0;

    darray_destroy(context.in_flight_fences);
    context.in_flight_fences = 0;

    darray_destroy(context.images_in_flight);
    context.images_in_flight = 0;

    LDEBUG("Destroying Command buffers...")
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        if (context.graphics_command_buffers[i].handle) {
            vulkan_command_buffer_free(
                &context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]);
            context.graphics_command_buffers[i].handle = 0;
        }
    }
    darray_destroy(context.graphics_command_buffers);
    context.graphics_command_buffers = 0;
    
    // Destroy framebuffers
    for (u32 i = 0; i < context.swapchain.image_count; i++) {
        vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
    }

    LDEBUG("Destroying renderpass...");
    vulkan_renderpass_destroy(&context, &context.main_renderpass);

    LDEBUG("Destroying swapchain...");
    vulkan_swapchain_destroy(&context, &context.swapchain);

    LDEBUG("Destroying Vulkan device..."); 
    vulkan_device_destroy(&context);

    LDEBUG("Destroying Vulkan surface...");
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

    LDEBUG("Destroying Vulkan Debugger...");
    if (context.debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func = 
        (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }

    LDEBUG("Destroying Vulkan Instance...");
    vkDestroyInstance(context.instance, context.allocator);

}

void vulkan_renderer_backend_on_resized(renderer_backend* backend, u16 width, u16 height)
{
    // Update the "framebuffer size generation", a counter which indicates when the 
    // framebuffer has been updated.
    cached_framebuffer_width = width;
    cached_framebuffer_height = height;
    context.framebuffer_size_generation++;

    LINFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width, height, context.framebuffer_size_generation);

}

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time)
{
    context.frame_delta_time = delta_time;

    // Making a local copy for readability.
    vulkan_device* device = &context.device; 
    
    // Check if recreating swapchain and boot out.
    if (context.recreating_swapchain) {
        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if (!vulkan_result_is_success(result)) {
            LERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) failed: '%s'", vulkan_result_string(result, true));
            return false;
        }
        LINFO("Recreating swapchain, booting.");
        return false;
    } 
    
    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if (context.framebuffer_size_generation != context.framebuffer_size_last_generation) {
        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if (!vulkan_result_is_success(result)) {
            LERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (2) failed: '%s'", vulkan_result_string(result, true));
            return false;
        }
    

        // If the swapchain failed, boot out before unsetting the flag
        if (!recreate_swapchain(backend)) {
            return false;
        }

        LINFO("Resized, booting.");
        return false;
    }

    // Wait for the execution of the current frame to complete.
    // The fence being free will allow this one to move on. 
    // I don't know why UINT64_MAX shows up as an error
    if (!vulkan_fence_wait(
        &context,
        &context.in_flight_fences[context.current_frame],
        UINT64_MAX
    )) {
        LWARN("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swapchain. Pass along the semaphore that should be signaled when this completes.
    // this same semaphore will later be waited on by the queue submission to ensure this image is available.
    // I don't know why UINT64_MAX shows up as an error
    if (!vulkan_swapchain_acquire_next_image_index(
        &context,
        &context.swapchain,
        UINT64_MAX,
        context.image_available_semaphores[context.current_frame],
        0,
        &context.image_index
    )) {
        return false;
    }

    // Begin recording commands
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    vulkan_command_buffer_reset(command_buffer);
    vulkan_command_buffer_begin(command_buffer, false, false, false);

    // Dynamic state. Settings should be consistent with OpenGL for easy shaders
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebuffer_height;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = -(f32)context.framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.height = context.framebuffer_height;
    scissor.extent.width = context.framebuffer_width;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    // Begin the render pass
    vulkan_renderpass_begin(
        command_buffer,
        &context.main_renderpass,
        context.swapchain.framebuffers[context.image_index].handle
    );



    return true;
}

void vulkan_renderer_update_global_state(
    mat4 projection, 
    mat4 view, 
    vec3 view_position, 
    vec4 ambient_color, 
    i32 mode
)
{
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    
    vulkan_object_shader_use(&context, &context.object_shader);

    context.object_shader.global_ubo.projection = projection;
    context.object_shader.global_ubo.view = view;
    
    // TODO: Other ubo properties

    vulkan_object_shader_update_global_state(&context, &context.object_shader, context.frame_delta_time);
}

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time)
{
    // for readability
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];

    // End renderpass
    vulkan_renderpass_end(command_buffer, &context.main_renderpass);

    // End command buffer
    vulkan_command_buffer_end(command_buffer);

    // Make sure the previous frame is not using this image (its fence is being waited on)
    // I don't know why UINT64_MAX shows up as an error
    if (context.images_in_flight[context.image_index] != VK_NULL_HANDLE) {
        vulkan_fence_wait(
            &context,
            context.images_in_flight[context.image_index],
            UINT64_MAX
        );
    }

    // Mark the image fence as in-use by this frame.
    context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];

    // Reset the fence for use on the next frame
    vulkan_fence_reset(&context, &context.in_flight_fences[context.current_frame]);

    // Submit the queue and wait for the operation to complete
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // Command buffer(s) to be executed
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    // The semaphore(s) to be signaled when the queue is complete.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &context.queue_complete_seamphores[context.current_frame];

    // Wait semaphore ensures that the operation cannot begin until the image is available.
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];

    // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
    // writes from executing until the semaphore signals (on frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphics_queue,
        1,
        &submit_info,
        context.in_flight_fences[context.current_frame].handle
    );

    if (result != VK_SUCCESS) {
        LERROR("vkQueueSubmit failed with result: %s", vulkan_result_string(result, true));
        return false;
    }

    vulkan_command_buffer_update_submitted(command_buffer);
    // End queue submission

    // Give the image back to the swapchain
    vulkan_swapchain_present(
        &context,
        &context.swapchain,
        context.device.graphics_queue,
        context.device.present_queue,
        context.queue_complete_seamphores[context.current_frame],
        context.image_index
    );

    return true;
}

void vulkan_backend_update_object(geometry_render_data data)
{
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];

    vulkan_object_shader_update_object(&context, &context.object_shader, data);
    
    // TODO: temp test code
    vulkan_object_shader_use(&context, &context.object_shader);

    // Bind the vertex buffer at offset.
    VkDeviceSize offsets[1] = {0};

    vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.object_vertex_buffer.handle, (VkDeviceSize*)offsets);

    // Bind index buffer at offset.
    vkCmdBindIndexBuffer(command_buffer->handle, context.object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

    // Issue the draw command
    vkCmdDrawIndexed(command_buffer->handle, 6, 1, 0, 0, 0);
    // TODO: End temp test code
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback( 
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data 
    )
{
    switch(message_severity) 
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LERROR(callback_data->pMessage);
            break;
        
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LWARN(callback_data->pMessage);
            break;
        
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LINFO(callback_data->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LTRACE(callback_data->pMessage);
            break;

        default:
            break;
    }
    return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    
    vkGetPhysicalDeviceMemoryProperties(
        context.device.physical_device, 
        &memory_properties
    );

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        // Check each memory type to see if its bit is set to 1.
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }

    LWARN("Unable to find suitable memory type!");
    return -1;
}

void create_command_buffers(renderer_backend* backend)
{
    if (!context.graphics_command_buffers) {
        context.graphics_command_buffers = darray_reserve(vulkan_command_buffer, context.swapchain.image_count);
        for (u32 i = 0; i < context.swapchain.image_count; i++) {
            lzero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
        }
    }

    for (u32 i = 0; i < context.swapchain.image_count; i++) {
        if (context.graphics_command_buffers[i].handle) {
            vulkan_command_buffer_free(
                &context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]
            );
        }
        lzero_memory(&context.graphics_command_buffers[i],sizeof(vulkan_command_buffer));
        vulkan_command_buffer_allocate(
            &context,
            context.device.graphics_command_pool,
            true,
            &context.graphics_command_buffers[i]
        );
    }
}

void regenerate_framebuffers(
    renderer_backend* backend, 
    vulkan_swapchain* swapchain, 
    vulkan_renderpass* renderpass
)
{
    for (u32 i = 0; i < swapchain->image_count; i++) {
        // TODO: Make this dynamic based on the currently configured attachments
        u32 attachment_count = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depth_attachment.view};
        
        vulkan_framebuffer_create(
            &context, 
            &context.main_renderpass,
            context.framebuffer_width,
            context.framebuffer_height,
            attachment_count,
            attachments,
            &context.swapchain.framebuffers[i]
        );
    }

}

b8 recreate_swapchain(renderer_backend* backend)
{
    // If already being recreated, do not try again.
    if (context.recreating_swapchain) {
        LDEBUG("recreate_swapchain called when already recreating. Booting.");
        return false;
    }

    // Detect if the window is too small to be drawn to
    if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
        LDEBUG("recreate_swapchain called when window is < 1 in a dimension. Booting.");
        return false;
    }

    // Mark as recreating if the dimensions are valid
    context.recreating_swapchain = true;

    // Wait for any operations to complete.
    vkDeviceWaitIdle(context.device.logical_device);

    // Clear these out just in case.
    for (u32 i = 0; i < context.swapchain.image_count; i++) {
        context.images_in_flight[i] = 0;
    }

    // Requery support
    vulkan_device_query_swapchain_support(
        context.device.physical_device,
        context.surface,
        &context.device.swapchain_support
    );
    vulkan_device_detect_depth_format(&context.device);

    vulkan_swapchain_recreate(
        &context,
        cached_framebuffer_width,
        cached_framebuffer_height,
        &context.swapchain
    ); 

    // Sync the framebuffer size with the cached sizes.
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;
    cached_framebuffer_width = 0; 
    cached_framebuffer_height = 0;

    // Update framebuffer size generation.
    context.framebuffer_size_last_generation = context.framebuffer_size_generation;

    // Cleanup swapchain
    for (u32 i = 0; i < context.swapchain.image_count; i++) {
        vulkan_command_buffer_free(
            &context,
            context.device.graphics_command_pool,
            &context.graphics_command_buffers[i]
        );
    }

    // Framebuffers
    for (u32 i = 0; i < context.swapchain.image_count; i++) {
        vulkan_framebuffer_destroy (
            &context,
            &context.swapchain.framebuffers[i]
        );
    }

    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;
    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    // Clear the recreating flag.
    context.recreating_swapchain = false;

    return true;

}

b8 create_buffers(vulkan_context* context)
{
    VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const u64 vertex_buffer_size = sizeof(vertex_3d) * 1024 * 1024;

    if (!vulkan_buffer_create(
        context,
        vertex_buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        memory_property_flags,
        true,
        &context->object_vertex_buffer 
    )) {
        LERROR("Error creating vertex buffer.");
        return false;
    }

    context->geometry_vertex_offset = 0;

    const u64 index_buffer_size = sizeof(u32) * 1024 * 1024;

    if (!vulkan_buffer_create(
        context,
        index_buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        memory_property_flags,
        true,
        &context->object_index_buffer
    )) {
        LERROR("Error creating index buffer.");
        return false;
    }
    context->geometry_index_offset = 0;
    
    return true;
}

void vulkan_renderer_create_texture(
    const char* name,
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8* pixels,
    b8 has_transparency,
    texture* out_texture
)
{
    out_texture->width = width;
    out_texture->height = height;
    out_texture->channel_count = channel_count;
    out_texture->generation = INVALID_ID;

    // Internal data creation
    // TODO: Use an allocator for this
    out_texture->internal_data = (vulkan_texture_data*)lallocate(sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
    vulkan_texture_data* data = (vulkan_texture_data*)out_texture->internal_data;
    VkDeviceSize image_size = width * height * channel_count;

    // NOTE: Assumes 8 bits per channel!
    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a staging buffer and load data into it
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memory_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer staging;
    vulkan_buffer_create(&context, image_size, usage, memory_prop_flags, true, &staging); 

    vulkan_buffer_load_data(&context, &staging, 0, image_size, 0, pixels);

    // NOTE: Lots of assumptions here, different texture types will require
    // different options here.
    vulkan_image_create(
        &context,
        VK_IMAGE_TYPE_2D,
        width,
        height,
        image_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &data->image
    );

    vulkan_command_buffer temp_buffer;
    VkCommandPool pool = context.device.graphics_command_pool;
    VkQueue queue = context.device.graphics_queue;
    vulkan_command_buffer_allocate_and_begin_single_use(&context, pool, &temp_buffer);

    // Transition the layout from whatever it is currently to optimal for receiving data.
    vulkan_image_transition_layout(
        &context,
        &temp_buffer,
        &data->image,
        image_format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    // Copy the data from the buffer
    vulkan_image_copy_from_buffer(&context, &temp_buffer, &data->image,  staging.handle);



    // Transition from optimal for data receipt to shader-read-only optimal layout
    vulkan_image_transition_layout(
        &context,
        &temp_buffer,
        &data->image,
        image_format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    vulkan_command_buffer_end_single_use(&context, pool, &temp_buffer, queue);

    vulkan_buffer_destroy(&context, &staging);

    // Create a sampler for the texture
    VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    // TODO: These filters should be configurable.
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 16;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    VkResult result = vkCreateSampler(context.device.logical_device, &sampler_info, context.allocator, &data->sampler);
    if (!vulkan_result_is_success(result)) {
        LERROR("Error createing texture sampler: %s", vulkan_result_string(result, true));
        return;
    }

    out_texture->has_transparency = has_transparency;
    out_texture->generation++;
}

void vulkan_renderer_destroy_texture(texture* texture)
{
    vkDeviceWaitIdle(context.device.logical_device);

    vulkan_texture_data* data = (vulkan_texture_data*)texture->internal_data;

    vulkan_image_destroy(&context, &data->image);
    lzero_memory(&data->image, sizeof(vulkan_image));
    
    vkDestroySampler(context.device.logical_device, data->sampler, context.allocator);
    data->sampler = 0;

    lfree(texture->internal_data, sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
    lzero_memory(texture, sizeof(struct texture));
}
