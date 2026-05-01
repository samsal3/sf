#include "SFGraphics.h"

#include <stdio.h>

#define SF_POW(b, p) pow(b, p)
#define SF_SNPRINTF snprintf

#define sfLogError(...) fprintf(stderr, "\033[31m[ERROR]: \033[m" __VA_ARGS__)
#define sfLogInfo(...) fprintf(stdout, "[INFO]:" __VA_ARGS__)
#define sfLogWarning(...) fprintf(stdout, "\033[31m[WARNING]: \033[m" __VA_ARGS__)
#define sfLogVerbose(...) fprintf(stdout, "[VERBOSE]:" __VA_ARGS__)

#define SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(Name)                                                                    \
   SF_INTERNAL SFGraphics##Name *sfGraphicsGetOrAllocate##Name##FromResourcePool(SFArena *arena, SFGraphicsResourcePool *pool) {           \
      SFGraphics##Name *result = NULL;                                                                                                     \
      if (sfQueueIsEpty(&pool->free)) {                                                                                                    \
         result = sfArenaAllocate(arena, sizeof(SFGraphics##Name));                                                                        \
         if (!result)                                                                                                                      \
            return NULL;                                                                                                                   \
      } else {                                                                                                                             \
         SFQueue *head = SF_QUEUE_HEAD(&pool->free);                                                                                       \
         sfQueueRemove(head);                                                                                                              \
         result = SF_QUEUE_DATA(head, SFGraphics##Name, queue);                                                                            \
      }                                                                                                                                    \
      sfMemorySet(result, 0, sizeof(SFGraphics##Name));                                                                                    \
      sfQueueInit(&result->queue);                                                                                                         \
      sfQueueInsertHead(&pool->pool, &result->queue);                                                                                      \
      return result;                                                                                                                       \
   }

SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(Texture)
SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(RenderTarget)
SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(CommandBuffer)
SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(Pipeline)

#undef SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION

SF_INTERNAL void sfGraphicsDefaultInitResourcePool(SFGraphicsResourcePool *pool) {
   sfQueueInit(&pool->pool);
   sfQueueInit(&pool->free);
}

SF_INTERNAL SFString sfGraphicsStringFromVulkanResult(VkResult result) {
   switch (result) {
      case VK_SUCCESS:                        return SF_STRING("VK_SUCCESS");
      case VK_NOT_READY:                      return SF_STRING("VK_NOT_READY");
      case VK_TIMEOUT:                        return SF_STRING("VK_TIMEOUT");
      case VK_EVENT_SET:                      return SF_STRING("VK_EVENT_SET");
      case VK_EVENT_RESET:                    return SF_STRING("VK_EVENT_RESET");
      case VK_INCOMPLETE:                     return SF_STRING("VK_INCOMPLETE");
      case VK_ERROR_OUT_OF_HOST_MEMORY:       return SF_STRING("VK_ERROR_OUT_OF_HOST_MEMORY");
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return SF_STRING("VK_ERROR_OUT_OF_DEVICE_MEMORY");
      case VK_ERROR_INITIALIZATION_FAILED:    return SF_STRING("VK_ERROR_INITIALIZATION_FAILED");
      case VK_ERROR_DEVICE_LOST:              return SF_STRING("VK_ERROR_DEVICE_LOST");
      case VK_ERROR_MEMORY_MAP_FAILED:        return SF_STRING("VK_ERROR_MEMORY_MAP_FAILED");
      case VK_ERROR_LAYER_NOT_PRESENT:        return SF_STRING("VK_ERROR_LAYER_NOT_PRESENT");
      case VK_ERROR_EXTENSION_NOT_PRESENT:    return SF_STRING("VK_ERROR_EXTENSION_NO_PRESENT");
      case VK_ERROR_FEATURE_NOT_PRESENT:      return SF_STRING("VK_ERROR_FEATURE_NOT_PRESENT");
      case VK_ERROR_INCOMPATIBLE_DRIVER:      return SF_STRING("VK_ERROR_INCOMPATIBLE_DRIVER");
      case VK_ERROR_TOO_MANY_OBJECTS:         return SF_STRING("VK_ERROR_TOO_MANY_OBJECTS");
      case VK_ERROR_FORMAT_NOT_SUPPORTED:     return SF_STRING("VK_ERROR_FORMAT_NOT_SUPPORTED");
      case VK_ERROR_FRAGMENTED_POOL:          return SF_STRING("VK_ERROR_FRAGMENTED_POOL");
      case VK_ERROR_OUT_OF_POOL_MEMORY:       return SF_STRING("VK_ERROR_OUT_OF_POOL_MEMORY");
      case VK_ERROR_INVALID_EXTERNAL_HANDLE:  return SF_STRING("VK_ERROR_INVALID_EXTERNAL_HANDLE");
      case VK_ERROR_SURFACE_LOST_KHR:         return SF_STRING("VK_ERROR_SURFACE_LOST_KHR");
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return SF_STRING("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
      case VK_SUBOPTIMAL_KHR:                 return SF_STRING("VK_SUBOPTIMAL_KHR");
      case VK_ERROR_OUT_OF_DATE_KHR:          return SF_STRING("VK_ERROR_OUT_OF_DATE_KHR");

      case VK_ERROR_UNKNOWN:
      default:               return SF_STRING("VK_ERROR_UNKNOWN");
   }
}

static SFBool sfGraphicsVulkanCheck(VkResult result, SFString what, int line, SFString file) { return VK_SUCCESS == result; }
#define SF_VULKAN_CHECK(e) sf_graphics_check((e), #e, __LINE__, __FILE__)

static VkBool32 VKAPI_CALL sfGraphicsVulkanLog(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) {
   (void)messageTypes;
   (void)userData;

   switch (messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: sfLogVerbose("%s\n", callbackData->pMessage); break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    sfLogInfo("%s\n", callbackData->pMessage); break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: sfLogWarning("%s\n", callbackData->pMessage); break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   sfLogError("%s\n", callbackData->pMessage); break;
      default:                                              sfLogInfo("%s\n", callbackData->pMessage); break;
   }

   return VK_TRUE;
}

SF_INTERNAL SFBool sfGraphicsAreQueueFamilyIndicesValid(SFGraphicsRenderer *r) {
   return (U32)-1 != r->vkGraphicsQueueFamilyIndex && (U32)-1 != r->vkPresentQueueFamilyIndex;
}

typedef struct SFGraphicsQueueFamilyPropertyList {
   U32                      size;
   VkQueueFamilyProperties *data;
} SFGraphicsQueueFamilyPropertyList;

SF_INTERNAL SFGraphicsQueueFamilyPropertyList sfGraphicsCreateQueueFamilyPropertyList(SFArena *arena, VkDevice device) {
   SFGraphicsQueueFamilyPropertyList result = {0};

   vkGetPhysicalDeviceQueueFamilyProperties(device, &result.size, NULL);
   if (result.size) {
      result.data = sfArenaAllocate(arena, result.size * sizeof(*result.data));
      if (result.data)
         vkGetPhysicalDeviceQueueFamilyProperties(device, &result.size, result.data);
   }

   return result;
}

SF_INTERNAL void sfGraphicsVulkanFindSuitableQueueFamilyIndices(SFArena *arena, SFGraphicsRenderer *r) {
   SFGraphicsQueueFamilyPropertyList properties = sfGraphicsCreateQueueFamilyPropertyList(arena, r->vkPhysicalDevice);

   if (!properties.size || !properties.data)
      return;

   for (U32 i = 0; i < properties.size && !sfGraphicsAreQueueFamilyIndicesValid(r); ++i) {
      VkBool32 supportsSurface = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(r->vkPhysicalDevice, i, r->vkSurface, &supportsSurface);

      if (supportsSurface)
         r->vkPresentQueueFamilyIndex = i;

      if (properties.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
         r->vkGraphicsQueueFamilyIndex = i;
   }
}

typedef struct SFGraphicsExtensionPropertiesList {
   U32                    size;
   VkExtensionProperties *data;
} SFGraphicsExtensionPropertiesList;

SF_INTERNAL SFGraphicsExtensionPropertiesList sfGraphicsCreateExtensionPropertiesList(SFArena *arena, VkDevice device) {
   SFGraphicsExtensionPropertiesList result = {0};

   if (SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &result.size, NULL))) {
      result.data = sfArenaAllocate(arena, result.size * sizeof(*result.data));
      if (result.data)
         vkEnumerateDeviceExtensionProperties(device, NULL, &result.size, result.data);
   }

   return result;
}

SF_INTERNAL SFBool sfGraphicsCheckDeviceExtensionSupport(SFArena *arena, SFGraphicsRenderer *r,
                                                         SFGraphicsRendererDescription *description) {
   SFGraphicsExtensionPropertiesList available = sfGraphicsCreateExtensionPropertiesList(arena, r->vkPhysicalDevice);

   for (U32 i = 0; i < description->vkDeviceExtensionCount; ++i) {
      SFBool   foundCurrent = SF_FALSE;
      SFString required     = sfStringFromNonLiteral(arena, description->vkDeviceExtensions[i]);

      for (U32 j = 0; j < available.size && !foundCurrent; ++j) {
         SFString currentAvailable = sfStringFromNonLiteral(arena, available.data[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE);
         foundCurrent              = sfStringCompare(required, currentAvailable, VK_MAX_EXTENSION_NAME_SIZE);
      }

      if (!foundCurrent)
         return SF_FALSE;
   }

   return SF_TRUE;
}

SF_INTERNAL SFBool sfGraphicsCheckDeviceSwapchainSupport(SFGraphicsRenderer *r) {
   U32 surfaceFormatCount = 0, presentModeCount = 0;

   SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vkPhysicalDevice, r->vkSurface, &surfaceFormatCount, NULL));
   SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vkPhysicalDevice, r->vkSurface, &presentModeCount, NULL));

   return surfaceFormatCount && presentModeCount;
}

typedef struct SFGraphicsSurfaceFormatList {
   U32                 size;
   VkSurfaceFormatKHR *data;
} SFGraphicsSurfaceFormatList;

SF_INTERNAL SFGraphicsSurfaceFormatList sfGraphicsCreateSurfaceFormatList(SFArena *arena, VkPhysicalDevice device, VkSurfaceKHR surface) {
   SFGraphicsSurfaceFormatList result = {0};

   if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &result.size, NULL))) {
      result.data = sfArenaAllocate(arena, result.size * sizeof(result.data));
      if (result.data)
         vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &result.size, result.data);
   }

   return result;
}

