#include "vulkan_device.h"
#include "core/logger.h"
#include "core/lmemory.h"
#include "core/lstring.h"
#include "containers/darray.h"

typedef struct vulkan_physical_device_requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    //darray
    const char** device_extension_names;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

// Forward declares
b8 select_physical_device(vulkan_context* context);
b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_info,
    vulkan_swapchain_support_info* out_swapchain_support
);

b8 vulkan_device_create(vulkan_context* context)
{
    LINFO("Selecting physical device...");
    if (!select_physical_device(context)) {
        return false;
    }

    LINFO("Creating logical device...");
    // NOTE: Do not create additional queues for shared indices.
    b8 present_shares_graphics_queue = context->device.graphics_queue_index == context->device.present_queue_index;
    b8 transfer_shares_graphics_queue = context->device.graphics_queue_index == context->device.transfer_queue_index;
    u32 index_count = 1;
    if (!present_shares_graphics_queue) {
        index_count++;
    }
    if (!transfer_shares_graphics_queue) {
        index_count++;
    }
    u32 indices[MAX_NUM_INDICES];
    u8 index = 0;
    indices[index++] = context->device.graphics_queue_index;
    if (!present_shares_graphics_queue) {
        indices[index++] = context->device.present_queue_index;
    }
    if (!transfer_shares_graphics_queue) {
        indices[index++] = context->device.transfer_queue_index;
    }

    VkDeviceQueueCreateInfo queue_create_infos[MAX_NUM_INDICES];
    for (u32 i = 0; i < index_count; ++i) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;
        
        // TODO: Enable this for future enhancement
        // if (indices[i] == context->device.graphics_queue_index) {
        //     queue_create_infos[i].queueCount = 2;
        // }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    // Request device features.
    // TODO: should be config driven
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;  // Request anistrophy

    b8 portability_required = false;

#if LPLATFORM_APPLE
    // Check to see if we are on MacOS

    portability_required = true;
    u32 available_extension_count = 0;
    VkExtensionProperties* available_extensions = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(context->device.physical_device, 0, &available_extension_count, 0));
    if (available_extension_count != 0) {
        available_extensions = lallocate(sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(context->device.physical_device, 0, &available_extension_count, available_extensions));
        for (u32 i = 0; i < available_extension_count; ++i) {
            if (strings_equal(available_extensions[i].extensionName, "VK_KHR_portability_subset")) {
                LINFO("Adding required extension 'VK_KHR_portability_subset'.");
                portability_required = true;
                break;
            }
        }
    }
    lfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);

#endif

    // Appending to required extensions if on Mac
    u32 extension_count = portability_required ? 2 : 1;
    const char** extension_names = portability_required
            ? (const char* [2]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset" }
            : (const char* [1]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME };



    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = extension_count;
    device_create_info.ppEnabledExtensionNames = extension_names;

    // Deprecated and ignored, so pass nothing.
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;


    // Create the device
    VK_CHECK(vkCreateDevice(
        context->device.physical_device,
        &device_create_info,
        context->allocator,
        &context->device.logical_device
    ));

    LINFO("Logical Device Created.");

    // Get queues
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.graphics_queue_index,
        0,
        &context->device.graphics_queue
    );
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.present_queue_index,
        0,
        &context->device.present_queue
    );
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.transfer_queue_index,
        0,
        &context->device.transfer_queue
    );

    LINFO("Queues obtained.");

    // Create command pool for graphics queue.
    VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(
        context->device.logical_device,
        &pool_create_info,
        context->allocator,
        &context->device.graphics_command_pool
    ));
    LINFO("Graphics command pool created.");

    return true;
}

