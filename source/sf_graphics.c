#include "sf_graphics.h"

#include <stdio.h>

#define SF_POW(b, p) pow(b, p)
#define SF_SNPRINTF snprintf

#define sf_log_error(...) fprintf(stderr, "\033[31m[ERROR]: \033[m" __VA_ARGS__)
#define sf_log_info(...) fprintf(stdout, "[INFO]:" __VA_ARGS__)
#define sf_log_warning(...) fprintf(stdout, "\033[31m[WARNING]: \033[m" __VA_ARGS__)
#define sf_log_verbose(...) fprintf(stdout, "[VERBOSE]:" __VA_ARGS__)

SF_INTERNAL sf_graphics_texture *sf_graphics_get_texture_from_resource_pool(sf_graphics_renderer *r) {
  sf_graphics_texture *result = NULL;

  for (u32 i = 1; i < SF_SIZE(r->texture_pool) && !result; ++i) {
    sf_graphics_texture *current = &r->texture_pool[i];

    if (!current->is_occupied) {
      result              = current;
      result->is_occupied = SF_TRUE;
    }
  }

  return result;
}

SF_INTERNAL sf_graphics_is_null_handle(sf_handle handle) {
  sf_handle null_handle = SF_NULL_HANDLE;
  return handle.value == null_handle.value;
}

SF_INTERNAL sf_handle sf_graphics_handle_from_texture(sf_graphics_renderer *r, sf_graphics_texture *texture) {
  sf_handle result = {0};

  if (texture) {
    result.value = (uintptr_t)(texture - &r->texture_pool[0]);
  }

  return result;
}

SF_INTERNAL sf_graphics_texture *sf_graphics_texture_from_handle(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_texture *result = NULL;

  if (handle.value < SF_SIZE(r->texture_pool)) {
    result = &r->texture_pool[handle.value];
  }

  return result;
}

SF_INTERNAL sf_graphics_render_target *sf_graphics_get_render_target_from_resource_pool(sf_graphics_renderer *r) {
  sf_graphics_render_target *result = NULL;

  for (u32 i = 1; i < SF_SIZE(r->render_target_pool) && !result; ++i) {
    sf_graphics_render_target *current = &r->render_target_pool[i];

    if (!current->is_occupied) {
      result              = current;
      result->is_occupied = SF_TRUE;
    }
  }

  return result;
}

SF_INTERNAL sf_handle sf_graphics_handle_from_render_target(sf_graphics_renderer *r, sf_graphics_render_target *render_target) {
  sf_handle result = {0};

  if (render_target) {
    result.value = (uintptr_t)(render_target - &r->render_target_pool[0]);
  }

  return result;
}

SF_INTERNAL sf_graphics_render_target *sf_graphics_render_target_from_handle(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_render_target *result = NULL;

  if (handle.value < SF_SIZE(r->render_target_pool)) {
    result = &r->render_target_pool[handle.value];
  }

  return result;
}

SF_INTERNAL sf_graphics_command_buffer *sf_graphics_get_command_buffer_from_resource_pool(sf_graphics_renderer *r) {
  sf_graphics_command_buffer *result = NULL;

  for (u32 i = 1; i < SF_SIZE(r->command_buffer_pool) && !result; ++i) {
    sf_graphics_command_buffer *current = &r->command_buffer_pool[i];

    if (!current->is_occupied) {
      result              = current;
      result->is_occupied = SF_TRUE;
    }
  }

  return result;
}

SF_INTERNAL sf_handle sf_graphics_handle_from_command_buffer(sf_graphics_renderer *r, sf_graphics_command_buffer *command_buffer) {
  sf_handle result = {0};

  if (command_buffer) {
    result.value = (uintptr_t)(command_buffer - &r->command_buffer_pool[0]);
  }

  return result;
}

SF_INTERNAL sf_graphics_command_buffer *sf_graphics_command_buffer_from_handle(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_command_buffer *result = NULL;

  if (handle.value < SF_SIZE(r->command_buffer_pool)) {
    result = &r->command_buffer_pool[handle.value];
  }

  return result;
}
SF_INTERNAL sf_graphics_pipeline *sf_graphics_get_pipeline_from_resource_pool(sf_graphics_renderer *r) {
  sf_graphics_pipeline *result = NULL;

  for (u32 i = 1; i < SF_SIZE(r->pipeline_pool) && !result; ++i) {
    sf_graphics_pipeline *current = &r->pipeline_pool[i];

    if (!current->is_occupied) {
      result              = current;
      result->is_occupied = SF_TRUE;
    }
  }

  return result;
}

SF_INTERNAL sf_handle sf_graphics_handle_from_pipeline(sf_graphics_renderer *r, sf_graphics_pipeline *pipeline) {
  sf_handle result = {0};

  if (pipeline) {
    result.value = (uintptr_t)(pipeline - &r->pipeline_pool[0]);
  }

  return result;
}

SF_INTERNAL sf_graphics_pipeline *sf_graphics_pipeline_from_handle(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_pipeline *result = NULL;

  if (handle.value < SF_SIZE(r->pipeline_pool)) {
    result = &r->pipeline_pool[handle.value];
  }

  return result;
}

SF_INTERNAL sf_string sf_graphics_string_from_vulkan_result(VkResult vk_result) {
  sf_string result = {0};

  switch (vk_result) {
    case VK_SUCCESS:                        result = SF_STRING("VK_SUCCESS"); break;
    case VK_NOT_READY:                      result = SF_STRING("VK_NOT_READY"); break;
    case VK_TIMEOUT:                        result = SF_STRING("VK_TIMEOUT"); break;
    case VK_EVENT_SET:                      result = SF_STRING("VK_EVENT_SET"); break;
    case VK_EVENT_RESET:                    result = SF_STRING("VK_EVENT_RESET"); break;
    case VK_INCOMPLETE:                     result = SF_STRING("VK_INCOMPLETE"); break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:       result = SF_STRING("VK_ERROR_OUT_OF_HOST_MEMORY"); break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:     result = SF_STRING("VK_ERROR_OUT_OF_DEVICE_MEMORY"); break;
    case VK_ERROR_INITIALIZATION_FAILED:    result = SF_STRING("VK_ERROR_INITIALIZATION_FAILED"); break;
    case VK_ERROR_DEVICE_LOST:              result = SF_STRING("VK_ERROR_DEVICE_LOST"); break;
    case VK_ERROR_MEMORY_MAP_FAILED:        result = SF_STRING("VK_ERROR_MEMORY_MAP_FAILED"); break;
    case VK_ERROR_LAYER_NOT_PRESENT:        result = SF_STRING("VK_ERROR_LAYER_NOT_PRESENT"); break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:    result = SF_STRING("VK_ERROR_EXTENSION_NOT_PRESENT"); break;
    case VK_ERROR_FEATURE_NOT_PRESENT:      result = SF_STRING("VK_ERROR_FEATURE_NOT_PRESENT"); break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:      result = SF_STRING("VK_ERROR_INCOMPATIBLE_DRIVER"); break;
    case VK_ERROR_TOO_MANY_OBJECTS:         result = SF_STRING("VK_ERROR_TOO_MANY_OBJECTS"); break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED:     result = SF_STRING("VK_ERROR_FORMAT_NOT_SUPPORTED"); break;
    case VK_ERROR_FRAGMENTED_POOL:          result = SF_STRING("VK_ERROR_FRAGMENTED_POOL"); break;
    case VK_ERROR_OUT_OF_POOL_MEMORY:       result = SF_STRING("VK_ERROR_OUT_OF_POOL_MEMORY"); break;
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:  result = SF_STRING("VK_ERROR_INVALID_EXTERNAL_HANDLE"); break;
    case VK_ERROR_SURFACE_LOST_KHR:         result = SF_STRING("VK_ERROR_SURFACE_LOST_KHR"); break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: result = SF_STRING("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"); break;
    case VK_SUBOPTIMAL_KHR:                 result = SF_STRING("VK_SUBOPTIMAL_KHR"); break;
    case VK_ERROR_OUT_OF_DATE_KHR:          result = SF_STRING("VK_ERROR_OUT_OF_DATE_KHR"); break;

    case VK_ERROR_UNKNOWN:
    default:               result = SF_STRING("VK_ERROR_UNKNOWN"); break;
  }

  return result;
}

static sf_bool sf_graphics_vulkan_check(VkResult result, sf_string what, int line, sf_string file) {
  sf_string result_string = sf_graphics_string_from_vulkan_result(result);
  fprintf(stderr, "%.*s - %.*s - %.*s:%i\n", (unsigned int)result_string.size, result_string.data, (unsigned int)what.size, what.data,
          (unsigned int)file.size, file.data, line);
  return result == VK_SUCCESS;
}
#define SF_VULKAN_CHECK(e) sf_graphics_vulkan_check((e), SF_STRING(#e), __LINE__, SF_STRING(__FILE__))