typedef struct SFGraphicsPresentModeList {
   U32               size;
   VkPresentModeKHR *data;
} SFGraphicsPresentModeList;

SF_INTERNAL SFGraphicsPresentModeList sfGraphicsCreatePresentModeList(SFArena *arena, VkPhysicalDevice device, VkSurfaceKHR surface) {
   SFGraphicsPresentModeList result = {0};

   if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &result.size, NULL))) {
      result.data = sfArenaAllocate(arena, result.size * sizeof(result.data));
      if (result.data)
         vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &result.size, result.data);
   }

   return result;
}

SF_INTERNAL SFBool sfGraphicsTestFormatFeatures(VkPhysicalDevice device, VkFormat format, VkImageTiling tiling,
                                                VkFormatFeatureFlags features) {
   VkFormatProperties properties = {0};

   vkGetPhysicalDeviceFormatProperties(device, format, &properties);

   if (VK_IMAGE_TILING_LINEAR == tiling)
      return !!(properties.linearTilingFeatures & features);
   else if (VK_IMAGE_TILING_OPTIMAL == tiling)
      return !!(properties.optimalTilingFeatures & features);

   return SF_FALSE;
}

SF_INTERNAL U32 sfGraphicsFindMemoryTypeIndex(VkPhysicalDevice device, VkMemoryPropertyFlags memProps, U32 filter) {
   VkPhysicalDeviceMemoryProperties available = {0};

   vkGetPhysicalDeviceMemoryProperties(device, &available);

   for (U32 i = 0; i < available.memoryTypeCount; ++i)
      if ((filter & (1 << i)) && (available.memoryTypes[i].propertyFlags & memProps) == memProps)
         return i;

   return (U32)-1;
}

SF_INTERNAL VkDeviceMemory sfGraphicsAllocateMemory(SFGraphicsRenderer *r, VkMemoryPropertyFlags memProps, U32 filter, U64 size) {
   VkDeviceMemory memory = VK_NULL_HANDLE;

   VkMemoryAllocateInfo info = {0};
   info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   info.pNext                = NULL;
   info.allocationSize       = size;
   info.memoryTypeIndex      = sfGraphicsFindMemoryTypeIndex(r->vkPhysicalDevice, memProps, filter);
   if (info.memoryTypeIndex == (U32)-1)
      return VK_NULL_HANDLE;

   if (SF_VULKAN_CHECK(vkAllocateMemory(r->vkDevice, &info, r->vkAllocationCallbacks, &memory)))
      return memory;

   return VK_NULL_HANDLE;
}

SF_INTERNAL VkDeviceMemory sfGraphicsAllocateMemoryForImage(SFGraphicsRenderer *r, VkImage image, VkMemoryPropertyFlags memProps) {
   VkMemoryRequirements requirements = {0};
   vkGetImageMemoryRequirements(r->vkDevice, image, &requirements);

   VkDeviceMemory memory = sfGraphicsAllocateMemory(r, memProps, requirements.memoryTypeBits, requirements.size);
   if (!memory)
      return VK_NULL_HANDLE;

   if (!SF_VULKAN_CHECK(vkBindImageMemory(r->vkDevice, image, memory, 0))) {
      vkFreeMemory(r->vkDevice, memory, r->vkAllocationCallbacks);
      return VK_NULL_HANDLE;
   }

   return memory;
}

SF_INTERNAL VkDeviceMemory sfGraphicsAllocateMemoryForBuffer(SFGraphicsRenderer *r, VkBuffer buffer, VkMemoryPropertyFlags memProps) {
   VkMemoryRequirements requirements = {0};
   vkGetBufferMemoryRequirements(r->vkDevice, buffer, &requirements);

   VkDeviceMemory memory = sfGraphicsAllocateMemory(r, memProps, requirements.memoryTypeBits, requirements.size);
   if (!memory)
      return VK_NULL_HANDLE;

   if (!SF_VULKAN_CHECK(vkBindImageMemory(r->vkDevice, buffer, memory, 0))) {
      vkFreeMemory(r->vkDevice, memory, r->vkAllocationCallbacks);
      return VK_NULL_HANDLE;
   }

   return memory;
}

SF_INTERNAL VkImageViewType sfGraphicsVulkanImageViewTypeFromTextureType(SFGraphicsTextureType type) {
   switch (type) {
      case SF_GRAPHICS_TEXTURE_TYPE_1D:   return VK_IMAGE_VIEW_TYPE_1D;
      case SF_GRAPHICS_TEXTURE_TYPE_2D:   return VK_IMAGE_VIEW_TYPE_2D;
      case SF_GRAPHICS_TEXTURE_TYPE_3D:   return VK_IMAGE_VIEW_TYPE_3D;
      case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
   }
}

SF_INTERNAL VkImageType sfGraphicsVulkanImageTypeFromTextureType(SFGraphicsTextureType type) {
   switch (type) {
      case SF_GRAPHICS_TEXTURE_TYPE_1D:   return VK_IMAGE_TYPE_1D;
      case SF_GRAPHICS_TEXTURE_TYPE_2D:   return VK_IMAGE_TYPE_2D;
      case SF_GRAPHICS_TEXTURE_TYPE_3D:   return VK_IMAGE_TYPE_3D;
      case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_TYPE_2D;
   }
}

SF_INTERNAL VkFormat sfGraphicsVulkanFormatFromFormat(SFGraphicsFormat format) {
   switch (format) {
      // 1 channel
      case SF_GRAPHICS_FORMAT_R8_UNORM:            return VK_FORMAT_R8_UNORM;
      case SF_GRAPHICS_FORMAT_R16_UNORM:           return VK_FORMAT_R16_UNORM;
      case SF_GRAPHICS_FORMAT_R16_UINT:            return VK_FORMAT_R16_UINT;
      case SF_GRAPHICS_FORMAT_R16_SFLOAT:          return VK_FORMAT_R16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32_UINT:            return VK_FORMAT_R32_UINT;
      case SF_GRAPHICS_FORMAT_R32_SFLOAT:          return VK_FORMAT_R32_SFLOAT;
      // 2 channel
      case SF_GRAPHICS_FORMAT_R8G8_UNORM:          return VK_FORMAT_R8G8_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16_UNORM:        return VK_FORMAT_R16G16_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:       return VK_FORMAT_R16G16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32G32_UINT:         return VK_FORMAT_R32G32_UINT;
      case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:       return VK_FORMAT_R32G32_SFLOAT;
      // 3 channel
      case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:        return VK_FORMAT_R8G8B8_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:     return VK_FORMAT_R16G16B16_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:    return VK_FORMAT_R16G16B16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32G32B32_UINT:      return VK_FORMAT_R32G32B32_UINT;
      case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
      // 4 channel
      case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:      return VK_FORMAT_B8G8R8A8_UNORM;
      case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:      return VK_FORMAT_R8G8B8A8_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:  return VK_FORMAT_R16G16B16A16_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:   return VK_FORMAT_R32G32B32A32_UINT;
      case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
      // Depth/stencil
      case SF_GRAPHICS_FORMAT_D16_UNORM:           return VK_FORMAT_D16_UNORM;
      case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: return VK_FORMAT_X8_D24_UNORM_PACK32;
      case SF_GRAPHICS_FORMAT_D32_SFLOAT:          return VK_FORMAT_D32_SFLOAT;
      case SF_GRAPHICS_FORMAT_S8_UINT:             return VK_FORMAT_S8_UINT;
      case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:   return VK_FORMAT_D16_UNORM_S8_UINT;
      case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:   return VK_FORMAT_D24_UNORM_S8_UINT;
      case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
   }
}