void vulkan_device_destroy(vulkan_context* context)
{
    // Unset the queues
    context->device.graphics_queue = 0;
    context->device.present_queue = 0;
    context->device.transfer_queue = 0;

    LDEBUG("Destroying Command Pool...");
    if (context->device.graphics_command_pool) {
        vkDestroyCommandPool(context->device.logical_device, 
            context->device.graphics_command_pool, 
            context->allocator
        );
        context->device.graphics_command_pool = 0;
    }

    // Destroy the logical device
    LINFO("Destroying logical device...");
    if (context->device.logical_device) {
        vkDestroyDevice(context->device.logical_device, context->allocator);
        context->device.logical_device = 0;
    }

    // Physical devices are not destoryed
    LINFO("Releasing physical device resources...");
    context->device.physical_device = 0;

    if (context->device.swapchain_support.formats) {
        lfree(context->device.swapchain_support.formats,
        sizeof(VkSurfaceFormatKHR) * context->device.swapchain_support.format_count,
        MEMORY_TAG_RENDERER);
        context->device.swapchain_support.formats = 0;
        context->device.swapchain_support.format_count = 0;
    }

    if (context->device.swapchain_support.present_modes) {
        lfree(context->device.swapchain_support.present_modes,
        sizeof(VkPresentModeKHR) * context->device.swapchain_support.present_mode_count,
        MEMORY_TAG_RENDERER);
        context->device.swapchain_support.present_modes = 0;
        context->device.swapchain_support.present_mode_count = 0;
    }

    lzero_memory(
        &context->device.swapchain_support.capabilities,
        sizeof(context->device.swapchain_support.capabilities)
    );

    context->device.graphics_queue_index = -1;
    context->device.present_queue_index = -1;
    context->device.transfer_queue_index = -1;
}

void vulkan_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* out_support_info
)
{
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device,
        surface,
        &out_support_info->capabilities
    ));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        surface,
        &out_support_info->format_count,
        0
    ));

    if (out_support_info->format_count != 0) {
        if (!out_support_info->formats) {
            out_support_info->formats = lallocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface, 
            &out_support_info->format_count,
            out_support_info->formats
        ));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        surface,
        &out_support_info->present_mode_count,
        0
    ));
    if (out_support_info->present_mode_count != 0) {
        if (!out_support_info->present_modes) {
            out_support_info->present_modes = lallocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface, 
            &out_support_info->present_mode_count,
            out_support_info->present_modes
        ));
    }

}

b8 vulkan_device_detect_depth_format(vulkan_device* device)
{
    // Format candidates
    // The 3 is for candidate counts/ size of candidates array.
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT};

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < 3; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(
            device->physical_device, 
            candidates[i], 
            &properties
        );

        if ((properties.linearTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        } else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        }
    }

    return false;
}

b8 select_physical_device(vulkan_context* context)
{
    u32 physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0));
    if (physical_device_count == 0){
        LFATAL("No device which supports Vulkan were found.");
        return false;
    }

    VkPhysicalDevice physical_devices[MAX_DEVICE_COUNT];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices));
    for (u32 i = 0; i < physical_device_count; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);
        
        // Checks if device supports local/host visible combo
        b8 supports_device_local_host_visible = false;
        for (u32 i = 0; i < memory.memoryTypeCount; ++i) {
            // Check each memory type to see if its bit is set to 1.
            if (
                ((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) &&
                ((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
               ) {
                supports_device_local_host_visible = true;
                break;
            }
        }

        // TODO: These requirements should probably be driven by engine
        // configuration.
        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = true;
        requirements.sampler_anisotropy = true;

    #if LPLATFORM_APPLE
        requirements.discrete_gpu = false;
    #else
        requirements.discrete_gpu = true;
    #endif

        requirements.device_extension_names = darray_create(const char*);
        darray_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_info queue_info = {};
        b8 result = physical_device_meets_requirements(
            physical_devices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchain_support
        );

        if (result) {
            LINFO("Selected device: '%s'.", properties.deviceName);
            // GPU type, etc
            switch(properties.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    LINFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    LINFO("GPU type is Discrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    LINFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    LINFO("GPU type is CPU.");
                    break;
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    LINFO("GPU type is Unknown.");
                    break;
            }

            LINFO(
                "GPU driver version: %d.%d.%d",
                VK_API_VERSION_MAJOR(properties.driverVersion),
                VK_API_VERSION_MINOR(properties.driverVersion),
                VK_API_VERSION_PATCH(properties.driverVersion)
            );

            // Vulkan API version.
            LINFO(
                "Vulkan API version: %d.%d.%d",
                VK_API_VERSION_MAJOR(properties.apiVersion),
                VK_API_VERSION_MINOR(properties.apiVersion),
                VK_API_VERSION_PATCH(properties.apiVersion)
            );

            // Memory information
            for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
                f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.f / 1024.f / 1024.f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    LINFO("Local GPU memory: %.2f GiB", memory_size_gib);
                } else {
                    LINFO("Shared System memory: %.2f GiB", memory_size_gib);
                }
            }

            context->device.physical_device = physical_devices[i];
            context->device.graphics_queue_index = queue_info.graphics_family_index;
            context->device.present_queue_index = queue_info.present_family_index;
            context->device.transfer_queue_index = queue_info.transfer_family_index;
            // NOTE: set compute index here if needed.

            // Keep a copy of properties, features and memory infor for later use.
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            context->device.supports_device_local_host_visible = supports_device_local_host_visible;
            break;
        }
    }

    // Ensure a device was selected
    if (!context->device.physical_device) {
        LERROR("No physical devices were found which meet the requirements.");
        return false;
    }

    LINFO("Physical device selected.");
    return true;
}