static VkBool32 VKAPI_CALL sf_graphics_vulkan_log(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                  VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                  const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) {
  (void)messageTypes;
  (void)userData;

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: sf_log_verbose("%s\n", callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    sf_log_info("%s\n", callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: sf_log_warning("%s\n", callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   sf_log_error("%s\n", callbackData->pMessage); break;
    default:                                              sf_log_info("%s\n", callbackData->pMessage); break;
  }

  return VK_TRUE;
}

SF_INTERNAL sf_bool sf_graphics_are_queue_family_indices_valid(sf_graphics_renderer *r) {
  return (u32)-1 != r->vk_graphics_queue_family_index && (u32)-1 != r->vk_present_queue_family_index;
}

typedef struct sf_graphics_queue_family_property_list {
  u32                      size;
  VkQueueFamilyProperties *data;
} sf_graphics_queue_family_property_list;

SF_INTERNAL sf_graphics_queue_family_property_list sf_graphics_create_queue_family_property_list(sf_arena *arena, VkPhysicalDevice device) {
  sf_graphics_queue_family_property_list result = {0};

  vkGetPhysicalDeviceQueueFamilyProperties(device, &result.size, NULL);

  if (result.size) {
    result.data = sf_arena_allocate(arena, result.size * sizeof(*result.data));
    if (result.data)
      vkGetPhysicalDeviceQueueFamilyProperties(device, &result.size, result.data);
  }

  return result;
}

SF_INTERNAL void sf_graphics_find_suitable_queue_family_indices(sf_arena *arena, sf_graphics_renderer *r) {
  sf_graphics_queue_family_property_list properties = sf_graphics_create_queue_family_property_list(arena, r->vk_physical_device);

  if (properties.size && properties.data) {
    for (u32 i = 0; i < properties.size && !sf_graphics_are_queue_family_indices_valid(r); ++i) {
      VkBool32 supports_surface = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(r->vk_physical_device, i, r->vk_surface, &supports_surface);

      if (supports_surface)
        r->vk_present_queue_family_index = i;

      if (properties.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        r->vk_graphics_queue_family_index = i;
    }
  }
}

typedef struct sf_graphics_extension_property_list {
  u32                    size;
  VkExtensionProperties *data;
} sf_graphics_extension_property_list;

SF_INTERNAL sf_graphics_extension_property_list sf_graphics_create_extension_property_list(sf_arena *arena, VkPhysicalDevice device) {
  sf_graphics_extension_property_list result = {0};

  if (SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &result.size, NULL))) {
    result.data = sf_arena_allocate(arena, result.size * sizeof(*result.data));
    if (result.data)
      vkEnumerateDeviceExtensionProperties(device, NULL, &result.size, result.data);
  }

  return result;
}

SF_INTERNAL sf_bool sf_graphics_check_device_extension_support(sf_arena *arena, sf_graphics_renderer *r,
                                                               sf_graphics_renderer_description *description) {
  sf_bool                             foundAll  = SF_TRUE;
  sf_graphics_extension_property_list available = sf_graphics_create_extension_property_list(arena, r->vk_physical_device);

  for (u32 i = 0; i < description->vk_device_extension_count && foundAll; ++i) {
    sf_bool   found_current = SF_FALSE;
    sf_string required      = sf_string_from_non_literal(description->vk_device_extensions[i], VK_MAX_EXTENSION_NAME_SIZE);

    for (u32 j = 0; j < available.size && !found_current; ++j) {
      sf_string current_available = sf_string_from_non_literal(available.data[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE);
      found_current               = sf_string_compare(required, current_available, VK_MAX_EXTENSION_NAME_SIZE);
    }

    foundAll = foundAll && found_current;
  }

  return foundAll;
}

SF_INTERNAL sf_bool sf_graphics_check_device_swapchain_support(sf_graphics_renderer *r) {
  u32 surface_format_count = 0, present_mode_count = 0;

  SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vk_physical_device, r->vk_surface, &surface_format_count, NULL));
  SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vk_physical_device, r->vk_surface, &present_mode_count, NULL));

  return surface_format_count && present_mode_count;
}

typedef struct sf_graphics_surface_format_list {
  u32                 size;
  VkSurfaceFormatKHR *data;
} sf_graphics_surface_format_list;

SF_INTERNAL sf_graphics_surface_format_list sf_graphics_create_surface_format_list(sf_arena *arena, VkPhysicalDevice device,
                                                                                   VkSurfaceKHR surface) {
  sf_graphics_surface_format_list result = {0};

  if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &result.size, NULL))) {
    result.data = sf_arena_allocate(arena, result.size * sizeof(result.data));
    if (result.data)
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &result.size, result.data);
  }

  return result;
}

typedef struct sf_graphics_present_mode_list {
  u32               size;
  VkPresentModeKHR *data;
} sf_graphics_present_mode_list;

SF_INTERNAL sf_graphics_present_mode_list sf_graphics_create_present_mode_list(sf_arena *arena, VkPhysicalDevice device,
                                                                               VkSurfaceKHR surface) {
  sf_graphics_present_mode_list result = {0};

  if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &result.size, NULL))) {
    result.data = sf_arena_allocate(arena, result.size * sizeof(result.data));
    if (result.data)
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &result.size, result.data);
  }

  return result;
}

SF_INTERNAL sf_bool sf_graphics_test_format_features(VkPhysicalDevice device, VkFormat format, VkImageTiling tiling,
                                                     VkFormatFeatureFlags features) {
  sf_bool            result     = SF_FALSE;
  VkFormatProperties properties = {0};
  vkGetPhysicalDeviceFormatProperties(device, format, &properties);

  if (VK_IMAGE_TILING_LINEAR == tiling)
    result = !!(properties.linearTilingFeatures & features);
  else if (VK_IMAGE_TILING_OPTIMAL == tiling)
    result = !!(properties.optimalTilingFeatures & features);

  return result;
}

SF_INTERNAL u32 sf_graphics_find_memory_type_index(VkPhysicalDevice device, VkMemoryPropertyFlags memory_properties, u32 filter) {
  u32 result = (u32)-1;

  VkPhysicalDeviceMemoryProperties available = {0};
  vkGetPhysicalDeviceMemoryProperties(device, &available);

  for (u32 i = 0; i < available.memoryTypeCount && result == (u32)-1; ++i)
    if ((filter & (1 << i)) && (available.memoryTypes[i].propertyFlags & memory_properties) == memory_properties)
      result = i;

  return result;
}

SF_INTERNAL VkDeviceMemory sf_graphics_allocate_memory(sf_graphics_renderer *r, VkMemoryPropertyFlags memory_properties, u32 filter,
                                                       u64 size) {
  VkDeviceMemory memory = VK_NULL_HANDLE;

  if (r && r->vk_device && size) {
    u32 memory_type_index = sf_graphics_find_memory_type_index(r->vk_physical_device, memory_properties, filter);
    if (memory_type_index != (u32)-1) {
      VkMemoryAllocateInfo info = {0};
      info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      info.pNext                = NULL;
      info.allocationSize       = size;
      info.memoryTypeIndex      = memory_type_index;
      if (!SF_VULKAN_CHECK(vkAllocateMemory(r->vk_device, &info, r->vk_allocation_callbacks, &memory)))
        memory = VK_NULL_HANDLE;
    }
  }

  return memory;
}

SF_INTERNAL VkDeviceMemory sf_graphics_allocate_memory_for_image(sf_graphics_renderer *r, VkImage image,
                                                                 VkMemoryPropertyFlags memory_properties) {
  VkDeviceMemory memory = VK_NULL_HANDLE;

  if (r && r->vk_device && image) {
    VkMemoryRequirements requirements = {0};
    vkGetImageMemoryRequirements(r->vk_device, image, &requirements);

    memory = sf_graphics_allocate_memory(r, memory_properties, requirements.memoryTypeBits, requirements.size);
    if (memory) {
      if (!SF_VULKAN_CHECK(vkBindImageMemory(r->vk_device, image, memory, 0))) {
        vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
        memory = VK_NULL_HANDLE;
      }
    }
  }

  return memory;
}

SF_INTERNAL VkDeviceMemory sf_graphics_alloacte_memory_for_buffer(sf_graphics_renderer *r, VkBuffer buffer,
                                                                  VkMemoryPropertyFlags memory_properties) {
  VkDeviceMemory memory = VK_NULL_HANDLE;

  if (r && r->vk_device && buffer) {
    VkMemoryRequirements requirements = {0};
    vkGetBufferMemoryRequirements(r->vk_device, buffer, &requirements);

    memory = sf_graphics_allocate_memory(r, memory_properties, requirements.memoryTypeBits, requirements.size);
    if (memory) {
      if (!SF_VULKAN_CHECK(vkBindBufferMemory(r->vk_device, buffer, memory, 0))) {
        vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
        memory = VK_NULL_HANDLE;
      }
    }
  }

  return memory;
}

SF_INTERNAL VkImageViewType sf_graphics_vulkan_ikmage_view_type_from_texture_type(sf_graphics_texture_type type) {
  VkImageViewType result = VK_IMAGE_VIEW_TYPE_1D;

  switch (type) {
    case SF_GRAPHICS_TEXTURE_TYPE_1D:   result = VK_IMAGE_VIEW_TYPE_1D; break;
    case SF_GRAPHICS_TEXTURE_TYPE_2D:   result = VK_IMAGE_VIEW_TYPE_2D; break;
    case SF_GRAPHICS_TEXTURE_TYPE_3D:   result = VK_IMAGE_VIEW_TYPE_3D; break;
    case SF_GRAPHICS_TEXTURE_TYPE_CUBE: result = VK_IMAGE_VIEW_TYPE_CUBE; break;
    default:                            result = VK_IMAGE_VIEW_TYPE_2D; break;
  }

  return result;
}

SF_INTERNAL VkImageType sf_graphics_vulkan_image_type_from_texture_type(sf_graphics_texture_type type) {
  VkImageType result = VK_IMAGE_TYPE_1D;

  switch (type) {
    case SF_GRAPHICS_TEXTURE_TYPE_1D:   result = VK_IMAGE_TYPE_1D; break;
    case SF_GRAPHICS_TEXTURE_TYPE_2D:   result = VK_IMAGE_TYPE_2D; break;
    case SF_GRAPHICS_TEXTURE_TYPE_3D:   result = VK_IMAGE_TYPE_3D; break;
    case SF_GRAPHICS_TEXTURE_TYPE_CUBE: result = VK_IMAGE_TYPE_2D; break;
    default:                            result = VK_IMAGE_TYPE_2D; break;
  }

  return result;
}

SF_INTERNAL VkFormat sf_graphics_vulkan_format_from_format(sf_graphics_format format) {
  VkFormat result = VK_FORMAT_UNDEFINED;

  switch (format) {
    // 1 channel
    case SF_GRAPHICS_FORMAT_R8_UNORM:            result = VK_FORMAT_R8_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16_UNORM:           result = VK_FORMAT_R16_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16_UINT:            result = VK_FORMAT_R16_UINT; break;
    case SF_GRAPHICS_FORMAT_R16_SFLOAT:          result = VK_FORMAT_R16_SFLOAT; break;
    case SF_GRAPHICS_FORMAT_R32_UINT:            result = VK_FORMAT_R32_UINT; break;
    case SF_GRAPHICS_FORMAT_R32_SFLOAT:          result = VK_FORMAT_R32_SFLOAT; break;
    // 2 channel
    case SF_GRAPHICS_FORMAT_R8G8_UNORM:          result = VK_FORMAT_R8G8_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16G16_UNORM:        result = VK_FORMAT_R16G16_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:       result = VK_FORMAT_R16G16_SFLOAT; break;
    case SF_GRAPHICS_FORMAT_R32G32_UINT:         result = VK_FORMAT_R32G32_UINT; break;
    case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:       result = VK_FORMAT_R32G32_SFLOAT; break;
    // 3 channel
    case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:        result = VK_FORMAT_R8G8B8_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:     result = VK_FORMAT_R16G16B16_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:    result = VK_FORMAT_R16G16B16_SFLOAT; break;
    case SF_GRAPHICS_FORMAT_R32G32B32_UINT:      result = VK_FORMAT_R32G32B32_UINT; break;
    case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:    result = VK_FORMAT_R32G32B32_SFLOAT; break;
    // 4 channel
    case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:      result = VK_FORMAT_B8G8R8A8_UNORM; break;
    case SF_GRAPHICS_FORMAT_B8G8R8A8_SRGB:       result = VK_FORMAT_B8G8R8A8_SRGB; break;
    case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:      result = VK_FORMAT_R8G8B8A8_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:  result = VK_FORMAT_R16G16B16A16_UNORM; break;
    case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: result = VK_FORMAT_R16G16B16A16_SFLOAT; break;
    case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:   result = VK_FORMAT_R32G32B32A32_UINT; break;
    case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: result = VK_FORMAT_R32G32B32A32_SFLOAT; break;
    // Depth/stencil
    case SF_GRAPHICS_FORMAT_D16_UNORM:           result = VK_FORMAT_D16_UNORM; break;
    case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: result = VK_FORMAT_X8_D24_UNORM_PACK32; break;
    case SF_GRAPHICS_FORMAT_D32_SFLOAT:          result = VK_FORMAT_D32_SFLOAT; break;
    case SF_GRAPHICS_FORMAT_S8_UINT:             result = VK_FORMAT_S8_UINT; break;
    case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:   result = VK_FORMAT_D16_UNORM_S8_UINT; break;
    case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:   result = VK_FORMAT_D24_UNORM_S8_UINT; break;
    case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:  result = VK_FORMAT_D32_SFLOAT_S8_UINT; break;
    default:                                     result = VK_FORMAT_UNDEFINED; break;
  }

  return result;
}