SF_INTERNAL VkImageAspectFlags sfGraphicsVulkanImageAspectFlagsFromFormat(SFGraphicsFormat format) {
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
      case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:
      case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:
      case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT:
      case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:
      case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return VK_IMAGE_ASPECT_COLOR_BIT;
      // Depth/stencil
      case SF_GRAPHICS_FORMAT_D16_UNORM:
      case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32:
      case SF_GRAPHICS_FORMAT_D32_SFLOAT:          return VK_IMAGE_ASPECT_DEPTH_BIT;
      case SF_GRAPHICS_FORMAT_S8_UINT:             return VK_IMAGE_ASPECT_STENCIL_BIT;
      case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:
      case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:
      case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:  return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      default:                                     return 0;
   }
}

SF_INTERNAL VkSampleCountFlags sfGraphicsVulkanSampleCountFromSampleCount(SFGraphicsSampleCount samples) {
   switch (samples) {
      case SF_GRAPHICS_SAMPLE_COUNT_1:  return VK_SAMPLE_COUNT_1_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_2:  return VK_SAMPLE_COUNT_2_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_4:  return VK_SAMPLE_COUNT_4_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_8:  return VK_SAMPLE_COUNT_8_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_16: return VK_SAMPLE_COUNT_16_BIT;
   }
}

