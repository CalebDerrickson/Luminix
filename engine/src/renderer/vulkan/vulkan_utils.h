#pragma once
#include "vulkan_types.inl"

/**
 * @brief Returns the string representation of result.
 * @param result The result to get the string for.
 * @param get_extended Indicates whether to also return an extended result.
 * @returns The error code and/or extended error message in string form. Defaults to success for unknown result types. 
 */
const char* vulkan_result_string(VkResult result, b8 get_extended);

/**
 * @brief Indicates if the result is a success or an error as defined by the Vulkan spec.
 * @returns True if success; false otherwise. Defaults to true for unknown result types.
 */
b8 vulkan_result_is_success(VkResult result);