SF_INTERNAL sf_graphics_format sf_graphics_format_from_vulkan_format(VkFormat format) {
  sf_graphics_format result = SF_GRAPHICS_FORMAT_UNDEFINED;

  switch (format) {
    // 1 channel
    case VK_FORMAT_R8_UNORM:            result = SF_GRAPHICS_FORMAT_R8_UNORM; break;
    case VK_FORMAT_R16_UNORM:           result = SF_GRAPHICS_FORMAT_R16_UNORM; break;
    case VK_FORMAT_R16_UINT:            result = SF_GRAPHICS_FORMAT_R16_UINT; break;
    case VK_FORMAT_R16_SFLOAT:          result = SF_GRAPHICS_FORMAT_R16_SFLOAT; break;
    case VK_FORMAT_R32_UINT:            result = SF_GRAPHICS_FORMAT_R32_UINT; break;
    case VK_FORMAT_R32_SFLOAT:          result = SF_GRAPHICS_FORMAT_R32_SFLOAT; break;
    // 2 channel
    case VK_FORMAT_R8G8_UNORM:          result = SF_GRAPHICS_FORMAT_R8G8_UNORM; break;
    case VK_FORMAT_R16G16_UNORM:        result = SF_GRAPHICS_FORMAT_R16G16_UNORM; break;
    case VK_FORMAT_R16G16_SFLOAT:       result = SF_GRAPHICS_FORMAT_R16G16_SFLOAT; break;
    case VK_FORMAT_R32G32_UINT:         result = SF_GRAPHICS_FORMAT_R32G32_UINT; break;
    case VK_FORMAT_R32G32_SFLOAT:       result = SF_GRAPHICS_FORMAT_R32G32_SFLOAT; break;
    // 3 channel
    case VK_FORMAT_R8G8B8_UNORM:        result = SF_GRAPHICS_FORMAT_R8G8B8_UNORM; break;
    case VK_FORMAT_R16G16B16_UNORM:     result = SF_GRAPHICS_FORMAT_R16G16B16_UNORM; break;
    case VK_FORMAT_R16G16B16_SFLOAT:    result = SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT; break;
    case VK_FORMAT_R32G32B32_UINT:      result = SF_GRAPHICS_FORMAT_R32G32B32_UINT; break;
    case VK_FORMAT_R32G32B32_SFLOAT:    result = SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT; break;
    // 4 channel
    case VK_FORMAT_B8G8R8A8_UNORM:      result = SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM; break;
    case VK_FORMAT_B8G8R8A8_SRGB:       result = SF_GRAPHICS_FORMAT_B8G8R8A8_SRGB; break;
    case VK_FORMAT_R8G8B8A8_UNORM:      result = SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM; break;
    case VK_FORMAT_R16G16B16A16_UNORM:  result = SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM; break;
    case VK_FORMAT_R16G16B16A16_SFLOAT: result = SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT; break;
    case VK_FORMAT_R32G32B32A32_UINT:   result = SF_GRAPHICS_FORMAT_R32G32B32A32_UINT; break;
    case VK_FORMAT_R32G32B32A32_SFLOAT: result = SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT; break;
    // Depth/stencil
    case VK_FORMAT_D16_UNORM:           result = SF_GRAPHICS_FORMAT_D16_UNORM; break;
    case VK_FORMAT_X8_D24_UNORM_PACK32: result = SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32; break;
    case VK_FORMAT_D32_SFLOAT:          result = SF_GRAPHICS_FORMAT_D32_SFLOAT; break;
    case VK_FORMAT_S8_UINT:             result = SF_GRAPHICS_FORMAT_S8_UINT; break;
    case VK_FORMAT_D16_UNORM_S8_UINT:   result = SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT; break;
    case VK_FORMAT_D24_UNORM_S8_UINT:   result = SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT; break;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:  result = SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT; break;
    default:                            result = SF_GRAPHICS_FORMAT_UNDEFINED; break;
  }

  return result;
}

SF_INTERNAL VkImageAspectFlags sf_graphics_vulkna_image_aspect_flags_from_format(sf_graphics_format format) {
  VkImageAspectFlags result = VK_IMAGE_ASPECT_NONE;
  switch (format) {
    // 1 channel
    case SF_GRAPHICS_FORMAT_R8_UNORM:
    case SF_GRAPHICS_FORMAT_R16_UNORM:
    case SF_GRAPHICS_FORMAT_R16_UINT:
    case SF_GRAPHICS_FORMAT_R16_SFLOAT:
    case SF_GRAPHICS_FORMAT_R32_UINT:
    case SF_GRAPHICS_FORMAT_R32_SFLOAT:
    // 2 channel
    case SF_GRAPHICS_FORMAT_R8G8_UNORM:
    case SF_GRAPHICS_FORMAT_R16G16_UNORM:
    case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:
    case SF_GRAPHICS_FORMAT_R32G32_UINT:
    case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:
    // 3 channel
    case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:
    case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:
    case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:
    case SF_GRAPHICS_FORMAT_R32G32B32_UINT:
    case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:
    // 4 channel
    case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:
    case SF_GRAPHICS_FORMAT_B8G8R8A8_SRGB:
    case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:
    case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:
    case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT:
    case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:
    case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: result = VK_IMAGE_ASPECT_COLOR_BIT; break;
    // Depth/stencil
    case SF_GRAPHICS_FORMAT_D16_UNORM:
    case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32:
    case SF_GRAPHICS_FORMAT_D32_SFLOAT:          result = VK_IMAGE_ASPECT_DEPTH_BIT; break;
    case SF_GRAPHICS_FORMAT_S8_UINT:             result = VK_IMAGE_ASPECT_STENCIL_BIT; break;
    case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:
    case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:
    case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:  result = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; break;
    default:                                     result = 0; break;
  }

  return result;
}

SF_INTERNAL VkSampleCountFlags sf_graphics_vulkan_sample_count_from_sample_count(sf_graphics_sample_count samples) {
  VkSampleCountFlagBits result = VK_SAMPLE_COUNT_1_BIT;
  switch (samples) {
    case SF_GRAPHICS_SAMPLE_COUNT_1:  result = VK_SAMPLE_COUNT_1_BIT; break;
    case SF_GRAPHICS_SAMPLE_COUNT_2:  result = VK_SAMPLE_COUNT_2_BIT; break;
    case SF_GRAPHICS_SAMPLE_COUNT_4:  result = VK_SAMPLE_COUNT_4_BIT; break;
    case SF_GRAPHICS_SAMPLE_COUNT_8:  result = VK_SAMPLE_COUNT_8_BIT; break;
    case SF_GRAPHICS_SAMPLE_COUNT_16: result = VK_SAMPLE_COUNT_16_BIT; break;
    default:                          result = VK_SAMPLE_COUNT_1_BIT; break;
  }
  return result;
}

SF_INTERNAL sf_graphics_sample_count sf_graphics_sample_count_from_vulkan_sample_count(VkSampleCountFlags samples) {
  sf_graphics_sample_count result = SF_GRAPHICS_SAMPLE_COUNT_1;
  switch (samples) {
    case VK_SAMPLE_COUNT_1_BIT:  result = SF_GRAPHICS_SAMPLE_COUNT_1; break;
    case VK_SAMPLE_COUNT_2_BIT:  result = SF_GRAPHICS_SAMPLE_COUNT_2; break;
    case VK_SAMPLE_COUNT_4_BIT:  result = SF_GRAPHICS_SAMPLE_COUNT_4; break;
    case VK_SAMPLE_COUNT_8_BIT:  result = SF_GRAPHICS_SAMPLE_COUNT_8; break;
    case VK_SAMPLE_COUNT_16_BIT: result = SF_GRAPHICS_SAMPLE_COUNT_16; break;
    default:                     result = SF_GRAPHICS_SAMPLE_COUNT_1; break;
  }
  return result;
}