SF_INTERNAL VkImageUsageFlags sfGraphicsVulkanImageUsageFlagsFromTextureUsage(SFGraphicsTextureUsageFlags usage) {
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
SF_INTERNAL void sfGraphicsVulkanCreateImageView(SFGraphicsRenderer *r, SFGraphicsTexture *texture) {
   VkImageViewCreateInfo info = {0};

   info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   info.pNext                           = NULL;
   info.flags                           = 0;
   info.image                           = texture->vkImage;
   info.viewType                        = sfGraphicsVulkanImageViewTypeFromTextureType(texture->type);
   info.format                          = sfGraphicsVulkanFormatFromFormat(texture->format);
   info.components.r                    = VK_COMPONENT_SWIZZLE_R;
   info.components.g                    = VK_COMPONENT_SWIZZLE_G;
   info.components.b                    = VK_COMPONENT_SWIZZLE_B;
   info.components.a                    = VK_COMPONENT_SWIZZLE_A;
   info.subresourceRange.aspectMask     = sfGraphicsVulkanImageAspectFlagsFromFormat(texture->format);
   info.subresourceRange.baseMipLevel   = 0;
   info.subresourceRange.levelCount     = texture->mips;
   info.subresourceRange.baseArrayLayer = 0;
   info.subresourceRange.layerCount     = 1;

   SF_VULKAN_CHECK(vkCreateImageView(r->vkDevice, &info, r->vkAllocationCallbacks, &texture->vkImageView));
}

SF_INTERNAL void sfGraphicsVulkanCreateImage(SFGraphicsRenderer *r, SFGraphicsTexture *texture) {
   VkImageCreateInfo info = {0};

   info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   info.pNext                 = NULL;
   info.flags                 = 0;
   info.imageType             = sfGraphicsVulkanImageTypeFromTextureType(texture->type);
   info.format                = sfGraphicsVulkanFormatFromFormat(texture->format);
   info.extent.width          = texture->width;
   info.extent.height         = texture->height;
   info.extent.depth          = texture->depth;
   info.mipLevels             = texture->mips;
   info.arrayLayers           = 1;
   info.samples               = sfGraphicsVulkanSampleCountFromSampleCount(texture->samples);
   info.tiling                = texture->mapped ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
   info.usage                 = sfGraphicsVulkanImageUsageFlagsFromTextureUsage(texture->usage);
   info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
   info.queueFamilyIndexCount = 0;
   info.pQueueFamilyIndices   = NULL;
   info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

   SF_VULKAN_CHECK(vkCreateImage(r->vkDevice, &info, r->vkAllocationCallbacks, &texture->vkImage));
}

SF_INTERNAL void sfGraphicsVulkanAllocateTextureMemory(SFGraphicsRenderer *r, SFGraphicsTexture *texture) {
   VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
   if (texture->mapped)
      properties |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

   texture->vkMemory = sfGraphicsAllocateMemoryForImage(r, texture->vkImage, properties);
   if (!texture->vkMemory)
      goto error;

   if (texture->mapped &&
       !SF_VULKAN_CHECK(vkMapMemory(r->vkDevice, texture->vkMemory, texture->vkImage, VK_WHOLE_SIZE, 0, &texture->mappedData)))
      goto error;

   return;

error:
   if (texture->vkMemory) {
      vkFreeMemory(r->vkDevice, texture->vkMemory, r->vkAllocationCallbacks);
      texture->vkMemory = VK_NULL_HANDLE;
   }
}

SF_EXTERNAL void sfGraphicsDestroyTexture(SFGraphicsRenderer *r, SFHandle handle) {
   SFGraphicsTexture *texture = (SFGraphicsTexture *)handle.value;

   if (!r || !texture)
      return;

   if (r->vkDevice) {
      sfQueueRemove(&texture->queue);
      sfQueueInsertHead(&r->texturePool.free, &texture->queue);

      if (texture->vkImageView) {
         vkDestroyImageView(r->vkDevice, texture->vkImageView, r->vkAllocationCallbacks);
         texture->vkImageView = VK_NULL_HANDLE;
      }

      if (texture->vkOwnsMemoryAndImage) {
         if (texture->vkMemory) {
            vkFreeMemory(r->vkDevice, texture->vkMemory, r->vkAllocationCallbacks);
            texture->vkMemory = VK_NULL_HANDLE;
         }
         if (texture->vkImage) {
            vkDestroyImage(r->vkDevice, texture->vkImage, r->vkAllocationCallbacks);
            texture->vkImage = VK_NULL_HANDLE;
         }
      }
   }
}

SF_INTERNAL SFHandle sfGraphicsVulkanCreateTexture(SFGraphicsRenderer *r, SFGraphicsTextureType type, SFGraphicsFormat format,
                                                   SFGraphicsSampleCount samples, SFGraphicsTextureUsage usage, U32 width, U32 height,
                                                   U32 depth, U32 mips, SFBool mapped, SFGraphicsClearValue *clearValue,
                                                   VkImage vkNotOwnedImage) {
   if (!r)
      return SF_NULL_HANDLE;

   SFGraphicsTexture *texture = sfGraphicsGetOrAllocateTextureFromResourcePool(&r->arena, &r->texturePool);
   if (!texture)
      return SF_NULL_HANDLE;

   texture->type    = type;
   texture->format  = format;
   texture->samples = samples;
   texture->usage   = usage;
   texture->width   = width;
   texture->height  = height;
   texture->depth   = depth;
   texture->mips    = mips;
   texture->mapped  = mapped;
   if (clearValue)
      texture->clearValue = *clearValue;
   else
      texture->clearValue.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;

   if (!vkNotOwnedImage) {
      texture->vkOwnsMemoryAndImage = SF_TRUE;

      sfGraphicsVulkanCreateImage(r, texture);
      if (!texture->vkImage)
         goto error;

      sfGraphicsVulkanAllocateTextureMemory(r, texture);
      if (!texture->vkMemory)
         goto error;

   } else {
      texture->vkOwnsMemoryAndImage = SF_FALSE;
      texture->vkImage              = vkNotOwnedImage;
      texture->mapped               = SF_FALSE;
   }

   sfGraphicsVulkanCreateImageView(r, texture);
   if (!texture->vkImageView)
      goto error;

   return SF_AS_HANDLE(texture);

error:
   sfGraphicsDestroyTexture(r, SF_AS_HANDLE(texture));
   return SF_NULL_HANDLE;
}

SF_EXTERNAL SFHandle sfGraphicsCreateTexture(SFGraphicsRenderer *r, SFGraphicsTextureType type, SFGraphicsFormat format,
                                             SFGraphicsSampleCount samples, SFGraphicsTextureUsage usage, uint32_t width, uint32_t height,
                                             uint32_t depth, uint32_t mips, SFBool mapped, SFGraphicsClearValue *clearValue) {
   return sfGraphicsVulkanCreateTexture(r, type, format, samples, usage, width, height, depth, mips, mapped, clearValue, VK_NULL_HANDLE);
}

SF_INTERNAL void sfGraphicsVulkanCreateRenderTargetRenderPass(SFGraphicsRenderer *r, SFGraphicsRenderTarget *renderTarget) {
   SFArena *arena = &r->renderTargetArena;

   U32 descriptionCount = renderTarget->colorAttachmentCount + renderTarget->resolveAttachmentCount +
                          !!renderTarget->depthStencilAttachment.value;

   VkAttachmentDescription *descriptions = sfArenaAllocate(arena, descriptionCount * sizeof(*descriptions));
   if (!descriptions)
      goto error;

   VkAttachmentReference *colorReferences = sfArenaAllocate(arena, renderTarget->colorAttachmentCount * sizeof(*colorReferences));
   if (!colorReferences)
      goto error;

   VkAttachmentReference *resolveReferences = sfArenaAllocate(arena, renderTarget->resolveAttachmentCount * sizeof(*resolveReferences));
   if (renderTarget->resolveAttachmentCount && !resolveReferences)
      goto error;

   for (U32 i = 0; i < renderTarget->colorAttachmentCount; ++i) {
      U32                      descIndex = i * (!!renderTarget->resolveAttachmentCount) * 2;
      VkAttachmentDescription *desc      = &descriptions[descIndex];
      VkAttachmentReference   *reference = &colorReferences[i];

      SFGraphicsTexture *attachment = (SFGraphicsTexture *)renderTarget->colorAttachments[i].value;

      desc->flags          = 0;
      desc->format         = sfGraphicsVulkanFormatFromFormat(attachment->format);
      desc->samples        = VK_SAMPLE_COUNT_1_BIT;
      desc->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
      desc->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
      desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
      desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
      if (attachment->vkOwnsMemoryAndImage)
         desc->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      else
         desc->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      reference->attachment = descIndex;
      reference->layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   }

   for (U32 i = 0; i < renderTarget->resolveAttachmentCount; ++i) {
      U32                      descIndex = i * 2 + 1;
      VkAttachmentDescription *desc      = &descriptions[descIndex];
      VkAttachmentReference   *reference = &resolveReferences[i];

      SFGraphicsTexture *attachment = (struct sf_graphics_texture *)renderTarget->resolveAttachments[i].value;

      desc->flags          = 0;
      desc->format         = sfGraphicsVulkanFormatFromFormat(attachment->format);
      desc->samples        = sfGraphicsVulkanSampleCountFromSampleCount(attachment->samples);
      desc->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
      desc->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
      desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
      desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
      desc->finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      reference->attachment = descIndex;
      reference->layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   }

   VkAttachmentReference depthStencilReference = {0};
   if (renderTarget->depthStencilAttachment.value) {
      U32                      descIndex = descriptionCount - 1;
      VkAttachmentDescription *desc      = &descriptions[descIndex];
      VkAttachmentReference   *ref       = &depthStencilReference;

      SFGraphicsTexture *attachment = (SFGraphicsTexture *)renderTarget->depthStencilAttachment.value;

      desc->flags          = 0;
      desc->format         = sfGraphicsVulkanFormatFromFormat(attachment->format);
      desc->samples        = sfGraphicsVulkanSampleCountFromSampleCount(attachment->samples);
      desc->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
      desc->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
      desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
      desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
      desc->finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      ref->attachment = descIndex;
      ref->layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   }

   VkSubpassDescription subpass = {0};
   subpass.flags                = 0;
   subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.inputAttachmentCount = 0;
   subpass.pInputAttachments    = NULL;
   subpass.colorAttachmentCount = renderTarget->colorAttachmentCount;
   subpass.pColorAttachments    = colorReferences;
   subpass.pResolveAttachments  = resolveReferences;
   if (renderTarget->depthStencilAttachment.value)
      subpass.pDepthStencilAttachment = &depthStencilReference;
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
   info.attachmentCount        = descriptionCount;
   info.pAttachments           = descriptions;
   info.subpassCount           = 1;
   info.pSubpasses             = &subpass;
   info.dependencyCount        = 1;
   info.pDependencies          = &dependency;

   SF_VULKAN_CHECK(vkCreateRenderPass(r->vkDevice, &info, r->vkAllocationCallbacks, &renderTarget->vkRenderPass));

error:
   sfArenaClear(arena);
}

SF_INTERNAL void sfGraphicsVulkanCreateRenderTargetFramebuffer(SFGraphicsRenderer *r, SFGraphicsRenderTarget *renderTarget) {
   VkImageView attachments[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT] = {0};

   for (U32 i = 0; i < renderTarget->colorAttachmentCount; ++i) {
      U32                attachmentIndex = i * (!!renderTarget->resolveAttachmentCount) * 2;
      SFGraphicsTexture *attachment      = (SFGraphicsTexture *)renderTarget->colorAttachments[i].value;
      attachments[attachmentIndex]       = attachment->vkImageView;
   }

   for (U32 i = 0; i < renderTarget->resolveAttachmentCount; ++i) {
      U32                attachmentIndex = i * 2 + 1;
      SFGraphicsTexture *attachment      = (SFGraphicsTexture *)renderTarget->resolveAttachments[i].value;
      attachments[attachmentIndex]       = attachment->vkImageView;
   }

   if (renderTarget->depthStencilAttachment.value) {
      U32                attachmentIndex = renderTarget->totalAttachmentCount - 1;
      SFGraphicsTexture *attachment      = (SFGraphicsTexture *)renderTarget->depthStencilAttachment.value;
      attachments[attachmentIndex]       = attachment->vkImageView;
   }

   VkFramebufferCreateInfo info = {0};
   info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   info.pNext                   = NULL;
   info.flags                   = 0;
   info.renderPass              = renderTarget->vkRenderPass;
   info.attachmentCount         = renderTarget->totalAttachmentCount;
   info.pAttachments            = attachments;
   info.width                   = renderTarget->width;
   info.height                  = renderTarget->height;
   info.layers                  = 1;

   SF_VULKAN_CHECK(vkCreateFramebuffer(r->vkDevice, &info, r->vkAllocationCallbacks, &renderTarget->vkFramebuffer));
}

SF_EXTERNAL void sfGraphicsDestroyRenderTarget(SFGraphicsRenderer *r, SFHandle handle) {
   SFGraphicsRenderTarget *renderTarget = (SFGraphicsRenderTarget *)handle.value;

   if (!r || !renderTarget)
      return;

   sfQueueRemove(&renderTarget->queue);
   sfQueueInsertHead(&r->renderTargetPool.free, &renderTarget->queue);

   if (r->vkDevice) {
      if (renderTarget->vkFramebuffer) {
         vkDestroyFramebuffer(r->vkDevice, renderTarget->vkFramebuffer, r->vkAllocationCallbacks);
         renderTarget->vkFramebuffer = VK_NULL_HANDLE;
      }

      if (renderTarget->vkRenderPass) {
         vkDestroyRenderPass(r->vkDevice, renderTarget->vkRenderPass, r->vkAllocationCallbacks);
         renderTarget->vkRenderPass = VK_NULL_HANDLE;
      }
   }

   for (U32 i = 0; i < renderTarget->colorAttachmentCount; ++i) {
      sfGraphicsDestroyTexture(r, renderTarget->colorAttachments[i]);
      renderTarget->colorAttachments[i] = SF_NULL_HANDLE;
   }
   renderTarget->colorAttachmentCount = 0;

   for (U32 i = 0; i < renderTarget->resolveAttachmentCount; ++i) {
      sfGraphicsDestroyTexture(r, renderTarget->resolveAttachments[i]);
      renderTarget->resolveAttachments[i] = SF_NULL_HANDLE;
   }
   renderTarget->resolveAttachmentCount = 0;

   sfGraphicsDestroyTexture(r, renderTarget->depthStencilAttachment);
   renderTarget->depthStencilAttachment = SF_NULL_HANDLE;
}

SF_INTERNAL SFHandle sfGraphicsVulkanCreateRenderTarget(SFGraphicsRenderer *r, U32 width, U32 height, SFGraphicsSampleCount samples,
                                                        SFGraphicsFormat colorFormat, SFGraphicsFormat depthStencilFormat,
                                                        U32 colorAttachmentCount, SFGraphicsClearValue *colorClearValues,
                                                        SFGraphicsClearValue *depthStencilClearValue, VkImage vkSwapchainImage) {
   if (!r)
      return SF_NULL_HANDLE;

   SFGraphicsRenderTarget *renderTarget = sfGraphicsGetOrAllocateRenderTargetFromResourcePool(&r->arena, &r->renderTargetPool);
   if (!renderTarget)
      return SF_NULL_HANDLE;

   renderTarget->samples                = samples;
   renderTarget->width                  = width;
   renderTarget->height                 = height;
   renderTarget->resolveAttachmentCount = SF_GRAPHICS_SAMPLE_COUNT_1 != samples ? colorAttachmentCount : 0;
   renderTarget->colorAttachmentCount   = colorAttachmentCount;

   renderTarget->totalAttachmentCount = renderTarget->colorAttachmentCount + renderTarget->resolveAttachmentCount +
                                        (SF_GRAPHICS_FORMAT_UNDEFINED != depthStencilFormat);

   for (U32 i = 0; i < renderTarget->colorAttachmentCount; ++i) {
      SFGraphicsClearValue *clearValue  = colorClearValues ? &colorClearValues[i] : NULL;
      renderTarget->colorAttachments[i] = sfGraphicsVulkanCreateTexture(
          r, SF_GRAPHICS_TEXTURE_TYPE_2D, colorFormat, SF_GRAPHICS_SAMPLE_COUNT_1, SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT, width,
          height, 1, 1, SF_FALSE, clearValue, vkSwapchainImage);
      if (!renderTarget->colorAttachments[i].value)
         goto error;
   }

   for (U32 i = 0; i < renderTarget->resolveAttachmentCount; ++i) {
      SFGraphicsClearValue *clearValue = colorClearValues ? &colorClearValues[i] : NULL;

      renderTarget->resolveAttachments[i] = sfGraphicsCreateTexture(r, SF_GRAPHICS_TEXTURE_TYPE_2D, colorFormat, samples,
                                                                    SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT, width, height, 1, 1,
                                                                    SF_FALSE, clearValue);
      if (!renderTarget->resolveAttachments[i].value)
         goto error;
   }

   if (SF_GRAPHICS_FORMAT_UNDEFINED != depthStencilFormat) {
      renderTarget->depthStencilAttachment = sfGraphicsCreateTexture(r, SF_GRAPHICS_TEXTURE_TYPE_2D, depthStencilFormat, samples,
                                                                     SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT, width, height, 1,
                                                                     1, SF_FALSE, depthStencilClearValue);
      if (!renderTarget->depthStencilAttachment.value)
         goto error;
   }

   sfGraphicsVulkanCreateRenderTargetRenderPass(r, renderTarget);
   if (!renderTarget->vkRenderPass)
      goto error;

   sfGraphicsVulkanCreateRenderTargetFramebuffer(r, renderTarget);
   if (!renderTarget->vkFramebuffer)
      goto error;

   return SF_AS_HANDLE(renderTarget);

error:
   sfGraphicsDestroyRenderTarget(r, SF_AS_HANDLE(renderTarget));
   return SF_NULL_HANDLE;
}

SF_EXTERNAL SFHandle sfGraphicsCreateRenderTarget(SFGraphicsRenderer *r, U32 width, U32 height, SFGraphicsSampleCount samples,
                                                  SFGraphicsFormat colorFormat, SFGraphicsFormat depthStencilFormat,
                                                  uint32_t colorAttachmentCount, SFGraphicsClearValue *colorClearValues,
                                                  SFGraphicsClearValue *depthStencilClearValue) {
   return sfGraphicsVulkanCreateRenderTarget(r, width, height, samples, colorFormat, depthStencilFormat, colorAttachmentCount,
                                             colorClearValues, depthStencilClearValue, VK_NULL_HANDLE);
}

SF_INTERNAL void sfGraphicsVulkanDestroySwapchainResources(SFGraphicsRenderer *r) {
   if (!r->vkDevice)
      return;

   for (U32 i = 0; i < r->swapchainRenderTargetCount; ++i) {
      sfGraphicsDestroyRenderTarget(r, r->swapchainRenderTargets[i]);
      r->swapchainRenderTargets[i] = SF_NULL_HANDLE;
   }
   r->swapchainRenderTargetCount = 0;

   for (U32 i = 0; i < SF_SIZE(r->vkSwapchainImages); ++i)
      r->vkSwapchainImages[i] = VK_NULL_HANDLE;

   r->vkSwapchainImageCount = 0;

   if (r->vkSwapchain) {
      vkDestroySwapchainKHR(r->vkDevice, r->vkSwapchain, r->vkAllocationCallbacks);
      r->vkSwapchain = VK_NULL_HANDLE;
   }
}

static void sfGraphicsVulkanCreateSwapchainResources(SFGraphicsRenderer *r) {
   {
      VkSurfaceCapabilitiesKHR capabilities = {0};
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->vkPhysicalDevice, r->vkSurface, &capabilities);

      U32 queueFamilyIndices[] = {r->vkGraphicsQueueFamilyIndex, r->vkPresentQueueFamilyIndex};

      VkSwapchainCreateInfoKHR info = {0};

      info.sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      info.pNext              = NULL;
      info.flags              = 0;
      info.surface            = r->vkSurface;
      info.minImageCount      = r->swapchainRequestedImageCount;
      info.imageFormat        = r->vkSurfaceFormat.format;
      info.imageColorSpace    = r->vkSurfaceFormat.colorSpace;
      info.imageExtent.width  = r->swapchainWidth;
      info.imageExtent.height = r->swapchainHeight;
      info.imageArrayLayers   = 1;
      info.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      if (r->vkGraphicsQueueFamilyIndex == r->vkPresentQueueFamilyIndex) {
         info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
         info.queueFamilyIndexCount = 1;
         info.pQueueFamilyIndices   = queueFamilyIndices;
      } else {
         info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
         info.queueFamilyIndexCount = SF_SIZE(queueFamilyIndices);
         info.pQueueFamilyIndices   = queueFamilyIndices;
      }
      info.preTransform   = capabilities.currentTransform;
      info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      info.presentMode    = r->vkPresentMode;
      info.clipped        = VK_TRUE;
      info.oldSwapchain   = VK_NULL_HANDLE;

      if (!SF_VULKAN_CHECK(vkCreateSwapchainKHR(r->vkDevice, &info, r->vkAllocationCallbacks, &r->vkSwapchain)))
         goto error;
   }

   if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vkDevice, r->vkSwapchain, &r->vkSwapchainImageCount, NULL)))
      goto error;

   if (SF_SIZE(r->vkSwapchainImages) < r->vkSwapchainImageCount)
      goto error;

   if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vkDevice, r->vkSwapchain, &r->vkSwapchainImageCount, r->vkSwapchainImages)))
      goto error;

   r->swapchainRenderTargetCount = r->vkSwapchainImageCount;
   for (U32 i = 0; i < r->swapchainRenderTargetCount; ++i) {
      r->swapchainRenderTargets[i] = sfGraphicsVulkanCreateRenderTarget(
          r, r->swapchainWidth, r->swapchainHeight, r->swapchainSampleCount, r->swapchainColorFormat, r->swapchainDepthStencilFormat, 1,
          &r->swapchainColorClearValue, &r->swapchainDepthStencilClearValue, &r->vkSwapchainImages[i]);
      if (!r->swapchainRenderTargets[i].value)
         goto error;
   }

   return;