b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_info,
    vulkan_swapchain_support_info* out_swapchain_support
)
{
// Evaluate device properties to determine if it meets the needs of our applcation.
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    // Discrete GPU?
    if (requirements->discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            LINFO("Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[MAX_QUEUE_FAMILIES];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    // Look at each queue and see what queues it supports
    LINFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; ++i) {
        u8 current_transfer_score = 0;

        // Graphics queue?
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_info->graphics_family_index = i;
            ++current_transfer_score;
        }

        // Compute queue?
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_info->compute_family_index = i;
            ++current_transfer_score;
        }

        // Transfer queue?
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }

        // Present queue?
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if (supports_present) {
            out_queue_info->present_family_index = i;
        }
    }

    // Print out some info about the device
    LINFO("       %d |       %d |       %d |        %d | %s",
          out_queue_info->graphics_family_index != -1,
          out_queue_info->present_family_index != -1,
          out_queue_info->compute_family_index != -1,
          out_queue_info->transfer_family_index != -1,
          properties->deviceName);

    if (
        (!requirements->graphics || (requirements->graphics && out_queue_info->graphics_family_index != -1)) &&
        (!requirements->present || (requirements->present && out_queue_info->present_family_index != -1)) &&
        (!requirements->compute || (requirements->compute && out_queue_info->compute_family_index != -1)) &&
        (!requirements->transfer || (requirements->transfer && out_queue_info->transfer_family_index != -1))) {
        LINFO("Device meets queue requirements.");
        LTRACE("Graphics Family Index: %i", out_queue_info->graphics_family_index);
        LTRACE("Present Family Index:  %i", out_queue_info->present_family_index);
        LTRACE("Transfer Family Index: %i", out_queue_info->transfer_family_index);
        LTRACE("Compute Family Index:  %i", out_queue_info->compute_family_index);

        // Query swapchain support.
        vulkan_device_query_swapchain_support(
            device,
            surface,
            out_swapchain_support);

        if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
            if (out_swapchain_support->formats) {
                lfree(out_swapchain_support->formats, sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count, MEMORY_TAG_RENDERER);
            }
            if (out_swapchain_support->present_modes) {
                lfree(out_swapchain_support->present_modes, sizeof(VkPresentModeKHR) * out_swapchain_support->present_mode_count, MEMORY_TAG_RENDERER);
            }
            LINFO("Required swapchain support not present, skipping device.");
            return false;
        }

        // Device extensions.
        if (requirements->device_extension_names) {
            u32 available_extension_count = 0;
            VkExtensionProperties* available_extensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                0,
                &available_extension_count,
                0
            ));
            if (available_extension_count != 0) {
                available_extensions = lallocate(sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    device,
                    0,
                    &available_extension_count,
                    available_extensions
                ));

                u32 required_extension_count = darray_length(requirements->device_extension_names);
                for (u32 i = 0; i < required_extension_count; ++i) {
                    b8 found = false;
                    for (u32 j = 0; j < available_extension_count; ++j) {
                        if (strings_equal(requirements->device_extension_names[i], available_extensions[j].extensionName)) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        LINFO("Required extension not found: '%s', skipping device.", requirements->device_extension_names[i]);
                        // lfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            lfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
        }

        // Sampler anisotropy
        if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            LINFO("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        // Device meets all requirements.
        return true;
    }

    return false;
}