SF_INTERNAL VkImageUsageFlags sf_graphics_vulkan_image_usage_from_texture_usage(sf_graphics_texture_usage_flags usage) {
  VkImageUsageFlags result = 0;

  if (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC) {
    result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }
  if (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST) {
    result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  if (usage & SF_GRAPHICS_TEXTURE_USAGE_SAMPLED) {
    result |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  if (usage & SF_GRAPHICS_TEXTURE_USAGE_STORAGE) {
    result |= VK_IMAGE_USAGE_STORAGE_BIT;
  }
  if (usage & SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT) {
    result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (usage & SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT) {
    result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  if (usage & SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC) {
    result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (usage & SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST) {
    result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  return result;
}
SF_INTERNAL void sf_graphics_vulkan_create_image_view(sf_graphics_renderer *r, sf_graphics_texture *texture) {
  if (r && texture && r->vk_device && texture->vk_image) {
    VkImageViewCreateInfo info = {0};

    info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext                           = NULL;
    info.flags                           = 0;
    info.image                           = texture->vk_image;
    info.viewType                        = sf_graphics_vulkan_ikmage_view_type_from_texture_type(texture->type);
    info.format                          = sf_graphics_vulkan_format_from_format(texture->format);
    info.components.r                    = VK_COMPONENT_SWIZZLE_R;
    info.components.g                    = VK_COMPONENT_SWIZZLE_G;
    info.components.b                    = VK_COMPONENT_SWIZZLE_B;
    info.components.a                    = VK_COMPONENT_SWIZZLE_A;
    info.subresourceRange.aspectMask     = sf_graphics_vulkna_image_aspect_flags_from_format(texture->format);
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = texture->mips;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;

    if (!SF_VULKAN_CHECK(vkCreateImageView(r->vk_device, &info, r->vk_allocation_callbacks, &texture->vk_image_view)))
      texture->vk_image_view = VK_NULL_HANDLE;
  }
}

SF_INTERNAL void sf_graphics_vulkan_create_image(sf_graphics_renderer *r, sf_graphics_texture *texture) {
  if (r && texture && r->vk_device) {
    VkImageCreateInfo info = {0};

    info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext                 = NULL;
    info.flags                 = 0;
    info.imageType             = sf_graphics_vulkan_image_type_from_texture_type(texture->type);
    info.format                = sf_graphics_vulkan_format_from_format(texture->format);
    info.extent.width          = texture->width;
    info.extent.height         = texture->height;
    info.extent.depth          = texture->depth;
    info.mipLevels             = texture->mips;
    info.arrayLayers           = 1;
    info.samples               = sf_graphics_vulkan_sample_count_from_sample_count(texture->samples);
    info.tiling                = texture->mapped ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    info.usage                 = sf_graphics_vulkan_image_usage_from_texture_usage(texture->usage);
    info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices   = NULL;
    info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    if (!SF_VULKAN_CHECK(vkCreateImage(r->vk_device, &info, r->vk_allocation_callbacks, &texture->vk_image)))
      texture->vk_image = VK_NULL_HANDLE;
  }
}

SF_INTERNAL void sf_graphics_vulkan_allocate_texture_memory(sf_graphics_renderer *r, sf_graphics_texture *texture) {
  if (r && texture && r->vk_device && texture->vk_image) {
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (texture->mapped)
      properties |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    texture->vk_memory = sf_graphics_allocate_memory_for_image(r, texture->vk_image, properties);
    if (texture->vk_memory) {
      if (texture->mapped) {
        if (!SF_VULKAN_CHECK(vkMapMemory(r->vk_device, texture->vk_memory, 0, VK_WHOLE_SIZE, 0, &texture->mapped_data))) {
          vkFreeMemory(r->vk_device, texture->vk_memory, r->vk_allocation_callbacks);
          texture->vk_memory = VK_NULL_HANDLE;
        }
      }
    }
  }
}

SF_EXTERNAL void sf_graphics_destroy_texture(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_texture *texture = sf_graphics_texture_from_handle(r, handle);

  if (r && texture && r->vk_device) {
    texture->is_occupied = SF_FALSE;

    if (texture->vk_image_view) {
      vkDestroyImageView(r->vk_device, texture->vk_image_view, r->vk_allocation_callbacks);
      texture->vk_image_view = VK_NULL_HANDLE;
    }

    if (texture->vk_owns_image_and_memory) {
      if (texture->vk_memory) {
        vkFreeMemory(r->vk_device, texture->vk_memory, r->vk_allocation_callbacks);
        texture->vk_memory = VK_NULL_HANDLE;
      }

      if (texture->vk_image) {
        vkDestroyImage(r->vk_device, texture->vk_image, r->vk_allocation_callbacks);
        texture->vk_image = VK_NULL_HANDLE;
      }
    }
  }
}

SF_INTERNAL sf_handle sf_graphics_vulkan_create_texture(sf_graphics_renderer *r, sf_graphics_texture_type type, sf_graphics_format format,
                                                        sf_graphics_sample_count samples, sf_graphics_texture_usage usage, u32 width,
                                                        u32 height, u32 depth, u32 mips, sf_bool mapped,
                                                        sf_graphics_clear_value *clear_value, VkImage vk_not_owned_image) {
  sf_graphics_texture *texture = NULL;
  if (r) {
    texture = sf_graphics_get_texture_from_resource_pool(r);
    if (texture) {
      texture->type    = type;
      texture->format  = format;
      texture->samples = samples;
      texture->usage   = usage;
      texture->width   = width;
      texture->height  = height;
      texture->depth   = depth;
      texture->mips    = mips;
      texture->mapped  = mapped;
      if (clear_value)
        texture->clear_value = *clear_value;
      else
        texture->clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;

      if (!vk_not_owned_image) {
        texture->vk_owns_image_and_memory = SF_TRUE;
        sf_graphics_vulkan_create_image(r, texture);
        sf_graphics_vulkan_allocate_texture_memory(r, texture);
      } else {
        texture->vk_owns_image_and_memory = SF_FALSE;
        texture->vk_image                 = vk_not_owned_image;
        texture->mapped                   = SF_FALSE;
      }

      sf_graphics_vulkan_create_image_view(r, texture);
      if (!texture->vk_image || (texture->vk_owns_image_and_memory && !texture->vk_memory) || !texture->vk_image_view) {
        sf_graphics_destroy_texture(r, sf_graphics_handle_from_texture(r, texture));
        texture = NULL;
      }
    }
  }
  return sf_graphics_handle_from_texture(r, texture);
}

SF_EXTERNAL sf_handle sf_graphics_create_texture(sf_graphics_renderer *r, sf_graphics_texture_type type, sf_graphics_format format,
                                                 sf_graphics_sample_count samples, sf_graphics_texture_usage usage, u32 width, u32 height,
                                                 u32 depth, u32 mips, sf_bool mapped, sf_graphics_clear_value *clear_value) {
  return sf_graphics_vulkan_create_texture(
      r, type, format, samples, usage, width, height, depth, mips, mapped, clear_value, VK_NULL_HANDLE);
}

SF_INTERNAL void sf_graphics_vulkan_create_render_target_render_pass(sf_graphics_renderer *r, sf_graphics_render_target *render_target) {
  if (r && render_target && r->vk_device) {
    VkAttachmentReference depth_stencil_attachment_reference = {0};

    sf_arena *arena = &r->render_target_arena;

    u32 color_attachment_count   = render_target->color_attachment_count;
    u32 resolve_attachment_count = render_target->resolve_attachment_count;

    u32 description_count = color_attachment_count + resolve_attachment_count +
                            !sf_graphics_is_null_handle(render_target->depth_stencil_attachment);

    VkAttachmentDescription *descriptions     = sf_arena_allocate(arena, description_count * sizeof(*descriptions));
    VkAttachmentReference   *color_references = sf_arena_allocate(arena, render_target->color_attachment_count * sizeof(*color_references));
    VkAttachmentReference   *resolve_references = sf_arena_allocate(
        arena, render_target->resolve_attachment_count * sizeof(*resolve_references));

    if (descriptions && (!color_attachment_count || color_references) && (!resolve_attachment_count || resolve_references)) {
      for (u32 i = 0; i < render_target->color_attachment_count; ++i) {
        u32                      desc_index = i * (!!resolve_attachment_count) * 2;
        VkAttachmentDescription *desc       = &descriptions[desc_index];
        VkAttachmentReference   *reference  = &color_references[i];

        sf_graphics_texture *attachment = sf_graphics_texture_from_handle(r, render_target->color_attachments[i]);

        desc->flags          = 0;
        desc->format         = sf_graphics_vulkan_format_from_format(attachment->format);
        desc->samples        = sf_graphics_vulkan_sample_count_from_sample_count(r->swapchain_sample_count);
        desc->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        desc->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        if (attachment->vk_owns_image_and_memory)
          desc->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        else
          desc->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        reference->attachment = desc_index;
        reference->layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }

      for (u32 i = 0; i < render_target->resolve_attachment_count; ++i) {
        u32                      desc_index = i * 2 + 1;
        VkAttachmentDescription *desc       = &descriptions[desc_index];
        VkAttachmentReference   *reference  = &resolve_references[i];

        sf_graphics_texture *attachment = sf_graphics_texture_from_handle(r, render_target->resolve_attachments[i]);

        desc->flags          = 0;
        desc->format         = sf_graphics_vulkan_format_from_format(attachment->format);
        desc->samples        = VK_SAMPLE_COUNT_1_BIT;
        desc->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        desc->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        desc->finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        reference->attachment = desc_index;
        reference->layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }

      if (!sf_graphics_is_null_handle(render_target->depth_stencil_attachment)) {
        u32                      desc_index = description_count - 1;
        VkAttachmentDescription *desc       = &descriptions[desc_index];
        VkAttachmentReference   *ref        = &depth_stencil_attachment_reference;

        sf_graphics_texture *attachment = sf_graphics_texture_from_handle(r, render_target->depth_stencil_attachment);

        desc->flags          = 0;
        desc->format         = sf_graphics_vulkan_format_from_format(attachment->format);
        desc->samples        = sf_graphics_vulkan_sample_count_from_sample_count(attachment->samples);
        desc->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        desc->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        desc->finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        ref->attachment = desc_index;
        ref->layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      }

      VkSubpassDescription subpass = {0};
      subpass.flags                = 0;
      subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.inputAttachmentCount = 0;
      subpass.pInputAttachments    = NULL;
      subpass.colorAttachmentCount = render_target->color_attachment_count;
      subpass.pColorAttachments    = color_references;
      subpass.pResolveAttachments  = resolve_references;
      if (!sf_graphics_is_null_handle(render_target->depth_stencil_attachment))
        subpass.pDepthStencilAttachment = &depth_stencil_attachment_reference;
      else
        subpass.pDepthStencilAttachment = NULL;
      subpass.preserveAttachmentCount = 0;
      subpass.pPreserveAttachments    = NULL;

      VkSubpassDependency dependency = {0};
      dependency.srcSubpass          = 0;
      dependency.dstSubpass          = 0;
      dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependency.dependencyFlags     = VK_DEPENDENCY_BY_REGION_BIT;

      VkRenderPassCreateInfo info = {0};
      info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      info.pNext                  = NULL;
      info.flags                  = 0;
      info.attachmentCount        = description_count;
      info.pAttachments           = descriptions;
      info.subpassCount           = 1;
      info.pSubpasses             = &subpass;
      info.dependencyCount        = 1;
      info.pDependencies          = &dependency;

      if (!SF_VULKAN_CHECK(vkCreateRenderPass(r->vk_device, &info, r->vk_allocation_callbacks, &render_target->vk_render_pass)))
        render_target->vk_render_pass = VK_NULL_HANDLE;
    }

    sf_arena_clear(arena);
  }
}

SF_INTERNAL void sf_graphics_vulkan_create_render_target_framebuffer(sf_graphics_renderer *r, sf_graphics_render_target *render_target) {
  if (r && render_target && r->vk_device && render_target->vk_render_pass) {
    VkImageView attachments[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT] = {0};

    for (u32 i = 0; i < render_target->color_attachment_count; ++i) {
      u32                  attachment_index = i * (!!render_target->resolve_attachment_count) * 2;
      sf_graphics_texture *attachment       = sf_graphics_texture_from_handle(r, render_target->color_attachments[i]);
      attachments[attachment_index]         = attachment->vk_image_view;
    }

    for (u32 i = 0; i < render_target->resolve_attachment_count; ++i) {
      u32                  attachment_index = i * 2 + 1;
      sf_graphics_texture *attachment       = sf_graphics_texture_from_handle(r, render_target->resolve_attachments[i]);
      attachments[attachment_index]         = attachment->vk_image_view;
    }

    if (!sf_graphics_is_null_handle(render_target->depth_stencil_attachment)) {
      u32                  attachment_index = render_target->total_attachment_count - 1;
      sf_graphics_texture *attachment       = sf_graphics_texture_from_handle(r, render_target->depth_stencil_attachment);
      attachments[attachment_index]         = attachment->vk_image_view;
    }

    VkFramebufferCreateInfo info = {0};
    info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext                   = NULL;
    info.flags                   = 0;
    info.renderPass              = render_target->vk_render_pass;
    info.attachmentCount         = render_target->total_attachment_count;
    info.pAttachments            = attachments;
    info.width                   = render_target->width;
    info.height                  = render_target->height;
    info.layers                  = 1;

    if (!SF_VULKAN_CHECK(vkCreateFramebuffer(r->vk_device, &info, r->vk_allocation_callbacks, &render_target->vk_framebuffer)))
      render_target->vk_framebuffer = VK_NULL_HANDLE;
  }
}

SF_EXTERNAL void sf_graphics_destroy_render_target(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_render_target *renderTarget = sf_graphics_render_target_from_handle(r, handle);

  if (r && renderTarget) {
    renderTarget->is_occupied = SF_FALSE;

    if (r->vk_device) {
      if (renderTarget->vk_framebuffer) {
        vkDestroyFramebuffer(r->vk_device, renderTarget->vk_framebuffer, r->vk_allocation_callbacks);
        renderTarget->vk_framebuffer = VK_NULL_HANDLE;
      }

      if (renderTarget->vk_render_pass) {
        vkDestroyRenderPass(r->vk_device, renderTarget->vk_render_pass, r->vk_allocation_callbacks);
        renderTarget->vk_render_pass = VK_NULL_HANDLE;
      }
    }

    for (u32 i = 0; i < renderTarget->color_attachment_count; ++i) {
      sf_graphics_destroy_texture(r, renderTarget->color_attachments[i]);
      renderTarget->color_attachments[i] = SF_NULL_HANDLE;
    }

    renderTarget->color_attachment_count = 0;

    for (u32 i = 0; i < renderTarget->resolve_attachment_count; ++i) {
      sf_graphics_destroy_texture(r, renderTarget->resolve_attachments[i]);
      renderTarget->resolve_attachments[i] = SF_NULL_HANDLE;
    }
    renderTarget->resolve_attachment_count = 0;

    sf_graphics_destroy_texture(r, renderTarget->depth_stencil_attachment);
    renderTarget->depth_stencil_attachment = SF_NULL_HANDLE;
  }
}

SF_INTERNAL sf_handle sf_graphics_vulkan_create_render_target(sf_graphics_renderer *r, u32 width, u32 height,
                                                              sf_graphics_sample_count samples, sf_graphics_format color_format,
                                                              sf_graphics_format depth_stencil_format, u32 color_attachment_count,
                                                              sf_graphics_clear_value *color_clear_values,
                                                              sf_graphics_clear_value *depth_stencil_clear_value,
                                                              VkImage                  vk_swapchain_image) {
  sf_graphics_render_target *result = NULL;
  if (r && r->vk_device) {
    result = sf_graphics_get_render_target_from_resource_pool(r);
    if (result) {
      sf_bool created_all_attachments = SF_TRUE;

      result->samples                  = samples;
      result->width                    = width;
      result->height                   = height;
      result->resolve_attachment_count = SF_GRAPHICS_SAMPLE_COUNT_1 != samples ? color_attachment_count : 0;
      result->color_attachment_count   = color_attachment_count;
      result->total_attachment_count   = color_attachment_count + result->resolve_attachment_count +
                                       !!(depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED);

      for (u32 i = 0; i < result->color_attachment_count && created_all_attachments; ++i) {
        sf_graphics_clear_value *clear_value = color_clear_values ? &color_clear_values[i] : NULL;

        sf_handle attachment = sf_graphics_create_texture(r, SF_GRAPHICS_TEXTURE_TYPE_2D, color_format, samples,
                                                          SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT, width, height, 1, 1, SF_FALSE,
                                                          clear_value);

        if (!sf_graphics_is_null_handle(attachment)) {
          result->color_attachments[i] = attachment;
        } else {
          created_all_attachments = SF_FALSE;
        }
      }

      for (u32 i = 0; i < result->resolve_attachment_count && created_all_attachments; ++i) {
        sf_graphics_clear_value *clear_value = color_clear_values ? &color_clear_values[i] : NULL;

        sf_handle attachment = sf_graphics_vulkan_create_texture(r, SF_GRAPHICS_TEXTURE_TYPE_2D, color_format, SF_GRAPHICS_SAMPLE_COUNT_1,
                                                                 SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT, width, height, 1, 1, SF_FALSE,
                                                                 clear_value, vk_swapchain_image);
        if (!sf_graphics_is_null_handle(attachment)) {
          result->resolve_attachments[i] = attachment;
        } else {
          created_all_attachments = SF_FALSE;
        }
      }

      if (depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED) {
        sf_handle attachment = sf_graphics_create_texture(r, SF_GRAPHICS_TEXTURE_TYPE_2D, depth_stencil_format, samples,
                                                          SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT, width, height, 1, 1, SF_FALSE,
                                                          depth_stencil_clear_value);
        if (!sf_graphics_is_null_handle(attachment)) {
          result->depth_stencil_attachment = attachment;
        } else {
          created_all_attachments = SF_FALSE;
        }
      }

      if (created_all_attachments) {
        sf_graphics_vulkan_create_render_target_render_pass(r, result);
        sf_graphics_vulkan_create_render_target_framebuffer(r, result);
      }

      if (!created_all_attachments || !result->vk_render_pass || !result->vk_framebuffer) {
        sf_graphics_destroy_render_target(r, sf_graphics_handle_from_render_target(r, result));
        result = NULL;
      }
    }
  }

  return sf_graphics_handle_from_render_target(r, result);
}

SF_EXTERNAL sf_handle sf_graphics_create_render_target(sf_graphics_renderer *r, u32 width, u32 height, sf_graphics_sample_count samples,
                                                       sf_graphics_format colorFormat, sf_graphics_format depthStencilFormat,
                                                       u32 colorAttachmentCount, sf_graphics_clear_value *colorClearValues,
                                                       sf_graphics_clear_value *depthStencilClearValue) {
  return sf_graphics_vulkan_create_render_target(r, width, height, samples, colorFormat, depthStencilFormat, colorAttachmentCount,
                                                 colorClearValues, depthStencilClearValue, VK_NULL_HANDLE);
}

SF_INTERNAL void sf_graphics_vulkan_destroy_swapchain_resources(sf_graphics_renderer *r) {
  if (r && r->vk_device) {
    for (u32 i = 0; i < r->swapchain_render_target_count; ++i) {
      sf_graphics_destroy_render_target(r, r->swapchain_render_targets[i]);
      r->swapchain_render_targets[i] = SF_NULL_HANDLE;
    }
    r->swapchain_render_target_count = 0;

    for (u32 i = 0; i < SF_SIZE(r->vk_swapchain_images); ++i)
      r->vk_swapchain_images[i] = VK_NULL_HANDLE;

    r->vk_swapchain_image_count = 0;

    if (r->vk_swapchain) {
      vkDestroySwapchainKHR(r->vk_device, r->vk_swapchain, r->vk_allocation_callbacks);
      r->vk_swapchain = VK_NULL_HANDLE;
    }
  }
}

SF_INTERNAL void sf_graphics_request_swapchain_dimensions(sf_graphics_renderer *r) {
  if (r && r->request_swapchain_dimensions) {
    r->request_swapchain_dimensions(r->platform_data, r);
  }
}

SF_INTERNAL void sf_graphics_vulkan_create_surface(sf_graphics_renderer *r) {
  if (r && r->create_vulkan_surface) {
    r->create_vulkan_surface(r->platform_data, r);
  }
}

SF_INTERNAL void sf_graphics_vulkan_create_swapchain_resources(sf_graphics_renderer *r) {
  if (r && r->vk_device) {
    sf_graphics_request_swapchain_dimensions(r);

    VkSurfaceCapabilitiesKHR capabilities = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->vk_physical_device, r->vk_surface, &capabilities);

    u32 queue_family_indices[] = {r->vk_graphics_queue_family_index, r->vk_present_queue_family_index};

    VkSwapchainCreateInfoKHR info = {0};

    info.sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.pNext              = NULL;
    info.flags              = 0;
    info.surface            = r->vk_surface;
    info.minImageCount      = r->swapchain_requested_image_count;
    info.imageFormat        = sf_graphics_vulkan_format_from_format(r->swapchain_color_format);
    info.imageColorSpace    = r->vk_surface_format.colorSpace;
    info.imageExtent.width  = r->swapchain_width;
    info.imageExtent.height = r->swapchain_height;
    info.imageArrayLayers   = 1;
    info.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (r->vk_graphics_queue_family_index == r->vk_present_queue_family_index) {
      info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
      info.queueFamilyIndexCount = 1;
      info.pQueueFamilyIndices   = queue_family_indices;
    } else {
      info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
      info.queueFamilyIndexCount = SF_SIZE(queue_family_indices);
      info.pQueueFamilyIndices   = queue_family_indices;
    }
    info.preTransform   = capabilities.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode    = r->vk_present_mode;
    info.clipped        = VK_TRUE;
    info.oldSwapchain   = VK_NULL_HANDLE;

    if (SF_VULKAN_CHECK(vkCreateSwapchainKHR(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_swapchain))) {
      u32 image_count = 0;

      if (SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &image_count, NULL))) {
        if (image_count <= SF_SIZE(r->vk_swapchain_images)) {
          if (SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &image_count, r->vk_swapchain_images)))
            r->vk_swapchain_image_count = image_count;
        }
      }
    }

    sf_bool created_all_render_targets = SF_TRUE;
    r->swapchain_render_target_count   = r->vk_swapchain_image_count;
    for (u32 i = 0; i < r->swapchain_render_target_count && created_all_render_targets; ++i) {
      r->swapchain_render_targets[i] = sf_graphics_vulkan_create_render_target(
          r, r->swapchain_width, r->swapchain_height, r->swapchain_sample_count, r->swapchain_color_format,
          r->swapchain_depth_stencil_format, 1, &r->swapchain_color_clear_value, &r->swapchain_depth_stencil_clear_value,
          r->vk_swapchain_images[i]);

      if (sf_graphics_is_null_handle(r->swapchain_render_targets[i]))
        created_all_render_targets = SF_FALSE;
    }

    if (!r->vk_swapchain_image_count || !created_all_render_targets) {
      sf_graphics_vulkan_destroy_swapchain_resources(r);
    }
  }
}

SF_EXTERNAL void sf_graphics_destroy_command_buffer(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, handle);

  if (r && command_buffer) {
    command_buffer->is_occupied = SF_FALSE;

    if (r->vk_device) {
      if (command_buffer->vk_command_pool) {
        vkFreeCommandBuffers(r->vk_device, command_buffer->vk_command_pool, 1, &command_buffer->vk_command_buffer);
        command_buffer->vk_command_buffer = VK_NULL_HANDLE;

        vkDestroyCommandPool(r->vk_device, command_buffer->vk_command_pool, r->vk_allocation_callbacks);
        command_buffer->vk_command_pool = VK_NULL_HANDLE;
      }
    }
  }
}