error:
   sfGraphicsVulkanDestroySwapchainResources(r);
}

SF_EXTERNAL void sfGraphicsDestroyCommandBuffer(SFGraphicsRenderer *r, SFHandle handle) {
   SFGraphicsCommandBuffer *commandBuffer = (SFGraphicsCommandBuffer *)handle.value;

   if (!r || !commandBuffer)
      return;

   sfQueueRemove(&commandBuffer->queue);
   sfQueueInsertHead(&r->commandBufferPool.free, &commandBuffer->queue);

   if (r->vkDevice) {
      if (commandBuffer->vkCommandPool) {
         vkFreeCommandBuffers(r->vkDevice, commandBuffer->vkCommandPool, 1, &commandBuffer->vkCommandBuffer);
         commandBuffer->vkCommandBuffer = VK_NULL_HANDLE;

         vkDestroyCommandPool(r->vkDevice, commandBuffer->vkCommandPool, r->vkAllocationCallbacks);
         commandBuffer->vkCommandPool = VK_NULL_HANDLE;
      }
   }
}

SF_EXTERNAL SFHandle sfGraphicsCreateCommandBuffer(SFGraphicsRenderer *r, SFBool transient) {
   if (!r || !r->vkDevice)
      return SF_NULL_HANDLE;

   SFGraphicsCommandBuffer *commandBuffer = sfGraphicsGetOrAllocateCommandBufferFromResourcePool(r, &r->commandBufferPool);
   if (!commandBuffer)
      return SF_NULL_HANDLE;

   VkCommandPoolCreateInfo commandPoolInfo = {0};
   commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   commandPoolInfo.pNext                   = NULL;
   commandPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   if (transient)
      commandPoolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
   // FIXME(samuel): for now manually set the graphics queue as the family index. Later we should pass the
   // requried queue handle
   commandPoolInfo.queueFamilyIndex = r->vkGraphicsQueueFamilyIndex;

   if (!SF_VULKAN_CHECK(vkCreateCommandPool(r->vkDevice, &commandPoolInfo, r->vkAllocationCallbacks, &commandBuffer->vkCommandPool)))
      goto error;

   VkCommandBufferAllocateInfo commandBufferInfo = {0};
   commandBufferInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   commandBufferInfo.pNext                       = NULL;
   commandBufferInfo.commandPool                 = commandBuffer->vkCommandPool;
   commandBufferInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   commandBufferInfo.commandBufferCount          = 1;

   if (!SF_VULKAN_CHECK(vkAllocateCommandBuffers(r->vkDevice, &commandBufferInfo, &commandBuffer->vkCommandBuffer)))
      goto error;

   return SF_AS_HANDLE(commandBuffer);

error:
   sfGraphicsDestroyCommandBuffer(r, SF_AS_HANDLE(commandBuffer));
   return SF_NULL_HANDLE;
}

SF_INTERNAL VkShaderModule sfGraphicsVulkanCreateShader(SFGraphicsRenderer *r, U32 codeSize, void const *code) {
   VkShaderModule shader = VK_NULL_HANDLE;

   if (!r || !r->vkDevice)
      return VK_NULL_HANDLE;

   VkShaderModuleCreateInfo info = {0};

   info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   info.pNext    = NULL;
   info.flags    = 0;
   info.codeSize = codeSize;
   info.pCode    = (U32 const *)code;

   if (!SF_VULKAN_CHECK(vkCreateShaderModule(r->vkDevice, &info, r->vkAllocationCallbacks, &shader)))
      return VK_NULL_HANDLE;

   return shader;
}

SF_EXTERNAL void sfGraphicsDestroyPipeline(SFGraphicsRenderer *r, SFHandle handle) {
   SFGraphicsPipeline *pipeline = (SFGraphicsPipeline *)handle.value;
   if (!r || !pipeline)
      return;

   sfQueueRemove(&pipeline->queue);
   sfQueueInsertHead(&r->pipelinePool.free, &pipeline->queue);

   if (r->vkDevice) {

      if (pipeline->vkPipeline) {
         vkDestroyPipeline(r->vkDevice, pipeline->vkPipeline, r->vkAllocationCallbacks);
         pipeline->vkPipeline = VK_NULL_HANDLE;
      }

      if (pipeline->vkPipelineLayout) {
         vkDestroyPipelineLayout(r->vkDevice, pipeline->vkPipelineLayout, r->vkAllocationCallbacks);
         pipeline->vkPipelineLayout = VK_NULL_HANDLE;
      }

      if (pipeline->vkVertexShader) {
         vkDestroyShaderModule(r->vkDevice, pipeline->vkVertexShader, r->vkAllocationCallbacks);
         pipeline->vkVertexShader = VK_NULL_HANDLE;
      }

      if (pipeline->vkFragmentShader) {
         vkDestroyShaderModule(r->vkDevice, pipeline->vkFragmentShader, r->vkAllocationCallbacks);
         pipeline->vkFragmentShader = VK_NULL_HANDLE;
      }
   }
}

SF_INTERNAL U64 sfGraphicsStrideFromFormat(SFGraphicsFormat format) {
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

SF_EXTERNAL SFHandle sfGraphicsCreatePipeline(SFGraphicsRenderer *r, SFGraphicsVertexLayout *vertexLayout,
                                              SFGraphicsDescriptorSetLayout *descriptorSetLayout, U32 vertexCodeSize,
                                              void const *vertexCode, uint32_t fragmentCodeSize, void const *fragmentCode) {
   if (!r)
      return SF_NULL_HANDLE;

   SFGraphicsPipeline *pipeline = sfGraphicsGetOrAllocatePipelineFromResourcePool(&r->arena, &r->pipelinePool);
   if (!pipeline)
      return SF_NULL_HANDLE;

   pipeline->vkVertexShader = sfGraphicsVulkanCreateShader(r, vertexCodeSize, vertexCode);
   if (!pipeline->vkVertexShader)
      goto error;

   pipeline->vkFragmentShader = sfGraphicsVulkanCreateShader(r, fragmentCodeSize, fragmentCode);
   if (!pipeline->vkFragmentShader)
      goto error;

   return SF_AS_HANDLE(pipeline);

error:
   sfGraphicsDestroyPipeline(r, SF_AS_HANDLE(pipeline));
   return SF_NULL_HANDLE;
}

#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)

typedef struct SFGraphicsDeviceList {
   U32               size;
   VkPhysicalDevice *data;
} SFGraphicsDeviceList;

SF_INTERNAL SFGraphicsDeviceList sfGraphicsCreateDeviceList(SFArena *arena, SFGraphicsRenderer *r) {
   SFGraphicsDeviceList result = {0};

   if (SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(r->vkInstance, &result.size, NULL))) {
      result.data = sfArenaAllocate(arena, result.size * sizeof(result.data));
      if (result.data) {
         vkEnumeratePhysicalDevices(r->vkInstance, &result.size, result.data);
      }
   }

   return result;
}