SF_EXTERNAL sf_handle sf_graphics_create_command_buffer(sf_graphics_renderer *r, sf_bool transient) {
  sf_graphics_command_buffer *command_buffer = NULL;
  if (r && r->vk_device) {
    command_buffer = sf_graphics_get_command_buffer_from_resource_pool(r);
    if (command_buffer) {
      VkCommandPoolCreateInfo command_pool_info = {0};
      command_pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      command_pool_info.pNext                   = NULL;
      command_pool_info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      if (transient)
        command_pool_info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
      // FIXME(samuel): for now manually set the graphics queue as the family
      // index. Later we should pass the requried queue handle
      command_pool_info.queueFamilyIndex = r->vk_graphics_queue_family_index;
      if (SF_VULKAN_CHECK(
              vkCreateCommandPool(r->vk_device, &command_pool_info, r->vk_allocation_callbacks, &command_buffer->vk_command_pool))) {
        VkCommandBufferAllocateInfo command_buffer_info = {0};

        command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_info.pNext              = NULL;
        command_buffer_info.commandPool        = command_buffer->vk_command_pool;
        command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_info.commandBufferCount = 1;

        if (!SF_VULKAN_CHECK(vkAllocateCommandBuffers(r->vk_device, &command_buffer_info, &command_buffer->vk_command_buffer)))
          command_buffer->vk_command_buffer = VK_NULL_HANDLE;
      } else {
        command_buffer->vk_command_pool = VK_NULL_HANDLE;
      }

      if (!command_buffer->vk_command_pool || !command_buffer->vk_command_buffer) {
        sf_graphics_destroy_command_buffer(r, sf_graphics_handle_from_command_buffer(r, command_buffer));
        command_buffer = NULL;
      }
    }
  }

  return sf_graphics_handle_from_command_buffer(r, command_buffer);
}