SF_EXTERNAL SFGraphicsRenderer *sfGraphicsCreateRenderer(SFArena *arena, SFGraphicsRendererDescription *description) {
   SFGraphicsRenderer *r = sfArenaAllocate(arena, sizeof(SFGraphicsRenderer));

   if (!r)
      return NULL;

   r->arena = sfArenaScratch(arena, 1024 * 128);
   if (!r->arena.data)
      return NULL;

   {
      SFString          appName  = sfStringNullTerminate(&r->arena, description->applicationName);
      VkApplicationInfo appInfo  = {0};
      appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pNext              = NULL;
      appInfo.pApplicationName   = appName.data;
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pEngineName        = NULL;
      appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion         = VK_MAKE_VERSION(1, 0, 0);

      VkInstanceCreateInfo info = {0};
      info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      info.pNext                = NULL;
      info.flags                = 0;
#ifdef __APPLE__
      info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
      info.pApplicationInfo        = &appInfo;
      info.enabledLayerCount       = description->vkInstanceLayerCount;
      info.ppEnabledLayerNames     = description->vkInstanceLayers;
      info.enabledExtensionCount   = description->vkInstanceExtensionCount;
      info.ppEnabledExtensionNames = description->vkInstanceExtensions;

      if (!SF_VULKAN_CHECK(vkCreateInstance(&info, r->vkAllocationCallbacks, &r->vkInstance)))
         goto error;
   }

   {
      r->vkCreateDebugUtilsMessengerEXT  = SF_VULKAN_PROC(vkCreateDebugUtilsMessengerEXT, r->vkInstance);
      r->vkDestroyDebugUtilsMessengerEXT = SF_VULKAN_PROC(vkDestroyDebugUtilsMessengerEXT, r->vkInstance);

      if (!r->vkCreateDebugUtilsMessengerEXT || !r->vkDestroyDebugUtilsMessengerEXT)
         goto error;
   }

   {
      PFN_vkCreateDebugUtilsMessengerEXT create = r->vkCreateDebugUtilsMessengerEXT;

      VkDebugUtilsMessengerCreateInfoEXT info = {0};
      info.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      info.pNext                              = NULL;
      info.flags                              = 0;
      info.messageSeverity                    = 0;
      info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
      info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
      info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
      info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      info.messageType = 0;
      info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
      info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
      info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      info.pfnUserCallback = sfGraphicsVulkanLog;
      info.pUserData       = NULL;

      if (!SF_VULKAN_CHECK(create(r->vkInstance, &info, r->vkAllocationCallbacks, &r->vkValidationMessenger)))
         goto error;
   }

   r->swapchainWidth  = description->width;
   r->swapchainHeight = description->height;

   description->createVulkanSurface(r);
   if (!r->vkSurface)
      goto error;

   {
      SFGraphicsDeviceList deviceList = sfGraphicsCreateDeviceList(&r->arena, r);

      if (!deviceList.data || !deviceList.size)
         goto error;

      for (U32 i = 0; i < deviceList.size; ++i) {
         r->vkPhysicalDevice = deviceList.data[i];

         sfGraphicsVulkanFindSuitableQueueFamilyIndices(&r->arena, r);
         if (!sfGraphicsAreQueueFamilyIndicesValid(r)) {
            r->vkPhysicalDevice = VK_NULL_HANDLE;
            continue;
         }

         if (!sfGraphicsCheckDeviceSwapchainSupport(r)) {
            r->vkPhysicalDevice = VK_NULL_HANDLE;
            continue;
         }

         if (!sfGraphicsCheckDeviceExtensionSupport(&r->arena, r, description)) {
            r->vkPhysicalDevice = VK_NULL_HANDLE;
            continue;
         }

         break;
      }

      if (!r->vkPhysicalDevice)
         goto error;
   }

   {
      VkPhysicalDeviceFeatures features = {0};
      vkGetPhysicalDeviceFeatures(r->vkPhysicalDevice, &features);

      float                   priority     = 1.0F;
      VkDeviceQueueCreateInfo queueInfo[2] = {0};

      queueInfo[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo[0].pNext            = NULL;
      queueInfo[0].flags            = 0;
      queueInfo[0].queueFamilyIndex = r->vkGraphicsQueueFamilyIndex;
      queueInfo[0].queueCount       = 1;
      queueInfo[0].pQueuePriorities = &priority;

      queueInfo[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo[1].pNext            = NULL;
      queueInfo[1].flags            = 0;
      queueInfo[1].queueFamilyIndex = r->vkPresentQueueFamilyIndex;
      queueInfo[1].queueCount       = 1;
      queueInfo[1].pQueuePriorities = &priority;

      VkDeviceCreateInfo info = {0};
      info.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      info.pNext              = NULL;
      info.flags              = 0;
      if (r->vkGraphicsQueueFamilyIndex == r->vkPresentQueueFamilyIndex)
         info.queueCreateInfoCount = 1;
      else
         info.queueCreateInfoCount = SF_SIZE(queueInfo);
      info.pQueueCreateInfos       = queueInfo;
      info.enabledLayerCount       = 0;
      info.ppEnabledLayerNames     = NULL;
      info.enabledExtensionCount   = description->vkDeviceExtensionCount;
      info.ppEnabledExtensionNames = description->vkDeviceExtensions;
      info.pEnabledFeatures        = &features;

      if (!SF_VULKAN_CHECK(vkCreateDevice(r->vkPhysicalDevice, &info, r->vkAllocationCallbacks, &r->vkDevice)))
         goto error;

      vkGetDeviceQueue(r->vkDevice, r->vkGraphicsQueueFamilyIndex, 0, &r->vkGraphicsQueue);
      vkGetDeviceQueue(r->vkDevice, r->vkPresentQueueFamilyIndex, 0, &r->vkPresentQueue);
   }

   {
      VkSurfaceFormatKHR          requestedFormat   = {.format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
      SFGraphicsSurfaceFormatList surfaceFormatList = sfGraphicsCreateSurfaceFormatList(&r->arena, &r->vkPhysicalDevice, r->vkSurface);

      for (U32 i = 0; i < surfaceFormatList.size; ++i) {
         VkSurfaceFormatKHR format = surfaceFormatList.data[i];

         if (requestedFormat.format == format.format || requestedFormat.colorSpace == format.colorSpace) {
            r->vkSurfaceFormat = requestedFormat;
            break;
         }
      }

      if (r->vkSurfaceFormat.format == VK_FORMAT_UNDEFINED)
         goto error;
   }

   {
      VkPresentModeKHR          requestedPresentMode = description->enableVSYNC ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
      SFGraphicsPresentModeList presentModeList      = sfGraphicsCreatePresentModeList(&r->arena, &r->vkPhysicalDevice, r->vkSurface);

      r->vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;

      for (U32 i = 0; i < presentModeList.size; ++i) {
         VkPresentModeKHR presentMode = presentModeList.data[i];

         if (requestedPresentMode == presentMode) {
            r->vkPresentMode = requestedPresentMode;
            break;
         }
      }

      if (r->vkPresentMode == VK_PRESENT_MODE_FIFO_KHR)
         r->enableVSYNC = SF_TRUE;
      else
         r->enableVSYNC = SF_FALSE;
   }

   {
      if (sfGraphicsTestFormatFeatures(
              r->vkPhysicalDevice, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
         r->vkDepthStencilFormat = VK_FORMAT_D32_SFLOAT;
      else if (sfGraphicsTestFormatFeatures(r->vkPhysicalDevice, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
         r->vkDepthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
      else if (sfGraphicsTestFormatFeatures(r->vkPhysicalDevice, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
         r->vkDepthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
   }

   {
      VkPhysicalDeviceProperties properties   = {0};
      VkSampleCountFlags         sampleCounts = 0;

      vkGetPhysicalDeviceProperties(r->vkPhysicalDevice, &properties);
      sampleCounts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

      if (0)
         (void)0;
      else if (sampleCounts & VK_SAMPLE_COUNT_2_BIT)
         r->vkSamples = VK_SAMPLE_COUNT_2_BIT;
      else
         goto error;
   }

   sfGraphicsVulkanCreateSwapchainResources(r);
   if (!r->vkSwapchain)
      goto error;

   r->mainCommandBufferCount = SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT;
   for (U32 i = 0; r->mainCommandBufferCount; ++i)
      r->mainCommandBuffers[i] = sfGraphicsCreateCommandBuffer(r, SF_FALSE);

   r->vkImageAcquiredSemaphoreCount = r->mainCommandBufferCount;
   for (U32 i = 0; i < r->vkImageAcquiredSemaphoreCount; ++i) {
      VkSemaphoreCreateInfo info = {0};

      info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      info.pNext = NULL;
      info.flags = 0;

      if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vkDevice, &info, r->vkAllocationCallbacks, &r->vkImageAcquiredSemaphores[i])))
         goto error;
   }

   r->vkInFlightFenceCount = r->mainCommandBufferCount;
   for (U32 i = 0; i < r->vkInFlightFenceCount; ++i) {
      VkFenceCreateInfo info = {0};

      info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      info.pNext = NULL;
      info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

      if (!SF_VULKAN_CHECK(vkCreateFence(r->vkDevice, &info, r->vkAllocationCallbacks, &r->vkInFlightFences[i])))
         goto error;
   }

   r->vkDrawCompleteSemaphoreCount = r->vkSwapchainImageCount;
   for (U32 i = 0; i < r->vkDrawCompleteSemaphoreCount; ++i) {
      VkSemaphoreCreateInfo info = {0};

      info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      info.pNext = NULL;
      info.flags = 0;

      if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vkDevice, &info, r->vkAllocationCallbacks, &r->vkDrawCompleteSemaphores[i])))
         goto error;
   }

   return r;

error:
   sfGraphicsDestroyRenderer(r);
   return NULL;
}

SF_EXTERNAL void sfGraphicsDestroyRenderer(SFGraphicsRenderer *r) {
   if (!r)
      return;

   if (r->vkDevice) {
      vkDeviceWaitIdle(r->vkDevice);

      for (U32 i = 0; i < SF_SIZE(r->vkDrawCompleteSemaphores); ++i) {
         VkSemaphore semaphore = r->vkDrawCompleteSemaphores[i];

         if (!semaphore)
            continue;

         vkDestroySemaphore(r->vkDevice, semaphore, r->vkAllocationCallbacks);
         r->vkDrawCompleteSemaphores[i] = VK_NULL_HANDLE;
      }
      r->vkDrawCompleteSemaphoreCount = 0;

      for (U32 i = 0; i < SF_SIZE(r->vkInFlightFences); ++i) {
         VkFence fence = r->vkInFlightFences[i];

         if (!fence)
            continue;

         vkDestroyFence(r->vkDevice, fence, r->vkAllocationCallbacks);
         r->vkInFlightFences[i] = VK_NULL_HANDLE;
      }
      r->vkInFlightFenceCount = 0;

      for (U32 i = 0; i < SF_SIZE(r->vkImageAcquiredSemaphores); ++i) {
         VkSemaphore semaphore = r->vkImageAcquiredSemaphores[i];

         if (!semaphore)
            continue;

         vkDestroySemaphore(r->vkDevice, semaphore, r->vkAllocationCallbacks);
         r->vkImageAcquiredSemaphores[i] = VK_NULL_HANDLE;
      }
      r->vkImageAcquiredSemaphoreCount = 0;

      for (U32 i = 0; i < SF_SIZE(r->mainCommandBuffers); ++i) {
         SFHandle commandBuffer = r->mainCommandBuffers[i];

         if (!commandBuffer.value)
            continue;

         sfGraphicsDestroyCommandBuffer(r, commandBuffer);
         r->mainCommandBuffers[i] = SF_NULL_HANDLE;
      }
      r->mainCommandBufferCount = 0;

      sfGraphicsDestroySwapchainResources(r);

      vkDestroyDevice(r->vkDevice, r->vkAllocationCallbacks);
      r->vkDevice = VK_NULL_HANDLE;
   }

   if (r->vkInstance) {

      if (r->vkSurface) {
         vkDestroySurfaceKHR(r->vkInstance, r->vkSurface, r->vkAllocationCallbacks);
         r->vkSurface = VK_NULL_HANDLE;
      }

      if (r->vkDestroyDebugUtilsMessengerEXT && r->vkValidationMessenger) {
         r->vkDestroyDebugUtilsMessengerEXT(r->vkInstance, r->vkValidationMessenger, r->vkAllocationCallbacks);
         r->vkValidationMessenger = VK_NULL_HANDLE;
      }

      vkDestroyInstance(r->vkInstance, r->vkAllocationCallbacks);
      r->vkInstance = VK_NULL_HANDLE;
   }

   sfArenaClear(&r->arena);
}

SF_EXTERNAL void sfGraphicsBeginFrame(SFGraphicsRenderer *r) {
   // https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html

   U32     currentFrameIndex    = r->currentFrameIndex;
   VkFence currentInFlightFence = r->vkInFlightFences[currentFrameIndex];
   vkWaitForFences(r->vkDevice, 1, &currentInFlightFence, VK_TRUE, (uint64_t)-1);
   vkResetFences(r->vkDevice, 1, &currentInFlightFence);

   SFGraphicsCommandBuffer *currentCommandBuffer = (SFGraphicsCommandBuffer *)r->mainCommandBuffers[currentFrameIndex].value;
   vkResetCommandPool(r->vkDevice, currentCommandBuffer->vkCommandPool, 0);

   U32         currentImageIndex             = 0;
   VkSemaphore currentImageAcquiredSemaphore = r->vkImageAcquiredSemaphores[currentFrameIndex];
   VkResult    result                        = vkAcquireNextImageKHR(
       r->vkDevice, r->vkSwapchain, (U64)-1, currentImageAcquiredSemaphore, VK_NULL_HANDLE, &r->currentSwapchainImageIndex);
   if (VK_ERROR_OUT_OF_DATE_KHR != result && VK_ERROR_SURFACE_LOST_KHR != result) {
      currentImageIndex = r->currentSwapchainImageIndex;
   } else {
      SF_VULKAN_CHECK(result);
      vkDeviceWaitIdle(r->vkDevice);
      sfGraphicsVulkanDestroySwapchainResources(r);
      sfGraphicsVulkanCreateSwapchainResources(r);
      r->swapchainSkipEndFrame = SF_TRUE;
      return;
   }

   VkCommandBufferBeginInfo commandBufferBeginInfo = {0};

   commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   commandBufferBeginInfo.pNext            = NULL;
   commandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
   commandBufferBeginInfo.pInheritanceInfo = NULL;

   vkBeginCommandBuffer(currentCommandBuffer->vkCommandBuffer, &commandBufferBeginInfo);

   VkClearValue clearValues[2] = {0};

   clearValues[0].color.float32[0]     = r->swapchainColorClearValue.data.rgba.r;
   clearValues[0].color.float32[1]     = r->swapchainColorClearValue.data.rgba.g;
   clearValues[0].color.float32[2]     = r->swapchainColorClearValue.data.rgba.b;
   clearValues[0].color.float32[3]     = r->swapchainColorClearValue.data.rgba.a;
   clearValues[1].depthStencil.depth   = r->swapchainDepthStencilClearValue.data.depthStencil.depth;   // 1.0F
   clearValues[1].depthStencil.stencil = r->swapchainDepthStencilClearValue.data.depthStencil.stencil; // 0.0F

   SFGraphicsRenderTarget *currentRenderTarget = (SFGraphicsRenderTarget *)r->swapchainRenderTargets[currentImageIndex].value;

   VkRenderPassBeginInfo renderPassBeginInfo = {0};

   renderPassBeginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassBeginInfo.pNext                    = NULL;
   renderPassBeginInfo.renderPass               = currentRenderTarget->vkRenderPass;
   renderPassBeginInfo.framebuffer              = currentRenderTarget->vkFramebuffer;
   renderPassBeginInfo.renderArea.offset.x      = 0;
   renderPassBeginInfo.renderArea.offset.y      = 0;
   renderPassBeginInfo.renderArea.extent.width  = currentRenderTarget->width;
   renderPassBeginInfo.renderArea.extent.height = currentRenderTarget->height;

   renderPassBeginInfo.clearValueCount = SF_SIZE(clearValues);
   renderPassBeginInfo.pClearValues    = clearValues;

   vkCmdBeginRenderPass(currentCommandBuffer->vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

   VkViewport viewport = {0};

   viewport.x        = 0;
   viewport.y        = 0;
   viewport.width    = currentRenderTarget->width;
   viewport.height   = currentRenderTarget->height;
   viewport.minDepth = 0.0F;
   viewport.maxDepth = .0F;

   vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);

   VkRect2D scissor = {0};

   scissor.offset.x      = 0;
   scissor.offset.y      = 0;
   scissor.extent.width  = currentRenderTarget->width;
   scissor.extent.height = currentRenderTarget->height;

   vkCmdSetScissor(currentCommandBuffer->vkCommandBuffer, 0, 1, &scissor);
}

SF_EXTERNAL void sfGraphicsEndFrame(SFGraphicsRenderer *r) {
   VkResult         result       = {0};

   if (r->swapchainSkipEndFrame) {
      r->swapchainSkipEndFrame = SF_FALSE;
      return;
   }

   U32                      currentFrameIndex    = r->currentFrameIndex;
   SFGraphicsCommandBuffer *currentCommandBuffer = (SFGraphicsCommandBuffer *)r->mainCommandBuffers[currentFrameIndex].value;

   vkCmdEndRenderPass(currentCommandBuffer->vkCommandBuffer);
   vkEndCommandBuffer(currentCommandBuffer->vkCommandBuffer);

   U32                     currentImageIndex            = r->currentSwapchainImageIndex;
   VkSemaphore             currentDrawCompleteSemaphore = r->vkDrawCompleteSemaphores[currentImageIndex];
   VkSemaphore             currentImageAcquiredSemaphore = r->vkImageAcquiredSemaphores[currentFrameIndex];
   VkFence                 currentInFlightFence         = r->vkInFlightFences[currentFrameIndex];
   VkPipelineStageFlagBits stage                        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

   VkSubmitInfo submitInfo = {0};

   submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.pNext                = NULL;
   submitInfo.waitSemaphoreCount   = 1;
   submitInfo.pWaitSemaphores      = &currentImageAcquiredSemaphore;
   submitInfo.pWaitDstStageMask    = &stage;
   submitInfo.commandBufferCount   = 1;
   submitInfo.pCommandBuffers      = &currentCommandBuffer->vkCommandBuffer;
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores    = &currentDrawCompleteSemaphore;

   vkQueueSubmit(r->vkGraphicsQueue, 1, &submitInfo, currentInFlightFence);


    VkPresentInfoKHR presentInfo = {0};

   presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.pNext              = NULL;
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores    = &currentDrawCompleteSemaphore;
   presentInfo.swapchainCount     = 1;
   presentInfo.pSwapchains        = &r->vkSwapchain;
   presentInfo.pImageIndices      = &currentImageIndex;
   presentInfo.pResults           = NULL;

   VkResult result = vkQueuePresentKHR(r->vkPresentQueue, &presentInfo);
   if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      SF_VULKAN_CHECK(result);
      vkDeviceWaitIdle(r->vkDevice);
      sfGraphicsVulkanDestroySwapchainResources(r);
      sfGraphicsVulkanCreateSwapchainResources(r);
   }
}