SF_INTERNAL VkShaderModule sf_graphics_vulkan_create_shader(sf_graphics_renderer *r, u32 code_size, void const *code) {
  VkShaderModule shader = VK_NULL_HANDLE;

  if (r && r->vk_device) {
    VkShaderModuleCreateInfo info = {0};

    info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext    = NULL;
    info.flags    = 0;
    info.codeSize = code_size;
    info.pCode    = (u32 const *)code;

    if (!SF_VULKAN_CHECK(vkCreateShaderModule(r->vk_device, &info, r->vk_allocation_callbacks, &shader)))
      shader = VK_NULL_HANDLE;
  }

  return shader;
}

SF_EXTERNAL void sf_graphics_destroy_pipeline(sf_graphics_renderer *r, sf_handle handle) {
  sf_graphics_pipeline *pipeline = sf_graphics_pipeline_from_handle(r, handle);

  if (r && pipeline) {
    pipeline->is_occupied = SF_FALSE;

    if (r->vk_device) {
      if (pipeline->vk_pipeline) {
        vkDestroyPipeline(r->vk_device, pipeline->vk_pipeline, r->vk_allocation_callbacks);
        pipeline->vk_pipeline = VK_NULL_HANDLE;
      }

      if (pipeline->vk_pipeline_layout) {
        vkDestroyPipelineLayout(r->vk_device, pipeline->vk_pipeline_layout, r->vk_allocation_callbacks);
        pipeline->vk_pipeline_layout = VK_NULL_HANDLE;
      }

      if (pipeline->vk_vertex_shader) {
        vkDestroyShaderModule(r->vk_device, pipeline->vk_vertex_shader, r->vk_allocation_callbacks);
        pipeline->vk_vertex_shader = VK_NULL_HANDLE;
      }

      if (pipeline->vk_fragment_shader) {
        vkDestroyShaderModule(r->vk_device, pipeline->vk_fragment_shader, r->vk_allocation_callbacks);
        pipeline->vk_fragment_shader = VK_NULL_HANDLE;
      }
    }
  }
}

SF_INTERNAL u64 sf_graphics_stride_from_format(sf_graphics_format format) {
  switch (format) {
    // 1 channel
    case SF_GRAPHICS_FORMAT_R8_UNORM:            return 1;
    case SF_GRAPHICS_FORMAT_R16_UNORM:           return 2;
    case SF_GRAPHICS_FORMAT_R16_UINT:            return 2;
    case SF_GRAPHICS_FORMAT_R16_SFLOAT:          return 2;
    case SF_GRAPHICS_FORMAT_R32_UINT:            return 4;
    case SF_GRAPHICS_FORMAT_R32_SFLOAT:          return 4;
    // 2 CHANNEL
    case SF_GRAPHICS_FORMAT_R8G8_UNORM:          return 2;
    case SF_GRAPHICS_FORMAT_R16G16_UNORM:        return 4;
    case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:       return 4;
    case SF_GRAPHICS_FORMAT_R32G32_UINT:         return 8;
    case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:       return 8;
    // 3 CHANNEL
    case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:        return 3;
    case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:     return 6;
    case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:    return 6;
    case SF_GRAPHICS_FORMAT_R32G32B32_UINT:      return 12;
    case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:    return 12;
    // 4 CHANNEL
    case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:      return 4;
    case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:      return 4;
    case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:  return 8;
    case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: return 8;
    case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:   return 16;
    case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return 16;
    // DEPTH/STENCIL
    case SF_GRAPHICS_FORMAT_D16_UNORM:           return 0;
    case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: return 0;
    case SF_GRAPHICS_FORMAT_D32_SFLOAT:          return 0;
    case SF_GRAPHICS_FORMAT_S8_UINT:             return 0;
    case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:   return 0;
    case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:   return 0;
    case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:  return 0;
    default:                                     return 0;
  }
}

SF_EXTERNAL sf_handle sf_graphics_create_pipeline(sf_graphics_renderer *r, sf_graphics_vertex_layout *vertex_layout,
                                                  sf_graphics_descriptor_set_layout *descriptor_set_layout, u32 vertex_code_size,
                                                  void const *vertex_code, u32 fragment_code_size, void const *fragment_code) {
  sf_graphics_pipeline *pipeline = NULL;

  if (r) {
    pipeline = sf_graphics_get_pipeline_from_resource_pool(r);

    if (pipeline) {
      pipeline->vk_vertex_shader   = sf_graphics_vulkan_create_shader(r, vertex_code_size, vertex_code);
      pipeline->vk_fragment_shader = sf_graphics_vulkan_create_shader(r, fragment_code_size, fragment_code);

      if (pipeline->vk_vertex_shader && pipeline->vk_fragment_shader) {}

      if (!pipeline->vk_pipeline || !pipeline->vk_vertex_shader || !pipeline->vk_fragment_shader) {
        sf_graphics_destroy_pipeline(r, sf_graphics_handle_from_pipeline(r, pipeline));
        pipeline = NULL;
      }
    }
  }

  return sf_graphics_handle_from_pipeline(r, pipeline);
}

#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)

typedef struct sf_graphics_device_list {
  u32               size;
  VkPhysicalDevice *data;
} sf_graphics_device_list;

SF_INTERNAL sf_graphics_device_list sf_graphics_create_device_list(sf_arena *arena, sf_graphics_renderer *r) {
  sf_graphics_device_list result = {0};

  if (SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(r->vk_instance, &result.size, NULL))) {
    result.data = sf_arena_allocate(arena, result.size * sizeof(result.data));
    if (result.data) {
      vkEnumeratePhysicalDevices(r->vk_instance, &result.size, result.data);
    }
  }

  return result;
}

SF_EXTERNAL sf_graphics_renderer *sf_graphics_create_renderer(sf_arena *arena, sf_graphics_renderer_description *description) {
  sf_graphics_renderer *r = sf_arena_allocate(arena, sizeof(sf_graphics_renderer));
  if (r) {
    r->platform_data                = description->data;
    r->create_vulkan_surface        = description->create_vulkan_surface;
    r->request_swapchain_dimensions = description->request_swapchain_dimensions;

    r->arena               = sf_arena_scratch(arena, 1024 * 256);
    r->render_target_arena = sf_arena_scratch(&r->arena, 1024 * 128);

    if (r->arena.data) {
      sf_string         app_name  = sf_string_null_terminate(&r->arena, description->application_name);
      VkApplicationInfo app_info  = {0};
      app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pNext              = NULL;
      app_info.pApplicationName   = app_name.data;
      app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      app_info.pEngineName        = NULL;
      app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
      app_info.apiVersion         = VK_MAKE_VERSION(1, 0, 0);

      VkInstanceCreateInfo info = {0};
      info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      info.pNext                = NULL;
      info.flags                = 0;
#ifdef __APPLE__
      info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
      info.pApplicationInfo        = &app_info;
      info.enabledLayerCount       = description->vk_instance_layer_count;
      info.ppEnabledLayerNames     = description->vk_instance_layers;
      info.enabledExtensionCount   = description->vk_instance_extension_count;
      info.ppEnabledExtensionNames = description->vk_instance_extensions;

      if (SF_VULKAN_CHECK(vkCreateInstance(&info, r->vk_allocation_callbacks, &r->vk_instance))) {
        r->vk_craete_debug_utils_messenger_ext  = SF_VULKAN_PROC(vkCreateDebugUtilsMessengerEXT, r->vk_instance);
        r->vk_destroy_debug_utils_messenger_ext = SF_VULKAN_PROC(vkDestroyDebugUtilsMessengerEXT, r->vk_instance);

        if (r->vk_craete_debug_utils_messenger_ext && r->vk_destroy_debug_utils_messenger_ext) {
          PFN_vkCreateDebugUtilsMessengerEXT create = r->vk_craete_debug_utils_messenger_ext;
          VkDebugUtilsMessengerCreateInfoEXT info   = {0};
          info.sType                                = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
          info.pNext                                = NULL;
          info.flags                                = 0;
          info.messageSeverity                      = 0;
          info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
          info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
          info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
          info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
          info.messageType = 0;
          info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
          info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
          info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
          info.pfnUserCallback = sf_graphics_vulkan_log;
          info.pUserData       = NULL;

          if (!SF_VULKAN_CHECK(create(r->vk_instance, &info, r->vk_allocation_callbacks, &r->vk_validation_messenger))) {
            r->vk_validation_messenger = VK_NULL_HANDLE;
          }
        }
      } else {
        r->vk_instance = VK_NULL_HANDLE;
      }

      if (r->vk_instance) {
        sf_graphics_vulkan_create_surface(r);

        if (r->vk_surface) {
          sf_graphics_device_list device_list = sf_graphics_create_device_list(&r->arena, r);

          if (device_list.data && device_list.size) {
            for (u32 i = 0; i < device_list.size && !r->vk_physical_device; ++i) {
              r->vk_physical_device = device_list.data[i];

              sf_graphics_find_suitable_queue_family_indices(&r->arena, r);
              if (!sf_graphics_are_queue_family_indices_valid(r)) {
                r->vk_physical_device = VK_NULL_HANDLE;
              } else if (!sf_graphics_check_device_swapchain_support(r)) {
                r->vk_physical_device = VK_NULL_HANDLE;
                continue;
              } else if (!sf_graphics_check_device_extension_support(&r->arena, r, description)) {
                r->vk_physical_device = VK_NULL_HANDLE;
              }
            }
          }
        }
      }

      if (r->vk_physical_device && r->vk_surface) {
        VkSurfaceFormatKHR requested_format = {.format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        sf_graphics_surface_format_list surface_format_list = sf_graphics_create_surface_format_list(
            &r->arena, r->vk_physical_device, r->vk_surface);

        for (u32 i = 0; i < surface_format_list.size; ++i) {
          VkSurfaceFormatKHR format = surface_format_list.data[i];

          if (requested_format.format == format.format || requested_format.colorSpace == format.colorSpace) {
            r->vk_surface_format = requested_format;
          }
        }

        if (r->vk_surface_format.format == VK_FORMAT_UNDEFINED)
          r->vk_physical_device = VK_NULL_HANDLE; // No format was found, invalidate the device;
      }

      if (r->vk_physical_device && r->vk_surface) {
        r->swapchain_requested_image_count = SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT;
        r->swapchain_color_format          = sf_graphics_format_from_vulkan_format(r->vk_surface_format.format);

        VkPresentModeKHR requested_present_mode = description->enable_vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
        sf_graphics_present_mode_list present_mode_list = sf_graphics_create_present_mode_list(
            &r->arena, r->vk_physical_device, r->vk_surface);

        r->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;

        for (u32 i = 0; i < present_mode_list.size; ++i) {
          VkPresentModeKHR present_mode = present_mode_list.data[i];

          if (requested_present_mode == present_mode) {
            r->vk_present_mode = requested_present_mode;
            break;
          }
        }

        if (r->vk_present_mode == VK_PRESENT_MODE_FIFO_KHR)
          r->enable_vsync = SF_TRUE;
        else
          r->enable_vsync = SF_FALSE;
      }
      if (r->vk_physical_device && r->vk_surface) {
        if (sf_graphics_test_format_features(
                r->vk_physical_device, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
          r->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT;
        else if (sf_graphics_test_format_features(r->vk_physical_device, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
          r->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        else if (sf_graphics_test_format_features(r->vk_physical_device, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
          r->vk_depth_stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;

        r->swapchain_depth_stencil_format = sf_graphics_format_from_vulkan_format(r->vk_depth_stencil_format);
      }
      if (r->vk_physical_device && r->vk_surface) {
        VkPhysicalDeviceProperties properties    = {0};
        VkSampleCountFlags         sample_counts = 0;

        vkGetPhysicalDeviceProperties(r->vk_physical_device, &properties);
        sample_counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

        if (0)
          (void)0;
        else if (sample_counts & VK_SAMPLE_COUNT_2_BIT)
          r->vk_samples = VK_SAMPLE_COUNT_2_BIT;

        r->swapchain_sample_count = sf_graphics_sample_count_from_vulkan_sample_count(r->vk_samples);
      }

      if (r->vk_physical_device) {
        VkPhysicalDeviceFeatures features = {0};
        vkGetPhysicalDeviceFeatures(r->vk_physical_device, &features);

        float                   priority      = 1.0F;
        VkDeviceQueueCreateInfo queue_info[2] = {0};

        queue_info[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].pNext            = NULL;
        queue_info[0].flags            = 0;
        queue_info[0].queueFamilyIndex = r->vk_graphics_queue_family_index;
        queue_info[0].queueCount       = 1;
        queue_info[0].pQueuePriorities = &priority;

        queue_info[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[1].pNext            = NULL;
        queue_info[1].flags            = 0;
        queue_info[1].queueFamilyIndex = r->vk_present_queue_family_index;
        queue_info[1].queueCount       = 1;
        queue_info[1].pQueuePriorities = &priority;

        VkDeviceCreateInfo info = {0};
        info.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pNext              = NULL;
        info.flags              = 0;
        if (r->vk_graphics_queue_family_index == r->vk_present_queue_family_index)
          info.queueCreateInfoCount = 1;
        else
          info.queueCreateInfoCount = SF_SIZE(queue_info);
        info.pQueueCreateInfos       = queue_info;
        info.enabledLayerCount       = 0;
        info.ppEnabledLayerNames     = NULL;
        info.enabledExtensionCount   = description->vk_device_extension_count;
        info.ppEnabledExtensionNames = description->vk_device_extensions;
        info.pEnabledFeatures        = &features;

        if (SF_VULKAN_CHECK(vkCreateDevice(r->vk_physical_device, &info, r->vk_allocation_callbacks, &r->vk_device))) {
          vkGetDeviceQueue(r->vk_device, r->vk_graphics_queue_family_index, 0, &r->vk_graphics_queue);
          vkGetDeviceQueue(r->vk_device, r->vk_present_queue_family_index, 0, &r->vk_present_queue);
        } else {
          r->vk_device = VK_NULL_HANDLE;
        }
      }

      if (r->vk_device) {
        sf_bool created_all_sync_objects = SF_TRUE;

        r->main_command_buffer_count = SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT;
        for (u32 i = 0; i < r->main_command_buffer_count && created_all_sync_objects; ++i) {
          r->main_command_buffers[i] = sf_graphics_create_command_buffer(r, SF_FALSE);
          if (sf_graphics_is_null_handle(r->main_command_buffers[i]))
            created_all_sync_objects = SF_FALSE;
        }

        r->vk_image_acquired_semaphore_count = r->main_command_buffer_count;
        for (u32 i = 0; i < r->vk_image_acquired_semaphore_count && created_all_sync_objects; ++i) {
          VkSemaphoreCreateInfo info = {0};

          info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
          info.pNext = NULL;
          info.flags = 0;

          if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_image_acquired_semaphores[i]))) {
            created_all_sync_objects           = SF_FALSE;
            r->vk_image_acquired_semaphores[i] = VK_NULL_HANDLE;
          }
        }

        r->vk_in_flight_fence_count = r->main_command_buffer_count;
        for (u32 i = 0; i < r->vk_in_flight_fence_count && created_all_sync_objects; ++i) {
          VkFenceCreateInfo info = {0};

          info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
          info.pNext = NULL;
          info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

          if (!SF_VULKAN_CHECK(vkCreateFence(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_in_flight_fences[i]))) {
            created_all_sync_objects  = SF_FALSE;
            r->vk_in_flight_fences[i] = VK_NULL_HANDLE;
          }
        }

        r->vk_draw_complete_semaphore_count = r->vk_swapchain_image_count;
        for (u32 i = 0; i < r->vk_draw_complete_semaphore_count && created_all_sync_objects; ++i) {
          VkSemaphoreCreateInfo info = {0};

          info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
          info.pNext = NULL;
          info.flags = 0;

          if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_draw_complete_semaphores[i]))) {
            created_all_sync_objects          = SF_FALSE;
            r->vk_draw_complete_semaphores[i] = VK_NULL_HANDLE;
          }
        }

        if (created_all_sync_objects) {
          sf_graphics_vulkan_create_swapchain_resources(r);
        }
      }

      if (!r->vk_device || !r->vk_swapchain) {
        sf_graphics_destroy_renderer(r);
        r = NULL;
      }
    }
  }

  return r;
}

SF_EXTERNAL void sf_graphics_destroy_renderer(sf_graphics_renderer *r) {
  if (r) {

    if (r->vk_device) {
      vkDeviceWaitIdle(r->vk_device);

      sf_graphics_vulkan_destroy_swapchain_resources(r);

      for (u32 i = 0; i < SF_SIZE(r->vk_draw_complete_semaphores); ++i) {
        VkSemaphore semaphore = r->vk_draw_complete_semaphores[i];

        if (semaphore) {
          vkDestroySemaphore(r->vk_device, semaphore, r->vk_allocation_callbacks);
          r->vk_draw_complete_semaphores[i] = VK_NULL_HANDLE;
        }
      }
      r->vk_draw_complete_semaphore_count = 0;

      for (u32 i = 0; i < SF_SIZE(r->vk_in_flight_fences); ++i) {
        VkFence fence = r->vk_in_flight_fences[i];

        if (fence) {
          vkDestroyFence(r->vk_device, fence, r->vk_allocation_callbacks);
          r->vk_in_flight_fences[i] = VK_NULL_HANDLE;
        }
      }
      r->vk_in_flight_fence_count = 0;

      for (u32 i = 0; i < SF_SIZE(r->vk_image_acquired_semaphores); ++i) {
        VkSemaphore semaphore = r->vk_image_acquired_semaphores[i];

        if (semaphore) {
          vkDestroySemaphore(r->vk_device, semaphore, r->vk_allocation_callbacks);
          r->vk_image_acquired_semaphores[i] = VK_NULL_HANDLE;
        }
      }
      r->vk_image_acquired_semaphore_count = 0;

      for (u32 i = 0; i < SF_SIZE(r->main_command_buffers); ++i) {
        sf_handle commandBuffer = r->main_command_buffers[i];

        if (!sf_graphics_is_null_handle(commandBuffer)) {
          sf_graphics_destroy_command_buffer(r, commandBuffer);
          r->main_command_buffers[i] = SF_NULL_HANDLE;
        }
      }
      r->main_command_buffer_count = 0;

      vkDestroyDevice(r->vk_device, r->vk_allocation_callbacks);
      r->vk_device = VK_NULL_HANDLE;
    }

    if (r->vk_instance) {
      if (r->vk_surface) {
        vkDestroySurfaceKHR(r->vk_instance, r->vk_surface, r->vk_allocation_callbacks);
        r->vk_surface = VK_NULL_HANDLE;
      }

      if (r->vk_destroy_debug_utils_messenger_ext && r->vk_validation_messenger) {
        r->vk_destroy_debug_utils_messenger_ext(r->vk_instance, r->vk_validation_messenger, r->vk_allocation_callbacks);
        r->vk_validation_messenger = VK_NULL_HANDLE;
      }

      vkDestroyInstance(r->vk_instance, r->vk_allocation_callbacks);
      r->vk_instance = VK_NULL_HANDLE;
    }

    sf_arena_clear(&r->arena);
  }
}

SF_EXTERNAL void sf_graphics_begin_frame(sf_graphics_renderer *r) {
  // https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html

  u32     current_frame_index     = r->current_frame_index;
  VkFence current_in_flight_fence = r->vk_in_flight_fences[current_frame_index];
  vkWaitForFences(r->vk_device, 1, &current_in_flight_fence, VK_TRUE, (uint64_t)-1);
  vkResetFences(r->vk_device, 1, &current_in_flight_fence);

  sf_graphics_command_buffer *current_command_buffer = sf_graphics_command_buffer_from_handle(
      r, r->main_command_buffers[current_frame_index]);
  vkResetCommandPool(r->vk_device, current_command_buffer->vk_command_pool, 0);

  u32         current_image_index              = 0;
  VkSemaphore current_image_acquired_semaphore = r->vk_image_acquired_semaphores[current_frame_index];
  VkResult    result                           = vkAcquireNextImageKHR(
      r->vk_device, r->vk_swapchain, (u64)-1, current_image_acquired_semaphore, VK_NULL_HANDLE, &r->current_swapchain_image_index);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_ERROR_SURFACE_LOST_KHR) {
    SF_VULKAN_CHECK(result);
    vkDeviceWaitIdle(r->vk_device);
    sf_graphics_vulkan_destroy_swapchain_resources(r);
    sf_graphics_vulkan_create_swapchain_resources(r);
    r->swapchain_skip_end_frame = SF_TRUE;
    current_image_index         = r->current_swapchain_image_index;
  } else {
    current_image_index = r->current_swapchain_image_index;

    VkCommandBufferBeginInfo command_buffer_begin_info = {0};

    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext            = NULL;
    command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(current_command_buffer->vk_command_buffer, &command_buffer_begin_info);

    VkClearValue clear_values[2] = {0};

    clear_values[0].color.float32[0]     = r->swapchain_color_clear_value.data.rgba.r;
    clear_values[0].color.float32[1]     = r->swapchain_color_clear_value.data.rgba.g;
    clear_values[0].color.float32[2]     = r->swapchain_color_clear_value.data.rgba.b;
    clear_values[0].color.float32[3]     = r->swapchain_color_clear_value.data.rgba.a;
    clear_values[1].depthStencil.depth   = r->swapchain_depth_stencil_clear_value.data.detph_stencil.depth;   // 1.0F
    clear_values[1].depthStencil.stencil = r->swapchain_depth_stencil_clear_value.data.detph_stencil.stencil; // 0.0F

    sf_graphics_render_target *current_render_target = sf_graphics_render_target_from_handle(
        r, r->swapchain_render_targets[current_image_index]);

    VkRenderPassBeginInfo render_pass_begin_info = {0};

    render_pass_begin_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.pNext                    = NULL;
    render_pass_begin_info.renderPass               = current_render_target->vk_render_pass;
    render_pass_begin_info.framebuffer              = current_render_target->vk_framebuffer;
    render_pass_begin_info.renderArea.offset.x      = 0;
    render_pass_begin_info.renderArea.offset.y      = 0;
    render_pass_begin_info.renderArea.extent.width  = current_render_target->width;
    render_pass_begin_info.renderArea.extent.height = current_render_target->height;

    render_pass_begin_info.clearValueCount = SF_SIZE(clear_values);
    render_pass_begin_info.pClearValues    = clear_values;

    vkCmdBeginRenderPass(current_command_buffer->vk_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {0};

    viewport.x        = 0;
    viewport.y        = 0;
    viewport.width    = current_render_target->width;
    viewport.height   = current_render_target->height;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = .0F;

    vkCmdSetViewport(current_command_buffer->vk_command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {0};

    scissor.offset.x      = 0;
    scissor.offset.y      = 0;
    scissor.extent.width  = current_render_target->width;
    scissor.extent.height = current_render_target->height;

    vkCmdSetScissor(current_command_buffer->vk_command_buffer, 0, 1, &scissor);
  }
}

SF_EXTERNAL void sf_graphics_end_frame(sf_graphics_renderer *r) {
  if (!r->swapchain_skip_end_frame) {
    u32                         current_frame_index    = r->current_frame_index;
    sf_graphics_command_buffer *current_command_buffer = sf_graphics_command_buffer_from_handle(
        r, r->main_command_buffers[current_frame_index]);

    vkCmdEndRenderPass(current_command_buffer->vk_command_buffer);
    vkEndCommandBuffer(current_command_buffer->vk_command_buffer);

    u32                  current_image_index              = r->current_swapchain_image_index;
    VkSemaphore          current_draw_complete_semaphore  = r->vk_draw_complete_semaphores[current_image_index];
    VkSemaphore          current_image_acquired_semaphore = r->vk_image_acquired_semaphores[current_frame_index];
    VkFence              current_in_flight_fence          = r->vk_in_flight_fences[current_frame_index];
    VkPipelineStageFlags stage                            = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = {0};

    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext                = NULL;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = &current_image_acquired_semaphore;
    submit_info.pWaitDstStageMask    = &stage;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &current_command_buffer->vk_command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &current_draw_complete_semaphore;

    vkQueueSubmit(r->vk_graphics_queue, 1, &submit_info, current_in_flight_fence);

    VkPresentInfoKHR present_info = {0};

    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext              = NULL;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &current_draw_complete_semaphore;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &r->vk_swapchain;
    present_info.pImageIndices      = &current_image_index;
    present_info.pResults           = NULL;

    VkResult result = vkQueuePresentKHR(r->vk_present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      SF_VULKAN_CHECK(result);
      vkDeviceWaitIdle(r->vk_device);
      sf_graphics_vulkan_destroy_swapchain_resources(r);
      sf_graphics_vulkan_create_swapchain_resources(r);
    }
  } else {
    r->swapchain_skip_end_frame = SF_FALSE;
  }
}

SF_INTERNAL void sf_graphics_glfw_platform_framebuffer_resize_callback(GLFWwindow *window, i32 width, i32 height) {
  sf_graphics_glfw_platform *platform = (sf_graphics_glfw_platform *)glfwGetWindowUserPointer(window);
  if (platform) {
    platform->window_width  = width;
    platform->window_height = height;
  }
}

SF_EXTERNAL sf_graphics_glfw_platform *sf_graphics_create_glfw_platform(sf_arena *arena, i32 width, i32 height, sf_string title) {
  sf_graphics_glfw_platform *platform = sf_arena_allocate(arena, sizeof(sf_graphics_glfw_platform));
  if (platform) {
    if (glfwInit()) {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

      sf_string window_title = sf_string_null_terminate(arena, title);
      platform->window       = glfwCreateWindow(width, height, window_title.data, NULL, NULL);
      if (platform->window) {
        glfwSetWindowUserPointer(platform->window, platform);
        glfwSetFramebufferSizeCallback(platform->window, sf_graphics_glfw_platform_framebuffer_resize_callback);
        glfwGetFramebufferSize(platform->window, &platform->window_width, &platform->window_height);
      }
    }

    if (!platform->window) {
      sf_graphics_destroy_glfw_platform(platform);
      platform = NULL;
    }
  }

  return platform;
}

SF_EXTERNAL void sf_graphics_destroy_glfw_platform(sf_graphics_glfw_platform *platform) {
  if (platform && platform->window) {
    glfwDestroyWindow(platform->window);
    platform->window = NULL;
  }

  glfwTerminate();
}

SF_EXTERNAL void sf_graphics_glfw_platform_process_events(sf_graphics_glfw_platform *platform) {
  if (platform)
    glfwPollEvents();
}

SF_EXTERNAL sf_bool sf_graphics_glfw_platform_should_close(sf_graphics_glfw_platform *platform) {
  sf_bool result = SF_TRUE;

  if (platform)
    result = glfwWindowShouldClose(platform->window);

  return result;
}

SF_INTERNAL void sf_graphics_glfw_platform_create_vulkan_surface(void *data, void *renderer) {
  sf_graphics_glfw_platform *platform = (sf_graphics_glfw_platform *)data;
  sf_graphics_renderer      *r        = (sf_graphics_renderer *)renderer;

  if (platform && r && r->vk_instance) {
    if (!SF_VULKAN_CHECK(glfwCreateWindowSurface(r->vk_instance, platform->window, r->vk_allocation_callbacks, &r->vk_surface)))
      r->vk_surface = VK_NULL_HANDLE;
  }
}

SF_INTERNAL void sf_graphics_glfw_platform_request_swapchain_dimensions(void *data, void *renderer) {
  sf_graphics_glfw_platform *platform = (sf_graphics_glfw_platform *)data;
  sf_graphics_renderer      *r        = (sf_graphics_renderer *)renderer;

  if (platform && r) {
    r->swapchain_width  = platform->window_width;
    r->swapchain_height = platform->window_height;
  }
}

SF_EXTERNAL void sf_graphics_glfw_platform_fill_renderer_description(sf_arena *arena, sf_graphics_glfw_platform *platform,
                                                                      sf_graphics_renderer_description *description) {
  if (platform && description) {
    description->data                         = platform;
    description->enable_vsync                 = SF_TRUE;
    description->width                        = platform->window_width;
    description->height                       = platform->window_height;
    description->create_vulkan_surface        = sf_graphics_glfw_platform_create_vulkan_surface;
    description->request_swapchain_dimensions = sf_graphics_glfw_platform_request_swapchain_dimensions;

    description->application_name = SF_STRING("SF");

    u32          base_instance_extension_count = 0;
    char const **base_instance_extensions      = glfwGetRequiredInstanceExtensions(&base_instance_extension_count);

    u32 required_instance_extension_count = base_instance_extension_count + 1;
    description->vk_instance_extensions   = sf_arena_allocate(arena, (required_instance_extension_count) * sizeof(char const *));
    if (description->vk_instance_extensions) {
      description->vk_instance_extension_count = required_instance_extension_count;

      for (u32 i = 0; i < base_instance_extension_count; ++i)
        description->vk_instance_extensions[i] = base_instance_extensions[i];

      description->vk_instance_extensions[base_instance_extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    SF_LOCAL_PERSIST char const *const validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    description->vk_instance_layer_count                   = SF_SIZE(validation_layers);
    description->vk_instance_layers                        = validation_layers;

    SF_LOCAL_PERSIST char const *const device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    description->vk_device_extension_count                 = SF_SIZE(device_extensions);
    description->vk_device_extensions                      = device_extensions;
  }
}
