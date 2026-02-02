#include "sf_graphics.h"
#include "vulkan/vulkan_core.h"

#include <GLFW/glfw3.h>
#include <sf.h>

#define SF_VULKAN_CHECK(e) ((e) == VK_SUCCESS)
#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)

SF_INTERNAL sf_bool sf_vulkan_validate_queue_family_indices(sf_graphics_renderer *r) {
   return r->graphics_queue.vk_queue_family_index != (uint32_t)-1 && r->present_queue.vk_queue_family_index != (uint32_t)-1;
}

typedef struct sf_vulkan_queue_family_property_list {
   uint32_t size;
   VkQueueFamilyProperties *data;
} sf_vulkan_queue_family_property_list;

SF_INTERNAL sf_vulkan_queue_family_property_list sf_vulkan_create_queue_family_property_list(sf_arena *arena, sf_graphics_renderer *r) {
   sf_vulkan_queue_family_property_list result = {0};

   vkGetPhysicalDeviceQueueFamilyProperties(r->vk_physical_device, &result.size, NULL);
   result.data = sf_allocate(arena, result.size * sizeof(*result.data));
   if (result.data)
      vkGetPhysicalDeviceQueueFamilyProperties(r->vk_physical_device, &result.size, result.data);

   return result;
}

SF_INTERNAL void sf_vulkan_find_queue_family_indices(sf_graphics_renderer *r) {
   sf_vulkan_queue_family_property_list list = sf_vulkan_create_queue_family_property_list(&r->arena, r);

   r->graphics_queue.vk_queue_family_index = (uint32_t)-1;
   r->present_queue.vk_queue_family_index = (uint32_t)-1;

   for (uint32_t i = 0; i < list.size && !sf_vulkan_validate_queue_family_indices(r); ++i) {
      VkBool32 supports_surface = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(r->vk_physical_device, i, r->vk_surface, &supports_surface);

      if (supports_surface)
         r->present_queue.vk_queue_family_index = i;

      if (list.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
         r->graphics_queue.vk_queue_family_index = i;
   }
}

typedef struct sf_vulkan_extension_property_list {
   uint32_t size;
   VkExtensionProperties *data;
} sf_vulkan_extension_property_list;

SF_INTERNAL sf_vulkan_extension_property_list sf_vulkan_create_extension_property_list(sf_arena *arena, sf_graphics_renderer *r) {
   sf_vulkan_extension_property_list result = {0};

   if (SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(r->vk_physical_device, NULL, &result.size, NULL))) {
      result.data = sf_allocate(arena, result.size * sizeof(*result.data));
      if (result.data) {
         SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(r->vk_physical_device, NULL, &result.size, result.data));
      }
   }

   return result;
}

SF_INTERNAL sf_bool sf_vulkan_check_physical_device_extension_support(sf_graphics_renderer *r) {
   sf_bool found_all_extensions = SF_TRUE;
   sf_vulkan_extension_property_list const list = sf_vulkan_create_extension_property_list(&r->arena, r);

   for (uint32_t i = 0; i < r->vk_device_extension_count && found_all_extensions; ++i) {
      sf_bool found_current_required_extension = SF_FALSE;
      sf_s8 current_required_extension = sf_s8_from_non_literal(r->vk_device_extensions[i]);

      for (uint32_t j = 0; j < list.size && !found_current_required_extension; ++j) {
         sf_s8 current_available_extension = sf_s8_from_non_literal(list.data[j].extensionName);
         found_current_required_extension = sf_s8_compare(current_required_extension, current_available_extension, VK_MAX_EXTENSION_NAME_SIZE);
      }

      found_all_extensions = found_all_extensions && found_current_required_extension;
   }

   return found_all_extensions;
}

SF_INTERNAL sf_bool sf_vulkan_check_physical_device_swapchain_support(sf_graphics_renderer *r) {
   uint32_t surface_format_count = 0, present_mode_count = 0;

   SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vk_physical_device, r->vk_surface, &surface_format_count, NULL));
   SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vk_physical_device, r->vk_surface, &present_mode_count, NULL));

   return surface_format_count && present_mode_count;
}

SF_INTERNAL sf_bool sf_vulkan_check_physical_device_support(sf_graphics_renderer *r) {
   sf_bool result = SF_TRUE;

   if (result && !sf_vulkan_check_physical_device_extension_support(r))
      result = SF_FALSE;

   if (result && !sf_vulkan_check_physical_device_swapchain_support(r))
      result = SF_FALSE;

   sf_vulkan_find_queue_family_indices(r);
   if (result && !sf_vulkan_validate_queue_family_indices(r))
      result = SF_FALSE;

   return result;
}

typedef struct sf_vulkan_physical_device_list {
   uint32_t size;
   VkPhysicalDevice *data;
} sf_vulkan_physical_device_list;

SF_INTERNAL sf_vulkan_physical_device_list sf_vulkan_create_physical_device_list(sf_arena *arena, sf_graphics_renderer *r) {
   sf_vulkan_physical_device_list result = {0};

   if (SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(r->vk_instance, &result.size, NULL))) {
      result.data = sf_allocate(arena, result.size * sizeof(*result.data));
      if (result.data) {
         SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(r->vk_instance, &result.size, NULL));
      }
   }
   return result;
}

SF_INTERNAL void sf_graphics_vulkan_pick_physical_device(sf_graphics_renderer *r) {
   sf_vulkan_physical_device_list list = sf_vulkan_create_physical_device_list(&r->arena, r);

   for (uint32_t i = 0; i < list.size; ++i) {
      r->vk_physical_device = list.data[i];
      if (sf_vulkan_check_physical_device_support(r))
         return;
   }

   r->vk_physical_device = VK_NULL_HANDLE;
}

typedef struct sf_vulkan_surface_format_list {
   uint32_t size;
   VkSurfaceFormatKHR *data;
} sf_vulkan_surface_format_list;

SF_INTERNAL sf_vulkan_surface_format_list sf_vulkan_create_surface_format_list(sf_arena *arena, sf_graphics_renderer *r) {
   sf_vulkan_surface_format_list result = {0};

   if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vk_physical_device, r->vk_surface, &result.size, NULL))) {
      result.data = sf_allocate(arena, result.size * sizeof(*result.data));
      if (result.data) {
         SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vk_physical_device, r->vk_surface, &result.size, result.data));
      }
   }

   return result;
}
typedef struct sf_vulkan_present_mode_list {
   uint32_t size;
   VkPresentModeKHR *data;
} sf_vulkan_present_mode_list;

SF_INTERNAL sf_vulkan_present_mode_list sf_vulkan_create_present_mode_list(sf_arena *arena, sf_graphics_renderer *r) {
   sf_vulkan_present_mode_list result = {0};

   if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vk_physical_device, r->vk_surface, &result.size, NULL))) {
      result.data = sf_allocate(arena, result.size * sizeof(*result.data));
      if (result.data) {
         SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vk_physical_device, r->vk_surface, &result.size, result.data));
      }
   }

   return result;
}

SF_INTERNAL uint32_t sf_graphics_vulkan_find_device_memory_type_index(VkPhysicalDevice device, VkMemoryPropertyFlags memory_properties, uint32_t filter) {
   uint32_t result = (uint32_t)-1;

   if (device) {
      VkPhysicalDeviceMemoryProperties available_properties = {0};
      vkGetPhysicalDeviceMemoryProperties(device, &available_properties);

      for (uint32_t current_type_index = 0; current_type_index < available_properties.memoryTypeCount && result == (uint32_t)-1; ++current_type_index)
         if ((filter & (1 << current_type_index)) && (available_properties.memoryTypes[current_type_index].propertyFlags & memory_properties) == memory_properties)
            result = current_type_index;
   }

   return result;
}

SF_INTERNAL VkDeviceMemory sf_graphics_vulkan_allocate_memory(sf_graphics_renderer *r, VkMemoryPropertyFlags memory_properties, uint32_t filter, uint64_t size) {
   VkDeviceMemory memory = VK_NULL_HANDLE;

   uint32_t memory_type_index = sf_graphics_vulkan_find_device_memory_type_index(r->vk_physical_device, memory_properties, filter);
   if (memory_type_index != (uint32_t)-1) {
      VkMemoryAllocateInfo info = {
         .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .pNext = NULL,
         .allocationSize = size,
         .memoryTypeIndex = memory_type_index
      };

      SF_VULKAN_CHECK(vkAllocateMemory(r->vk_device, &info, r->vk_allocation_callbacks, &memory));
   }

   return memory;
}

SF_INTERNAL VkDeviceMemory sf_graphics_vulkan_allocate_and_bind_memory_to_image(sf_graphics_renderer *r, VkImage image, VkMemoryPropertyFlags properties) {
   VkDeviceMemory memory = VK_NULL_HANDLE;

   if (r && image) {
      VkMemoryRequirements requirements = {0};
      vkGetImageMemoryRequirements(r->vk_device, image, &requirements);

      memory = sf_graphics_vulkan_allocate_memory(r, properties, requirements.memoryTypeBits, requirements.size);
      if (memory && SF_VULKAN_CHECK(vkBindImageMemory(r->vk_device, image, memory, 0))) {
         vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
         memory = VK_NULL_HANDLE;
      }
   }

   return memory;
}

SF_INTERNAL VkDeviceMemory sf_graphics_vulkan_allocate_and_bind_memory_to_buffer(sf_graphics_renderer *r, VkBuffer buffer, VkMemoryPropertyFlags properties) {
   VkDeviceMemory memory = VK_NULL_HANDLE;

   if (r && buffer) {
      VkMemoryRequirements requirements = {0};
      vkGetBufferMemoryRequirements(r->vk_device, buffer, &requirements);

      memory = sf_graphics_vulkan_allocate_memory(r, properties, requirements.memoryTypeBits, requirements.size);
      if (memory && !SF_VULKAN_CHECK(vkBindBufferMemory(r->vk_device, buffer, memory, 0))) {
         vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
         memory = VK_NULL_HANDLE;
      }
   }

   return memory;
}

void sf_graphics_device_wait_idle(sf_graphics_renderer *r) {
   vkDeviceWaitIdle(r->vk_device);
}

SF_INTERNAL void sf_graphics_destroy_swapchain_resources(sf_graphics_renderer *r) {
   sf_graphics_device_wait_idle(r);

   for (uint32_t i = 0; i < SF_SIZE(r->swapchain_render_targets); ++i) {
      sf_graphics_destroy_render_target(r, r->swapchain_render_targets[i]);
      r->swapchain_render_targets[i] = SF_NULL_HANDLE;
   }

   SF_ARRAY_INIT(r->vk_swapchain_images, VK_NULL_HANDLE);
   r->vk_swapchain_image_count = 0;

   if (r->vk_device && r->vk_swapchain) {
      vkDestroySwapchainKHR(r->vk_device, r->vk_swapchain, r->vk_allocation_callbacks);
      r->vk_swapchain = VK_NULL_HANDLE;
   }
}

SF_INTERNAL void sf_graphics_default_init_texture(sf_graphics_texture *t) {
   SF_QUEUE_INIT(&t->queue);
   t->type = SF_GRAPHICS_TEXTURE_TYPE_2D;
   t->sample_count = SF_GRAPHICS_SAMPLE_COUNT_1;
   t->format = SF_GRAPHICS_FORMAT_UNDEFINED;
   t->usage = SF_GRAPHICS_TEXTURE_USAGE_SAMPLED;
   t->width = 0;
   t->height = 0;
   t->depth = 0;
   t->mips = 0;
   t->mapped = 0;
   t->clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;
   t->owns_image = SF_FALSE;
   t->mapped_data = NULL;
   t->vk_layout = VK_IMAGE_LAYOUT_UNDEFINED;
   t->vk_aspect = 0;
   t->vk_image = VK_NULL_HANDLE;
   t->vk_memory = VK_NULL_HANDLE;
   t->vk_image_view = VK_NULL_HANDLE;
   t->vk_sampler = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_texture *sf_graphics_get_or_allocate_texture(sf_graphics_renderer *r) {
   sf_graphics_texture *t = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_texture_queue)) {
      t = sf_allocate(&r->arena, sizeof(*t));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_texture_queue);
      SF_QUEUE_REMOVE(q);
      t = SF_QUEUE_DATA(q, sf_graphics_texture, queue);
   }
   if (t) {
      sf_graphics_default_init_texture(t);
      SF_QUEUE_INSERT_HEAD(&r->texture_queue, &t->queue);
   }
   return t;
}

SF_INTERNAL void sf_graphics_create_swapchain_resources(sf_graphics_renderer *r) {
   uint32_t const queue_family_indices[] = { r->graphics_queue.vk_queue_family_index, r->present_queue.vk_queue_family_index };

   VkSurfaceCapabilitiesKHR capabilities = {0};
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->vk_physical_device, r->vk_surface, &capabilities);

   VkSwapchainCreateInfoKHR const swapchain_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = NULL,
      .flags = 0,
      .surface = r->vk_surface,
      .minImageCount = SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT,
      .imageFormat = r->vk_surface_format,
      .imageColorSpace = r->vk_surface_color_space,
      .imageExtent.width = r->swapchain_width,
      .imageExtent.height = r->swapchain_height,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = queue_family_indices[0] == queue_family_indices[1] ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
      .queueFamilyIndexCount = queue_family_indices[0] == queue_family_indices[1] ? 1 : SF_SIZE(queue_family_indices),
      .pQueueFamilyIndices = queue_family_indices,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = r->vk_present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
   };

   if (SF_VULKAN_CHECK(vkCreateSwapchainKHR(r->vk_device, &swapchain_info, r->vk_allocation_callbacks, &r->vk_swapchain))) {
      if (SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &r->vk_swapchain_image_count, NULL))) {
         if (SF_SIZE(r->vk_swapchain_images) < r->vk_swapchain_image_count) {
            r->swapchain_image_count = r->vk_swapchain_image_count;
            SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &r->vk_swapchain_image_count, r->vk_swapchain_images));
         }
      }
   }

   r->draw_complete_semaphore_count = r->swapchain_image_count;
   for (uint32_t i = 0; i < r->draw_complete_semaphore_count; ++i)
      r->draw_complete_semaphores[i] = sf_graphics_create_semaphore(r);

   r->swapchain_render_target_count = r->swapchain_image_count;
   for (uint32_t i = 0; i < r->swapchain_render_target_count; ++i) {
      sf_graphics_render_target_description const desc = {
         .sample_count = r->sample_count,
         .color_format = r->color_attachment_format,
         .depth_stencil_format = r->depth_stencil_format,
         .width = r->swapchain_width,
         .height = r->swapchain_height,
         .color_attachment_count = 1,
         .color_attachment_clear_values[0] = r->swapchain_color_clear_value,
         .depth_stencil_attachment_clear_value = r->swapchain_depth_stencil_clear_value,
         .vk_not_owned_color_image = r->vk_swapchain_images[i]
      };
      r->swapchain_render_targets[i] = sf_graphics_create_render_target(r, &desc);
   }
}

sf_bool sf_graphics_begin_command(sf_graphics_renderer *r, sf_handle command_buffer) {
   sf_bool result = SF_FALSE;
   if (r && command_buffer) {
      sf_graphics_command_buffer *c = (sf_graphics_command_buffer *)command_buffer;
      VkCommandBufferBeginInfo const info = {
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .pNext = NULL,
         .flags = 0,
         .pInheritanceInfo = NULL
      };
      result = SF_VULKAN_CHECK(vkBeginCommandBuffer(c->vk_command_buffer, &info));
   }
   return result;
}

sf_bool sf_graphics_end_command(sf_graphics_renderer *r, sf_handle command_buffer) {
   sf_bool result = SF_FALSE;
   if (command_buffer) {
      sf_graphics_command_buffer *c = (sf_graphics_command_buffer *)command_buffer;
      result = SF_VULKAN_CHECK(vkEndCommandBuffer(c->vk_command_buffer));
   }
   return result;
}

sf_bool sf_graphics_queue_submit_command(sf_graphics_renderer *r, sf_handle queue, uint32_t command_buffer_count, sf_handle const *command_buffers, uint32_t wait_semaphore_count, sf_handle const *wait_semaphores, uint32_t signal_semaphore_count, sf_handle const *signal_semaphores) {
   sf_bool result = SF_FALSE;
   if (r && queue) {
      sf_graphics_queue *q = (sf_graphics_queue *)queue;
      VkCommandBuffer vk_command_buffers[SF_GRAPHICS_MAX_COMMAND_BUFFER_SUBMIT_COUNT] = {0};
      VkSemaphore vk_wait_semaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};
      VkPipelineStageFlags vk_stage_flags[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};
      VkSemaphore vk_signal_semaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};

      for (uint32_t i = 0; i < command_buffer_count; i++)
         vk_command_buffers[i] = ((sf_graphics_command_buffer const *)command_buffers[i])->vk_command_buffer;

      for (uint32_t i = 0; i < wait_semaphore_count; i++) {
         vk_wait_semaphores[i] = ((sf_graphics_semaphore const *)wait_semaphores[i])->vk_semaphore;
         vk_stage_flags[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
      }

      for (uint32_t i = 0; i < signal_semaphore_count; i++) {
         sf_graphics_semaphore const *s = (sf_graphics_semaphore const *)signal_semaphores[i];
         vk_signal_semaphores[i] = s->vk_semaphore;
      }

      VkSubmitInfo submit_info = {
         .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .pNext = NULL,
         .waitSemaphoreCount = wait_semaphore_count,
         .pWaitSemaphores = vk_wait_semaphores,
         .pWaitDstStageMask = vk_stage_flags,
         .commandBufferCount = command_buffer_count,
         .pCommandBuffers = vk_command_buffers,
         .signalSemaphoreCount = signal_semaphore_count,
         .pSignalSemaphores = vk_signal_semaphores
      };

      result = SF_VULKAN_CHECK(vkQueueSubmit(q->vk_queue, 1, &submit_info, VK_NULL_HANDLE));
   }

   return result;
}

sf_bool sf_graphics_queue_present(sf_graphics_renderer *r, sf_handle queue, uint32_t wait_semaphore_count, sf_handle *wait_semaphores) {
   sf_bool result = SF_FALSE;

   if (r && queue) {
      sf_graphics_queue *q = (sf_graphics_queue *)queue;

      VkSemaphore vk_wait_semaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};
      for (uint32_t i = 0; i < wait_semaphore_count; i++)
         vk_wait_semaphores[i] = ((sf_graphics_semaphore *)wait_semaphores[i])->vk_semaphore;

      VkPresentInfoKHR submit_info = {
         .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .pNext = NULL,
         .waitSemaphoreCount = wait_semaphore_count,
         .pWaitSemaphores = vk_wait_semaphores,
         .swapchainCount = 1,
         .pSwapchains = &r->vk_swapchain,
         .pImageIndices = &r->swapchain_current_image_index,
         .pResults = NULL
      };
      result = SF_VULKAN_CHECK(vkQueuePresentKHR(q->vk_queue, &submit_info));
   }

   return result;
}

sf_bool sf_graphics_queue_wait_idle(sf_handle queue) {
   sf_bool result = SF_FALSE;
   if (queue) {
      sf_graphics_queue *q = (sf_graphics_queue *)queue;
      result = SF_VULKAN_CHECK(vkQueueWaitIdle(q->vk_queue));
   }
   return result;
}

void sf_graphics_destroy_texture(sf_graphics_renderer *r, sf_handle texture) {
   if (r && texture) {
      sf_graphics_texture *t = (sf_graphics_texture *)texture;

      SF_QUEUE_REMOVE(&t->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_texture_queue, &t->queue);

      if (t->vk_image_view) {
         vkDestroyImageView(r->vk_device, t->vk_image_view, r->vk_allocation_callbacks);
         t->vk_image_view = VK_NULL_HANDLE;
      }

      if (t->owns_image) {
         if (t->vk_memory) {
            vkFreeMemory(r->vk_device, t->vk_memory, r->vk_allocation_callbacks);
            t->vk_memory = VK_NULL_HANDLE;
         }
         if (t->vk_image) {
            vkDestroyImage(r->vk_device, t->vk_image, r->vk_allocation_callbacks);
            t->vk_image = VK_NULL_HANDLE;
         }
      }
   }
}

SF_INTERNAL void sf_graphics_default_init_semaphore(sf_graphics_semaphore *s) {
   SF_QUEUE_INIT(&s->queue);
   s->vk_semaphore = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_semaphore *sf_graphics_get_or_allocate_semaphore(sf_graphics_renderer *r) {
   sf_graphics_semaphore *s = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_semaphore_queue)) {
      s = sf_allocate(&r->arena, sizeof(*s));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_semaphore_queue);
      SF_QUEUE_REMOVE(q);
      s = SF_QUEUE_DATA(q, sf_graphics_semaphore, queue);
   }

   if (s) {
      sf_graphics_default_init_semaphore(s);
      SF_QUEUE_INSERT_HEAD(&r->semaphore_queue, &s->queue);
   }

   return s;
}

void sf_graphics_destroy_semaphore(sf_graphics_renderer *r, sf_handle semaphore) {
   if (r && semaphore) {
      sf_graphics_semaphore *s = (sf_graphics_semaphore *)semaphore;

      SF_QUEUE_REMOVE(&s->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_semaphore_queue, &s->queue);

      if (r->vk_device && s->vk_semaphore) {
         vkDestroySemaphore(r->vk_device, s->vk_semaphore, r->vk_allocation_callbacks);
         s->vk_semaphore = VK_NULL_HANDLE;
      }
   }
}

sf_handle sf_graphics_create_semaphore(sf_graphics_renderer *r) {
   sf_graphics_semaphore *s = sf_graphics_get_or_allocate_semaphore(r);

   if (s) {
      VkSemaphoreCreateInfo info = {
         .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
         .pNext = NULL,
         .flags = 0
      };
      if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vk_device, &info, r->vk_allocation_callbacks, &s->vk_semaphore))) {
         sf_graphics_destroy_semaphore(r, SF_AS_HANDLE(s));
         s = SF_NULL_HANDLE;
      }
   }

   return SF_AS_HANDLE(s);
}

SF_INTERNAL void sf_graphics_default_init_fence(sf_graphics_fence *f) {
   SF_QUEUE_INIT(&f->queue);
   f->vk_fence = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_fence *sf_graphics_get_or_allocate_fence(sf_graphics_renderer *r) {
   sf_graphics_fence *f = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_fence_queue)) {
      f = sf_allocate(&r->arena, sizeof(*f));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_fence_queue);
      SF_QUEUE_REMOVE(q);
      f = SF_QUEUE_DATA(q, sf_graphics_fence, queue);
   }

   if (f) {
      sf_graphics_default_init_fence(f);
      SF_QUEUE_INSERT_HEAD(&r->fence_queue, &f->queue);
   }

   return f;
}

void sf_graphics_destroy_fence(sf_graphics_renderer *r, sf_handle fence) {
   if (r && fence) {
      sf_graphics_fence *f = (sf_graphics_fence *)fence;

      SF_QUEUE_REMOVE(&f->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_fence_queue, &f->queue);

      if (r->vk_device && f->vk_fence) {
         vkDestroyFence(r->vk_device, f->vk_fence, r->vk_allocation_callbacks);
         f->vk_fence = VK_NULL_HANDLE;
      }
   }
}

sf_handle sf_graphics_create_fence(sf_graphics_renderer *r) {
   sf_graphics_fence *f = sf_graphics_get_or_allocate_fence(r);
   if (f) {
      VkFenceCreateInfo const info = {
         .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
         .pNext = NULL,
         .flags = VK_FENCE_CREATE_SIGNALED_BIT
      };

      if (!SF_VULKAN_CHECK(vkCreateFence(r->vk_device, &info, r->vk_allocation_callbacks, &f->vk_fence))) {
         sf_graphics_destroy_fence(r, SF_AS_HANDLE(f));
         f = SF_NULL_HANDLE;
      }
   }
   return SF_AS_HANDLE(f);
}

SF_INTERNAL void sf_graphics_default_init_command_pool(sf_graphics_command_pool *p) {
   SF_QUEUE_INIT(&p->queue);
   p->vk_command_pool = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_command_pool *sf_graphics_get_or_allocate_command_pool(sf_graphics_renderer *r) {
   sf_graphics_command_pool *p = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_command_pool_queue)) {
      p = sf_allocate(&r->arena, sizeof(*p));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_command_pool_queue);
      SF_QUEUE_REMOVE(q);
      p = SF_QUEUE_DATA(q, sf_graphics_command_pool, queue);
   }
   if (p) {
      sf_graphics_default_init_command_pool(p);
      SF_QUEUE_INSERT_HEAD(&r->command_pool_queue, &p->queue);
   }
   return p;
}

void sf_graphics_destroy_command_pool(sf_graphics_renderer *r, sf_handle command_pool) {
   if (r && command_pool) {
      sf_graphics_command_pool *p = (sf_graphics_command_pool *)command_pool;

      SF_QUEUE_REMOVE(&p->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_command_pool_queue, &p->queue);

      if (r->vk_device && p->vk_command_pool) {
         vkDestroyCommandPool(r->vk_device, p->vk_command_pool, r->vk_allocation_callbacks);
         p->vk_command_pool = VK_NULL_HANDLE;
      }
   }
}

sf_handle sf_graphics_create_command_pool(sf_graphics_renderer *r, sf_handle queue, sf_bool transient, sf_bool reset) {
   sf_graphics_command_pool *p = sf_graphics_get_or_allocate_command_pool(r);
   if (p) {
      sf_graphics_queue *q = (sf_graphics_queue *)queue;

      VkCommandPoolCreateInfo const info = {
         .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .pNext = NULL,
         .flags = 0 | transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0 | reset ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0,
         .queueFamilyIndex = q->vk_queue_family_index
      };
      if (!SF_VULKAN_CHECK(vkCreateCommandPool(r->vk_device, &info, r->vk_allocation_callbacks, &p->vk_command_pool))) {
         sf_graphics_destroy_command_pool(r, SF_AS_HANDLE(p));
         p = SF_NULL_HANDLE;
      }
   }
   return SF_AS_HANDLE(p);
}

SF_INTERNAL void sf_graphics_default_init_command_buffer(sf_graphics_command_buffer *p) {
   SF_QUEUE_INIT(&p->queue);
   p->vk_command_buffer = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_command_buffer *sf_graphics_get_or_allocate_command_buffer(sf_graphics_renderer *r) {
   sf_graphics_command_buffer *cb = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_command_buffer_queue)) {
      cb = sf_allocate(&r->arena, sizeof(*cb));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_command_buffer_queue);
      SF_QUEUE_REMOVE(q);
      cb = SF_QUEUE_DATA(q, sf_graphics_command_buffer, queue);
   }

   if (cb) {
      sf_graphics_default_init_command_buffer(cb);
      SF_QUEUE_INSERT_HEAD(&r->command_buffer_queue, &cb->queue);
   }
   return cb;
}

void sf_graphics_destroy_command_buffer(sf_graphics_renderer *r, sf_handle command_pool, sf_handle command_buffer) {
   if (r && command_pool && command_buffer) {
      sf_graphics_command_buffer *cb = (sf_graphics_command_buffer *)command_buffer;
      sf_graphics_command_pool *p = (sf_graphics_command_pool *)command_pool;

      SF_QUEUE_REMOVE(&cb->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_command_pool_queue, &cb->queue);

      if (r->vk_device && p->vk_command_pool && cb->vk_command_buffer) {
         vkFreeCommandBuffers(r->vk_device, p->vk_command_pool, 1, &cb->vk_command_buffer);
         cb->vk_command_buffer = VK_NULL_HANDLE;
      }
   }
}

sf_handle sf_graphics_create_command_buffer(sf_graphics_renderer *r, sf_handle command_pool, sf_bool secondary) {
   sf_graphics_command_buffer *cb = sf_graphics_get_or_allocate_command_buffer(r);
   if (cb) {
      sf_graphics_command_pool *p = (sf_graphics_command_pool *)command_pool;
      VkCommandBufferAllocateInfo const info = {
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .pNext = NULL,
         .commandPool = p->vk_command_pool,
         .level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1
      };
      if (!SF_VULKAN_CHECK(vkAllocateCommandBuffers(r->vk_device, &info, &cb->vk_command_buffer))) {
         sf_graphics_destroy_command_buffer(r, command_pool, SF_AS_HANDLE(cb));
         cb = SF_NULL_HANDLE;
      }
   }
   return SF_AS_HANDLE(cb);
}

SF_INTERNAL VkImageType sf_graphics_vulkan_as_vulkan_image_type(enum sf_graphics_texture_type type) {
   switch (type) {
      case SF_GRAPHICS_TEXTURE_TYPE_1D:
         return VK_IMAGE_TYPE_1D;
      case SF_GRAPHICS_TEXTURE_TYPE_2D:
         return VK_IMAGE_TYPE_2D;
      case SF_GRAPHICS_TEXTURE_TYPE_3D:
         return VK_IMAGE_TYPE_3D;
      case SF_GRAPHICS_TEXTURE_TYPE_CUBE:
         return VK_IMAGE_TYPE_2D;
   }
}

SF_INTERNAL VkFormat sf_graphics_vulkan_as_vulkan_format(enum sf_graphics_format format) {
   switch (format) {
      case SF_GRAPHICS_FORMAT_UNDEFINED:
         return VK_FORMAT_UNDEFINED;
      case SF_GRAPHICS_FORMAT_R8_UNORM:
         return VK_FORMAT_R8_UNORM;
      case SF_GRAPHICS_FORMAT_R16_UNORM:
         return VK_FORMAT_R16_UNORM;
      case SF_GRAPHICS_FORMAT_R16_UINT:
         return VK_FORMAT_R16_UINT;
      case SF_GRAPHICS_FORMAT_R16_SFLOAT:
         return VK_FORMAT_R16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32_UINT:
         return VK_FORMAT_R32_UINT;
      case SF_GRAPHICS_FORMAT_R32_SFLOAT:
         return VK_FORMAT_R32_SFLOAT;
      case SF_GRAPHICS_FORMAT_R8G8_UNORM:
         return VK_FORMAT_R8G8_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16_UNORM:
         return VK_FORMAT_R16G16_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:
         return VK_FORMAT_R16G16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32G32_UINT:
         return VK_FORMAT_R32G32_UINT;
      case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:
         return VK_FORMAT_R32G32_SFLOAT;
      case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:
         return VK_FORMAT_R8G8B8_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:
         return VK_FORMAT_R16G16B16_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:
         return VK_FORMAT_R16G16B16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32G32B32_UINT:
         return VK_FORMAT_R32G32B32_UINT;
      case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:
         return VK_FORMAT_R32G32B32_SFLOAT;
      case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:
         return VK_FORMAT_B8G8R8A8_UNORM;
      case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:
         return VK_FORMAT_R8G8B8A8_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:
         return VK_FORMAT_R16G16B16A16_UNORM;
      case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT:
         return VK_FORMAT_R16G16B16A16_SFLOAT;
      case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:
         return VK_FORMAT_R32G32B32A32_UINT;
      case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT:
         return VK_FORMAT_R32G32B32A32_SFLOAT;
      case SF_GRAPHICS_FORMAT_D16_UNORM:
         return VK_FORMAT_D16_UNORM;
      case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32:
         return VK_FORMAT_X8_D24_UNORM_PACK32;
      case SF_GRAPHICS_FORMAT_D32_SFLOAT:
         return VK_FORMAT_D32_SFLOAT;
      case SF_GRAPHICS_FORMAT_S8_UINT:
         return VK_FORMAT_S8_UINT;
      case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:
         return VK_FORMAT_D16_UNORM_S8_UINT;
      case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:
         return VK_FORMAT_D24_UNORM_S8_UINT;
      case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:
         return VK_FORMAT_D32_SFLOAT_S8_UINT;
      default:
         return VK_FORMAT_UNDEFINED;
   }
}

SF_INTERNAL VkImageUsageFlags sf_graphics_vulkan_as_vulkan_usage(enum sf_graphics_texture_usage usage) {
   VkImageUsageFlags result = 0;
   if (SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC == (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC))
      result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

   if (SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST == (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST)) {
      result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   }
   if (SF_GRAPHICS_TEXTURE_USAGE_SAMPLED == (usage & SF_GRAPHICS_TEXTURE_USAGE_SAMPLED))
      result |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

   if (SF_GRAPHICS_TEXTURE_USAGE_STORAGE == (usage & SF_GRAPHICS_TEXTURE_USAGE_STORAGE)) {
      result |= VK_IMAGE_USAGE_STORAGE_BIT;
   }
   if (SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT == (usage & SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT)) {
      result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   if (SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT == (usage & SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT)) {
      result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   }
   if (SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC == (usage & SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC)) {
      result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   if (SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST == (usage & SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST)) {
      result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   return result;
}

SF_INTERNAL VkSampleCountFlagBits sf_graphics_vulkan_as_vulkan_sample_count(enum sf_graphics_sample_count sample_count) {
   switch (sample_count) {
      case SF_GRAPHICS_SAMPLE_COUNT_1:
         return VK_SAMPLE_COUNT_1_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_2:
         return VK_SAMPLE_COUNT_2_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_4:
         return VK_SAMPLE_COUNT_4_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_8:
         return VK_SAMPLE_COUNT_8_BIT;
      case SF_GRAPHICS_SAMPLE_COUNT_16:
         return VK_SAMPLE_COUNT_16_BIT;
   }
}

SF_INTERNAL VkImageViewType sf_graphics_vulkan_as_image_view_type(enum sf_graphics_texture_type type) {
   switch (type) {
      case SF_GRAPHICS_TEXTURE_TYPE_1D:
         return VK_IMAGE_VIEW_TYPE_1D;
      case SF_GRAPHICS_TEXTURE_TYPE_2D:
         return VK_IMAGE_VIEW_TYPE_2D;
      case SF_GRAPHICS_TEXTURE_TYPE_3D:
         return VK_IMAGE_VIEW_TYPE_3D;
      case SF_GRAPHICS_TEXTURE_TYPE_CUBE:
         return VK_IMAGE_VIEW_TYPE_CUBE;
   }
}

SF_INTERNAL VkImageAspectFlags sf_graphics_vulkan_find_aspect_flags(VkFormat format) {
   switch (format) {
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_X8_D24_UNORM_PACK32:
      case VK_FORMAT_D32_SFLOAT:
         return VK_IMAGE_ASPECT_DEPTH_BIT;

      case VK_FORMAT_S8_UINT:
         return VK_IMAGE_ASPECT_STENCIL_BIT;

      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
         return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

      default:
         return VK_IMAGE_ASPECT_COLOR_BIT;
   }
}

sf_handle sf_graphics_create_texture(sf_graphics_renderer *r, sf_graphics_texture_description const *desc) {
   sf_graphics_texture *t = sf_graphics_get_or_allocate_texture(r);
   if (t) {
      t->type = desc->type;
      t->sample_count = desc->sample_count;
      t->format = desc->format;
      t->usage = desc->usage;
      t->width = desc->width;
      t->height = desc->height;
      t->depth = desc->depth;
      t->mips = desc->mips;
      t->mapped = desc->mapped;
      t->clear_value = desc->clear_value;

      if (desc->vk_not_owned_image) {
         t->owns_image = SF_FALSE;
         t->vk_image = desc->vk_not_owned_image;
         t->mapped = SF_FALSE;
      } else {
         VkImageCreateInfo const info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .imageType = sf_graphics_vulkan_as_vulkan_image_type(t->type),
            .format = sf_graphics_vulkan_as_vulkan_format(t->format),
            .extent.width = t->width,
            .extent.height = t->height,
            .extent.depth = t->depth,
            .mipLevels = t->mips,
            .arrayLayers = 1,
            .samples = sf_graphics_vulkan_as_vulkan_sample_count(t->sample_count),
            .tiling = t->mapped ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL,
            .usage = sf_graphics_vulkan_as_vulkan_usage(t->usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = NULL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
         };

         if (SF_VULKAN_CHECK(vkCreateImage(r->vk_device, &info, r->vk_allocation_callbacks, &t->vk_image))) {
            VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | (t->mapped ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0);
            t->vk_memory = sf_graphics_vulkan_allocate_and_bind_memory_to_image(r, t->vk_image, flags);
            if (t->vk_memory && t->mapped) {
               SF_VULKAN_CHECK(vkMapMemory(r->vk_device, t->vk_memory, 0, VK_WHOLE_SIZE, 0, &t->mapped_data));
            }
         }
      }

      if (t->vk_image && (t->owns_image ? !!t->vk_memory : SF_TRUE) && (t->mapped ? !!t->mapped_data : SF_TRUE)) {
         VkImageViewCreateInfo const info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .image = t->vk_image,
            .viewType = sf_graphics_vulkan_as_image_view_type(t->type),
            .format = sf_graphics_vulkan_as_vulkan_format(t->format),
            .components.r = VK_COMPONENT_SWIZZLE_R,
            .components.g = VK_COMPONENT_SWIZZLE_G,
            .components.b = VK_COMPONENT_SWIZZLE_B,
            .components.a = VK_COMPONENT_SWIZZLE_A,
            .subresourceRange = {
               .aspectMask = sf_graphics_vulkan_find_aspect_flags(sf_graphics_vulkan_as_vulkan_format(t->format)),
               .baseMipLevel = 0,
               .levelCount = t->mips,
               .baseArrayLayer = 0,
               .layerCount = 1,
            }
         };

         if (SF_VULKAN_CHECK(vkCreateImageView(r->vk_device, &info, r->vk_allocation_callbacks, &t->vk_image_view))) {
            t->vk_aspect = info.subresourceRange.aspectMask;
            t->vk_layout = (SF_GRAPHICS_TEXTURE_USAGE_STORAGE & t->usage) == SF_GRAPHICS_TEXTURE_USAGE_STORAGE ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
         }
      }


      if (!t->vk_image || !(t->owns_image ? !!t->vk_memory : SF_TRUE) ||  !(t->mapped ? !!t->mapped_data : SF_TRUE) || !t->vk_image_view) {
         sf_graphics_destroy_texture(r, SF_AS_HANDLE(t));
         t = SF_NULL_HANDLE;
      }
   }

   return SF_AS_HANDLE(t);
}

SF_INTERNAL uint32_t sf_graphics_calculate_mip_levels(uint32_t width, uint32_t height) {
   uint32_t result = 1;

   if (width == 0 || height == 0)
      return 0;

   while (width > 1 || height > 1) {
      width >>= 1;
      height >>= 1;
      result++;
   }

   return result;
}

SF_INTERNAL void sf_graphics_default_init_render_target(sf_graphics_render_target *t) {
   SF_QUEUE_INIT(&t->queue);

   t->sample_count = SF_GRAPHICS_SAMPLE_COUNT_1;
   t->color_format = SF_GRAPHICS_FORMAT_UNDEFINED;
   t->depth_stencil_format = SF_GRAPHICS_FORMAT_UNDEFINED;
   t->width = 0;
   t->height = 0;
   t->color_attachment_clear_value_count = 0;

   for (uint32_t i = 0; i < SF_SIZE(t->color_attachment_clear_values); ++i)
      t->color_attachment_clear_values[i].type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;

   t->depth_stencil_attachment_clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;
   t->depth_stencil_attachment = SF_NULL_HANDLE;
   t->depth_stencil_multisampling_attachment = SF_NULL_HANDLE;

   t->color_attachment_count = 0;
   SF_ARRAY_INIT(t->color_attachments, SF_NULL_HANDLE);

   t->color_multisample_attachment_count = 0;
   SF_ARRAY_INIT(t->color_multisample_attachments, SF_NULL_HANDLE);

   t->vk_swapchain_image = VK_NULL_HANDLE;
   t->vk_render_pass = VK_NULL_HANDLE;
   t->vk_framebuffer = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_render_target *sf_graphics_get_or_allocate_render_target(sf_graphics_renderer *r) {
   sf_graphics_render_target *t = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_render_target_queue)) {
      t = sf_allocate(&r->arena, sizeof(*t));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_render_target_queue);
      SF_QUEUE_REMOVE(q);
      t = SF_QUEUE_DATA(q, sf_graphics_render_target, queue);
   }
   if (t) {
      sf_graphics_default_init_render_target(t);
      SF_QUEUE_INSERT_HEAD(&r->render_target_queue, &t->queue);
   }

   return t;
}

void sf_graphics_destroy_render_target(sf_graphics_renderer *r, sf_handle render_target) {
   if (!render_target) {
      sf_graphics_render_target *t = (sf_graphics_render_target *)render_target;

      SF_QUEUE_REMOVE(&t->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_texture_queue, &t->queue);

      if (r->vk_device) {
         if (t->vk_framebuffer) {
            vkDestroyFramebuffer(r->vk_device, t->vk_framebuffer, r->vk_allocation_callbacks);
            t->vk_framebuffer = VK_NULL_HANDLE;
         }

         if (t->vk_render_pass) {
            vkDestroyRenderPass(r->vk_device, t->vk_render_pass, r->vk_allocation_callbacks);
            t->vk_render_pass = VK_NULL_HANDLE;
         }
      }

      sf_graphics_destroy_texture(r, t->depth_stencil_multisampling_attachment);
      sf_graphics_destroy_texture(r, t->depth_stencil_attachment);

      for (uint32_t i = 0; i < SF_SIZE(t->color_multisample_attachments); ++i) {
         sf_graphics_destroy_texture(r, t->color_multisample_attachments[i]);
         t->color_multisample_attachments[i] = SF_NULL_HANDLE;
      }
      t->color_multisample_attachment_count = 0;

      for (uint32_t i = 0; i < SF_SIZE(t->color_attachments); ++i) {
         sf_graphics_destroy_texture(r, t->color_attachments[i]);
         t->color_attachments[i] = SF_NULL_HANDLE;
      }
      t->color_attachment_count = 0;
   }
}

SF_INTERNAL sf_bool sf_graphics_validate_render_target_attachments(sf_graphics_render_target *rt) {
   sf_bool result = SF_TRUE;

   for (uint32_t i = 0; i < rt->color_attachment_count && result; ++i)
      result = !!rt->color_attachments[i];

   for (uint32_t i = 0; i < rt->color_multisample_attachment_count && result; ++i)
      result = !!rt->color_multisample_attachments[i];

   result = result && (rt->depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED ? !!rt->depth_stencil_attachment : SF_TRUE);

   return result;
}

sf_handle sf_graphics_create_render_target(sf_graphics_renderer *r, sf_graphics_render_target_description const *desc) {
   sf_graphics_render_target *t = sf_graphics_get_or_allocate_render_target(r);
   if (t) {
      sf_arena *arena = &r->render_target_arena;

      t->sample_count = desc->sample_count;
      t->color_format = desc->color_format;
      t->depth_stencil_format = desc->depth_stencil_format;
      t->width = desc->width;
      t->height = desc->height;

      t->color_attachment_clear_value_count = desc->color_attachment_count;
      for (uint32_t i = 0; i < t->color_multisample_attachment_count; ++i)
         t->color_attachment_clear_values[i] = desc->color_attachment_clear_values[i];

      t->depth_stencil_attachment_clear_value = desc->depth_stencil_attachment_clear_value;
      t->vk_swapchain_image = desc->vk_not_owned_color_image;

      uint32_t total_attachment_count = 0;
      VkAttachmentDescription *attachment_descriptions = NULL;
      VkAttachmentReference *color_attachment_references = NULL;
      VkAttachmentReference *color_resolve_attachment_references = NULL;
      VkAttachmentReference depth_stencil_reference = {0};
      VkImageView *attachment_image_views = NULL;

      if (t->sample_count == SF_GRAPHICS_SAMPLE_COUNT_1) {
         t->color_attachment_count = desc->color_attachment_count;

         total_attachment_count = t->color_attachment_count + t->depth_stencil_format == SF_GRAPHICS_FORMAT_UNDEFINED ? 0 : 1;
         attachment_descriptions = sf_allocate(arena, total_attachment_count * sizeof(*attachment_descriptions));
         color_attachment_references = sf_allocate(arena, t->color_attachment_count * sizeof(*color_attachment_references));
         attachment_image_views = sf_allocate(arena, total_attachment_count * sizeof(*attachment_image_views));
         if (attachment_descriptions && color_attachment_references && attachment_image_views) {
            for (uint32_t i = 0; i < t->color_attachment_count; ++i) {
               uint32_t color_index = i;
               sf_graphics_clear_value const *clear_value = &desc->color_attachment_clear_values[i];

               sf_graphics_texture_description const texture_desc = {
                  .type = SF_GRAPHICS_TEXTURE_TYPE_2D,
                  .width = t->width,
                  .height = t->height,
                  .sample_count = t->sample_count,
                  .format = t->color_format,
                  .mips = 1,
                  .mapped = SF_FALSE,
                  .usage = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
                  .vk_not_owned_image = t->vk_swapchain_image,
                  .clear_value = *clear_value
               };

               t->color_attachments[i] = sf_graphics_create_texture(r, &texture_desc);
               if (t->color_attachments[i]) {
                  attachment_descriptions[color_index].flags = 0;
                  attachment_descriptions[color_index].format = sf_graphics_vulkan_as_vulkan_format(t->color_format);
                  attachment_descriptions[color_index].samples = VK_SAMPLE_COUNT_1_BIT;
                  attachment_descriptions[color_index].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                  attachment_descriptions[color_index].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                  attachment_descriptions[color_index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                  attachment_descriptions[color_index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                  attachment_descriptions[color_index].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                  if (t->vk_swapchain_image)
                     attachment_descriptions[color_index].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                  else
                     attachment_descriptions[color_index].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                  color_attachment_references[i].attachment = color_index;
                  color_attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                  attachment_image_views[color_index] = ((sf_graphics_texture *)t->color_attachments[i])->vk_image_view;
               }
            }
         }
      } else {
         t->color_attachment_count = desc->color_attachment_count;
         t->color_multisample_attachment_count = desc->color_attachment_count;

         total_attachment_count = 2 * t->color_attachment_count + t->depth_stencil_format == SF_GRAPHICS_FORMAT_UNDEFINED ? 0 : 1;
         attachment_descriptions = sf_allocate(arena, total_attachment_count * sizeof(*attachment_descriptions));
         color_attachment_references = sf_allocate(arena, t->color_attachment_count * sizeof(*color_attachment_references));
         color_resolve_attachment_references = sf_allocate(arena, t->color_attachment_count * sizeof(*color_attachment_references));
         attachment_image_views = sf_allocate(arena, total_attachment_count * sizeof(*attachment_image_views));
         if (attachment_descriptions && color_attachment_references && color_resolve_attachment_references && attachment_image_views) {
            for (uint32_t i = 0; i < t->color_attachment_count; ++i) {
               uint32_t color_index = 2 * i;
               uint32_t resolve_index = color_index + 1;
               sf_graphics_clear_value const *clear_value = &desc->color_attachment_clear_values[i];

               sf_graphics_texture_description const color_texture_desc = {
                  .type = SF_GRAPHICS_TEXTURE_TYPE_2D,
                  .width = t->width,
                  .height = t->height,
                  .sample_count = t->sample_count,
                  .format = t->color_format,
                  .mips = 1,
                  .mapped = SF_FALSE,
                  .usage = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
                  .vk_not_owned_image = t->vk_swapchain_image,
                  .clear_value = *clear_value
               };
               t->color_attachments[i] = sf_graphics_create_texture(r, &color_texture_desc);

               sf_graphics_texture_description const resolve_texture_desc = {
                  .type = SF_GRAPHICS_TEXTURE_TYPE_2D,
                  .width = t->width,
                  .height = t->height,
                  .sample_count = t->sample_count,
                  .format = t->color_format,
                  .mips = 1,
                  .mapped = SF_FALSE,
                  .usage = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
                  .vk_not_owned_image = VK_NULL_HANDLE,
                  .clear_value = *clear_value
               };
               t->color_multisample_attachments[i] = sf_graphics_create_texture(r, &resolve_texture_desc);
               if (t->color_attachments[i] && t->color_multisample_attachments[i]) {
                  attachment_descriptions[color_index].flags = 0;
                  attachment_descriptions[color_index].format = sf_graphics_vulkan_as_vulkan_format(t->color_format);
                  attachment_descriptions[color_index].samples = VK_SAMPLE_COUNT_1_BIT;
                  attachment_descriptions[color_index].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                  attachment_descriptions[color_index].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                  attachment_descriptions[color_index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                  attachment_descriptions[color_index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                  attachment_descriptions[color_index].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                  if (t->vk_swapchain_image)
                     attachment_descriptions[color_index].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                  else
                     attachment_descriptions[color_index].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                  attachment_descriptions[resolve_index].flags = 0;
                  attachment_descriptions[resolve_index].format = sf_graphics_vulkan_as_vulkan_format(t->color_format);
                  attachment_descriptions[resolve_index].samples = sf_graphics_vulkan_as_vulkan_sample_count(t->sample_count);
                  attachment_descriptions[resolve_index].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                  attachment_descriptions[resolve_index].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                  attachment_descriptions[resolve_index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                  attachment_descriptions[resolve_index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                  attachment_descriptions[resolve_index].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                  attachment_descriptions[resolve_index].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                  color_attachment_references[i].attachment = color_index;
                  color_attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                  color_resolve_attachment_references[i].attachment = resolve_index;
                  color_resolve_attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                  attachment_image_views[color_index] = ((sf_graphics_texture *)t->color_attachments[i])->vk_image_view;
                  attachment_image_views[resolve_index] = ((sf_graphics_texture *)t->color_multisample_attachments[i])->vk_image_view;
               }
            }
         }
      }

      if (t->depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED) {
         uint32_t depth_stencil_index = total_attachment_count - 1;

         sf_graphics_texture_description const texture_desc = {
            .type = SF_GRAPHICS_TEXTURE_TYPE_2D,
            .width = t->width,
            .height = t->height,
            .sample_count = t->sample_count,
            .format = t->depth_stencil_format,
            .mips = 1,
            .mapped = SF_FALSE,
            .usage = SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
            .vk_not_owned_image = VK_NULL_HANDLE,
            .clear_value = desc->depth_stencil_attachment_clear_value
         };

         t->depth_stencil_attachment = sf_graphics_create_texture(r, &texture_desc);
         if (t->depth_stencil_attachment) {
            attachment_descriptions[depth_stencil_index].flags = 0;
            attachment_descriptions[depth_stencil_index].format = sf_graphics_vulkan_as_vulkan_format(t->depth_stencil_format);
            attachment_descriptions[depth_stencil_index].samples = sf_graphics_vulkan_as_vulkan_sample_count(t->sample_count);
            attachment_descriptions[depth_stencil_index].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment_descriptions[depth_stencil_index].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment_descriptions[depth_stencil_index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment_descriptions[depth_stencil_index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment_descriptions[depth_stencil_index].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment_descriptions[depth_stencil_index].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depth_stencil_reference.attachment = depth_stencil_index;
            depth_stencil_reference.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

            attachment_image_views[depth_stencil_index] = ((sf_graphics_texture *)t->depth_stencil_attachment)->vk_image_view;
         }
      }

      if (sf_graphics_validate_render_target_attachments(t)) {
         VkRenderPassCreateInfo const render_pass_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .attachmentCount = total_attachment_count,
            .pAttachments = attachment_descriptions,
            .subpassCount = 1,
            .pSubpasses = &(VkSubpassDescription){
               .flags = 0,
               .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .inputAttachmentCount = 0,
               .pInputAttachments = NULL,
               .colorAttachmentCount = t->color_attachment_count,
               .pColorAttachments = color_attachment_references,
               .pResolveAttachments = color_resolve_attachment_references,
               .pDepthStencilAttachment = t->depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED ? &depth_stencil_reference : NULL,
               .preserveAttachmentCount = 0,
               .pPreserveAttachments = NULL },
            .dependencyCount = 1,
            .pDependencies = &(VkSubpassDependency){
               .srcSubpass = 0,
               .dstSubpass = 0,
               .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
            }
         };

         if (SF_VULKAN_CHECK(vkCreateRenderPass(r->vk_device, &render_pass_info, r->vk_allocation_callbacks, &t->vk_render_pass))) {
            VkFramebufferCreateInfo const framebuffer_info = {
               .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
               .pNext = NULL,
               .flags = 0,
               .renderPass = t->vk_render_pass,
               .attachmentCount = total_attachment_count,
               .pAttachments = attachment_image_views,
               .width = t->width,
               .height = t->height,
               .layers = 1
            };
            SF_VULKAN_CHECK(vkCreateFramebuffer(r->vk_device, &framebuffer_info, r->vk_allocation_callbacks, &t->vk_framebuffer));
         }
      }

      if (!sf_graphics_validate_render_target_attachments(t) || !t->vk_render_pass || !t->vk_framebuffer) {
         sf_graphics_destroy_render_target(r, SF_AS_HANDLE(t));
         t = SF_NULL_HANDLE;
      }
   }

   return SF_AS_HANDLE(t);
}

SF_INTERNAL void sf_graphics_default_init_program(sf_graphics_program *p) {
   SF_QUEUE_INIT(&p->queue);
   p->stages = 0;
   p->vk_vertex_shader = VK_NULL_HANDLE;
   p->vk_tesselation_control_shader = VK_NULL_HANDLE;
   p->vk_tesselation_evaluation_shader = VK_NULL_HANDLE;
   p->vk_geometry_shader = VK_NULL_HANDLE;
   p->vk_compute_shader = VK_NULL_HANDLE;
   p->vk_fragment_shader = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_program *sf_graphics_get_or_allocate_program(sf_graphics_renderer *r) {
   sf_graphics_program *p = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_program_queue)) {
      p = sf_allocate(&r->arena, sizeof(*p));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_program_queue);
      SF_QUEUE_REMOVE(q);
      p = SF_QUEUE_DATA(q, sf_graphics_program, queue);
   }

   if (p) {
      sf_graphics_default_init_program(p);
      SF_QUEUE_INSERT_HEAD(&r->program_queue, &p->queue);
   }

   return p;
}

SF_INTERNAL VkShaderModule sf_graphics_vulkan_create_shader_module(sf_graphics_renderer *r, uint32_t code_size_in_bytes, void const *code) {
   VkShaderModule shader = VK_NULL_HANDLE;

   if (r && code_size_in_bytes && code) {
      VkShaderModuleCreateInfo const info = {
         .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .pNext = NULL,
         .flags = 0,
         .codeSize = code_size_in_bytes,
         .pCode = code,
      };

      SF_VULKAN_CHECK(vkCreateShaderModule(r->vk_device, &info, r->vk_allocation_callbacks, &shader));
   }

   return shader;
}

void sf_graphics_destroy_program(sf_graphics_renderer *r, sf_handle program) {
   if (r && program) {
      sf_graphics_program *p = (sf_graphics_program *)program;

      SF_QUEUE_REMOVE(&p->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_program_queue, &p->queue);

      if (r->vk_device) {
         if (p->vk_compute_shader) {
            vkDestroyShaderModule(r->vk_device, p->vk_compute_shader, r->vk_allocation_callbacks);
            p->vk_compute_shader = VK_NULL_HANDLE;
         }

         if (p->vk_fragment_shader) {
            vkDestroyShaderModule(r->vk_device, p->vk_fragment_shader, r->vk_allocation_callbacks);
            p->vk_fragment_shader = VK_NULL_HANDLE;
         }

         if (p->vk_geometry_shader) {
            vkDestroyShaderModule(r->vk_device, p->vk_geometry_shader, r->vk_allocation_callbacks);
            p->vk_geometry_shader = VK_NULL_HANDLE;
         }

         if (p->vk_tesselation_evaluation_shader) {
            vkDestroyShaderModule(r->vk_device, p->vk_tesselation_evaluation_shader, r->vk_allocation_callbacks);
            p->vk_tesselation_evaluation_shader = VK_NULL_HANDLE;
         }

         if (p->vk_tesselation_control_shader) {
            vkDestroyShaderModule(r->vk_device, p->vk_tesselation_control_shader, r->vk_allocation_callbacks);
            p->vk_tesselation_control_shader = VK_NULL_HANDLE;
         }

         if (p->vk_vertex_shader) {
            vkDestroyShaderModule(r->vk_device, p->vk_vertex_shader, r->vk_allocation_callbacks);
            p->vk_vertex_shader = VK_NULL_HANDLE;
         }
      }
   }
}

sf_handle sf_graphics_create_program(sf_graphics_renderer *r, sf_graphics_program_description const *desc) {
   sf_graphics_program *p = sf_graphics_get_or_allocate_program(r);
   if (p) {
      if (desc->vertex_code_size) {
         p->stages |= SF_GRAPHICS_SHADER_STAGE_VERTEX;
         p->vk_vertex_shader = sf_graphics_vulkan_create_shader_module(r, desc->vertex_code_size, desc->vertex_code);
      }

      if (desc->tesselation_control_code_size) {
         p->stages |= SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL;
         p->vk_tesselation_control_shader = sf_graphics_vulkan_create_shader_module(r, desc->tesselation_control_code_size, desc->tesselation_control_code);
      }

      if (desc->tesselation_evaluation_code_size) {
         p->stages |= SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION;
         p->vk_tesselation_evaluation_shader = sf_graphics_vulkan_create_shader_module(r, desc->tesselation_evaluation_code_size, desc->tesselation_evaluation_code);
      }

      if (desc->geometry_code_size) {
         p->stages |= SF_GRAPHICS_SHADER_STAGE_GEOMETRY;
         p->vk_geometry_shader = sf_graphics_vulkan_create_shader_module(r, desc->geometry_code_size, desc->geometry_code);
      }

      if (desc->fragment_code_size) {
         p->stages |= SF_GRAPHICS_SHADER_STAGE_FRAGMENT;
         p->vk_fragment_shader = sf_graphics_vulkan_create_shader_module(r, desc->fragment_code_size, desc->fragment_code);
      }

      if (desc->compute_code_size) {
         p->stages |= SF_GRAPHICS_SHADER_STAGE_COMPUTE;
         p->vk_compute_shader = sf_graphics_vulkan_create_shader_module(r, desc->compute_code_size, desc->compute_code);
      }

      if ((p->stages & SF_GRAPHICS_SHADER_STAGE_VERTEX) == SF_GRAPHICS_SHADER_STAGE_VERTEX ? !p->vk_vertex_shader : SF_FALSE
         || (p->stages & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL) == SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL ? !p->vk_tesselation_control_shader : SF_FALSE
         || (p->stages & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION) == SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION ? !p->vk_tesselation_evaluation_shader : SF_FALSE
         || (p->stages & SF_GRAPHICS_SHADER_STAGE_GEOMETRY) == SF_GRAPHICS_SHADER_STAGE_GEOMETRY ? !p->vk_geometry_shader : SF_FALSE
         || (p->stages & SF_GRAPHICS_SHADER_STAGE_FRAGMENT) == SF_GRAPHICS_SHADER_STAGE_FRAGMENT ? !p->vk_fragment_shader : SF_FALSE
         || (p->stages & SF_GRAPHICS_SHADER_STAGE_COMPUTE) == SF_GRAPHICS_SHADER_STAGE_COMPUTE ? !p->vk_compute_shader : SF_FALSE) {
         sf_graphics_destroy_program(r, SF_AS_HANDLE(p));
         p = SF_NULL_HANDLE;
      }
   }

   return SF_AS_HANDLE(p);
}

SF_INTERNAL VkDescriptorType sf_graphics_vulkan_as_vulkan_descriptor_type(enum sf_graphics_descriptor_type type) {
   switch (type) {
      case SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER:
         return VK_DESCRIPTOR_TYPE_SAMPLER;
      case SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      case SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE:
         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      default:
         return VK_DESCRIPTOR_TYPE_MAX_ENUM;
   }
}

SF_INTERNAL VkShaderStageFlags sf_graphics_vulkan_as_vulkan_shader_stages(sf_graphics_shader_stage_flags stages) {
   VkShaderStageFlags result = 0;

   if ((stages & SF_GRAPHICS_SHADER_STAGE_VERTEX) == SF_GRAPHICS_SHADER_STAGE_VERTEX)
      result |= VK_SHADER_STAGE_VERTEX_BIT;

   if ((stages & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL) == SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL)
      result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

   if ((stages & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION) == SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION)
      result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

   if ((stages & SF_GRAPHICS_SHADER_STAGE_GEOMETRY) == SF_GRAPHICS_SHADER_STAGE_GEOMETRY)
      result |= VK_SHADER_STAGE_GEOMETRY_BIT;

   if ((stages & SF_GRAPHICS_SHADER_STAGE_COMPUTE) == SF_GRAPHICS_SHADER_STAGE_COMPUTE)
      result |= VK_SHADER_STAGE_COMPUTE_BIT;

   return result;
}

SF_INTERNAL void sf_graphics_default_init_descriptor(sf_graphics_descriptor *d) {
   d->type = SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER;
   d->stages = 0;
   d->binding = 0;
   d->entry_count = 0;
   SF_ARRAY_INIT(d->entries, SF_NULL_HANDLE);
}

SF_INTERNAL void sf_graphics_default_init_descriptor_set(sf_graphics_descriptor_set *ds) {
   SF_QUEUE_INIT(&ds->queue);

   ds->descriptor_count = 0;
   for (uint32_t i = 0; SF_SIZE(ds->descriptors); ++i)
      sf_graphics_default_init_descriptor(&ds->descriptors[i]);

   ds->vk_descriptor_set_layout = VK_NULL_HANDLE;
   ds->vk_descriptor_pool = VK_NULL_HANDLE;
   ds->vk_descriptor_set = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_descriptor_set *sf_graphics_get_or_allocate_descriptor_set(sf_graphics_renderer *r) {
   sf_graphics_descriptor_set *ds = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_descriptor_set_queue)) {
      ds = sf_allocate(&r->arena, sizeof(*ds));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_descriptor_set_queue);
      SF_QUEUE_REMOVE(q);
      ds = SF_QUEUE_DATA(q, sf_graphics_descriptor_set, queue);
   }

   if (ds) {
      sf_graphics_default_init_descriptor_set(ds);
      SF_QUEUE_INSERT_HEAD(&r->descriptor_set_queue, &ds->queue);
   }

   return ds;
}

void sf_graphics_destroy_descriptor_set(sf_graphics_renderer *r, sf_handle descriptor_set) {
   if (r && descriptor_set) {
      sf_graphics_descriptor_set *ds = (sf_graphics_descriptor_set *)descriptor_set;

      SF_QUEUE_REMOVE(&ds->queue);
      SF_QUEUE_INSERT_HEAD(&r->free_program_queue, &ds->queue);

      if (r->vk_device) {
         if (ds->vk_descriptor_pool && ds->vk_descriptor_set) {
            vkFreeDescriptorSets(r->vk_device, ds->vk_descriptor_pool, 1, &ds->vk_descriptor_set);
            ds->vk_descriptor_set = VK_NULL_HANDLE;
         }

         if (ds->vk_descriptor_pool) {
            vkDestroyDescriptorPool(r->vk_device, ds->vk_descriptor_pool, r->vk_allocation_callbacks);
            ds->vk_descriptor_pool = VK_NULL_HANDLE;
         }

         if (ds->vk_descriptor_set_layout) {
            vkDestroyDescriptorSetLayout(r->vk_device, ds->vk_descriptor_set_layout, r->vk_allocation_callbacks);
            ds->vk_descriptor_set_layout = VK_NULL_HANDLE;
         }
      }
   }
}

sf_handle sf_graphics_create_descriptor_set(sf_graphics_renderer *r, sf_graphics_descriptor_set_description const *desc) {
   sf_graphics_descriptor_set *ds = sf_graphics_get_or_allocate_descriptor_set(r);
   if (ds) {
      ds->descriptor_count = desc->descriptor_count;
      for (uint32_t i = 0; i < ds->descriptor_count; ++i)
         ds->descriptors[i] = desc->descriptors[i];

      uint32_t descriptor_count_by_type[SF_GRAPHICS_DESCRIPTOR_TYPE_COUNT] = {0};
      for (uint32_t i = 0; i < ds->descriptor_count; ++i)
         ++descriptor_count_by_type[desc->descriptors[i].type];

      VkDescriptorPoolSize pool_sizes[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT] = {0};
      VkDescriptorSetLayoutBinding bindings[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT] = {0};
      for (uint32_t i = 0; i < ds->descriptor_count; ++i) {
         sf_graphics_descriptor *d = &ds->descriptors[i];

         pool_sizes[i].type = sf_graphics_vulkan_as_vulkan_descriptor_type(d->type);
         pool_sizes[i].descriptorCount = descriptor_count_by_type[d->type];

         bindings[i].binding = d->binding;
         bindings[i].descriptorType = sf_graphics_vulkan_as_vulkan_descriptor_type(d->type);
         bindings[i].descriptorCount = d->entry_count;
         bindings[i].stageFlags = sf_graphics_vulkan_as_vulkan_shader_stages(d->stages);
         bindings[i].pImmutableSamplers = NULL;
      }

      VkDescriptorSetLayoutCreateInfo const layout_info = {
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .pNext = NULL,
         .flags = 0,
         .bindingCount = ds->descriptor_count,
         .pBindings = bindings
      };
      if (SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(r->vk_device, &layout_info, r->vk_allocation_callbacks, &ds->vk_descriptor_set_layout))) {
         VkDescriptorPoolCreateInfo const descriptor_pool_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = NULL,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1,
            .poolSizeCount = ds->descriptor_count,
            .pPoolSizes = pool_sizes
         };
         if (SF_VULKAN_CHECK(vkCreateDescriptorPool(r->vk_device, &descriptor_pool_info, r->vk_allocation_callbacks, &ds->vk_descriptor_pool))) {
            VkDescriptorSetAllocateInfo const descriptor_set_info = {
               .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
               .pNext = NULL,
               .descriptorPool = ds->vk_descriptor_pool,
               .descriptorSetCount = 1,
               .pSetLayouts = &ds->vk_descriptor_set_layout
            };
            SF_VULKAN_CHECK(vkAllocateDescriptorSets(r->vk_device, &descriptor_set_info, &ds->vk_descriptor_set));
         }
      }

      if (!ds->vk_descriptor_set_layout || !ds->vk_descriptor_pool || !ds->vk_descriptor_set) {
         sf_graphics_destroy_descriptor_set(r, SF_AS_HANDLE(ds));
         ds = SF_NULL_HANDLE;
      }
   }

   return SF_AS_HANDLE(ds);
}

SF_INTERNAL void sf_graphics_default_init_pipeline(sf_graphics_pipeline *p) {
   SF_QUEUE_INIT(&p->queue);
   p->type = SF_GRAPHICS_PIPELINE_TYPE_GRAPHICS;
   p->vk_pipeline_layout = VK_NULL_HANDLE;
   p->vk_pipeline = VK_NULL_HANDLE;
}

SF_INTERNAL sf_graphics_pipeline *sf_graphics_get_or_allocate_pipeline(sf_graphics_renderer *r) {
   sf_graphics_pipeline *p = NULL;

   if (SF_QUEUE_IS_EMPTY(&r->free_pipeline_queue)) {
      p = sf_allocate(&r->arena, sizeof(*p));
   } else {
      sf_queue *q = SF_QUEUE_HEAD(&r->free_pipeline_queue);
      SF_QUEUE_REMOVE(q);
      p = SF_QUEUE_DATA(q, sf_graphics_pipeline, queue);
   }

   if (p) {
      sf_graphics_default_init_pipeline(p);
      SF_QUEUE_INSERT_HEAD(&r->pipeline_queue, &p->queue);
   }

   return p;
}

sf_handle sf_graphics_create_pipeline(sf_graphics_renderer *r, sf_graphics_pipeline_description const *desc) {
   sf_graphics_pipeline *p = sf_graphics_get_or_allocate_pipeline(r);
   if (p) {
      sf_graphics_descriptor_set const *ds = (sf_graphics_descriptor_set const *)desc->descriptor_set;
      VkPipelineLayoutCreateInfo const layout_info ={
         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .pNext = NULL,
         .flags = 0,
         .setLayoutCount = ds ? 1 : 0,
         .pSetLayouts = ds ? (&ds->vk_descriptor_set_layout) : NULL,
         .pushConstantRangeCount = 0,
         .pPushConstantRanges = NULL,
      };
      if (SF_VULKAN_CHECK(vkCreatePipelineLayout(r->vk_device, &layout_info, r->vk_allocation_callbacks, &p->vk_pipeline_layout))) {
         VkPipelineShaderStageCreateInfo stages[5] = {0};
         sf_graphics_program const *program = (sf_graphics_program const *)desc->program;
         for (uint32_t i = 0; i < SF_SIZE(stages); ++i) {
            sf_graphics_shader_stage_flags const stage = (1 << i);
            if ((stage & desc->))
         }
      }
   }
}

sf_handle sf_graphics_get_graphics_queue(struct sf_graphics_renderer *r) {
   return SF_AS_HANDLE(&r->graphics_queue);
}

sf_handle sf_graphics_get_present_queue(struct sf_graphics_renderer *r) {
   return SF_AS_HANDLE(&r->present_queue);
}

SF_INTERNAL void sf_graphics_default_init_renderer(struct sf_graphics_renderer *r) {
   r->arena.data = NULL;
   r->arena.position = 0;
   r->arena.alignment = 0;
   r->arena.capacity = 0;

   r->api.os = SF_API_OS_MACOS;
   r->api.window = NULL;

   r->swapchain_width = 0;
   r->swapchain_height = 0;
   r->swapchain_image_count = 0;
   r->swapchain_color_clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;
   r->swapchain_depth_stencil_clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;

   r->sample_count = SF_GRAPHICS_SAMPLE_COUNT_1;
   r->color_attachment_format = SF_GRAPHICS_FORMAT_UNDEFINED;
   r->depth_stencil_format = SF_GRAPHICS_FORMAT_UNDEFINED;

   r->buffering_count = 0;
   r->enable_vsync = SF_TRUE;

   r->application_name = NULL;

   r->vk_instance_layer_count = 0;
   r->vk_instance_layers = NULL;

   r->vk_instance_extension_count = 0;
   r->vk_instance_extensions = NULL;

   r->vk_device_extension_count = 0;
   r->vk_device_extensions = NULL;

   r->vk_allocation_callbacks = NULL;
   r->vk_debug_callback = NULL;

   r->graphics_queue.vk_queue = VK_NULL_HANDLE;
   r->graphics_queue.vk_queue_family_index = 0;

   r->present_queue.vk_queue = VK_NULL_HANDLE;
   r->present_queue.vk_queue_family_index = 0;

   SF_QUEUE_INIT(&r->texture_queue);
   SF_QUEUE_INIT(&r->free_texture_queue);

   SF_QUEUE_INIT(&r->buffer_queue);
   SF_QUEUE_INIT(&r->free_buffer_queue);

   SF_QUEUE_INIT(&r->command_pool_queue);
   SF_QUEUE_INIT(&r->free_command_pool_queue);

   SF_QUEUE_INIT(&r->command_buffer_queue);
   SF_QUEUE_INIT(&r->free_command_buffer_queue);

   SF_QUEUE_INIT(&r->semaphore_queue);
   SF_QUEUE_INIT(&r->free_semaphore_queue);

   SF_QUEUE_INIT(&r->fence_queue);
   SF_QUEUE_INIT(&r->free_fence_queue);

   r->render_target_arena.data = NULL;
   r->render_target_arena.position = 0;
   r->render_target_arena.alignment = 0;
   r->render_target_arena.capacity = 0;
   SF_QUEUE_INIT(&r->render_target_queue);
   SF_QUEUE_INIT(&r->free_render_target_queue);

   SF_QUEUE_INIT(&r->program_queue);
   SF_QUEUE_INIT(&r->free_program_queue);

   SF_QUEUE_INIT(&r->error_queue);

   r->swapchain_skip_end_frame = SF_FALSE;
   r->swapchain_current_image_index = 0;
   r->swapchain_render_target_count = 0;
   SF_ARRAY_INIT(r->swapchain_render_targets, SF_NULL_HANDLE);

   r->image_acquired_semaphore_count = 0;
   SF_ARRAY_INIT(r->image_acquired_semaphores, SF_NULL_HANDLE);

   r->in_flight_fence_count = 0;
   SF_ARRAY_INIT(r->in_flight_fences, SF_NULL_HANDLE);

   r->draw_complete_semaphore_count = 0;
   SF_ARRAY_INIT(r->draw_complete_semaphores, SF_NULL_HANDLE);

   r->vk_instance = VK_NULL_HANDLE;
   r->vk_allocation_callbacks = VK_NULL_HANDLE;
   r->vk_validation_messenger = VK_NULL_HANDLE;
   r->vk_surface_format = VK_FORMAT_UNDEFINED;
   r->vk_depth_stencil_format = VK_FORMAT_UNDEFINED;
   r->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
   r->vk_surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
   r->vk_surface = VK_NULL_HANDLE;
   r->vk_sample_count = VK_SAMPLE_COUNT_1_BIT;
   r->vk_physical_device = VK_NULL_HANDLE;
   r->vk_device = VK_NULL_HANDLE;
   r->vk_swapchain = VK_NULL_HANDLE;
   r->vk_swapchain_image_count = 0;
   SF_ARRAY_INIT(r->vk_swapchain_images, SF_NULL_HANDLE);

   r->vk_create_debug_utils_messenger_ext = NULL;
   r->vk_destroy_debug_utils_messenger_ext = NULL;
}

SF_INTERNAL sf_bool sf_vulkan_test_format_features(sf_graphics_renderer *r, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
   sf_bool result = SF_FALSE;
   if (r) {
      VkFormatProperties properties = {0};
      vkGetPhysicalDeviceFormatProperties(r->vk_physical_device, format, &properties);

      if (VK_IMAGE_TILING_LINEAR == tiling)
         result = (properties.linearTilingFeatures & features) == features;

      if (VK_IMAGE_TILING_OPTIMAL == tiling)
         result = (properties.optimalTilingFeatures & features) == features;
   }
   return result;
}

sf_graphics_renderer *sf_graphics_create_renderer(sf_graphics_renderer_description const *desc) {
   sf_graphics_renderer *r = sf_allocate(desc->arena, sizeof(sf_graphics_renderer));
   if (r) {
      sf_graphics_default_init_renderer(r);

      r->api.os = desc->api.os;
      r->api.window = desc->api.window;

      r->swapchain_width = desc->swapchain_width;
      r->swapchain_height = desc->swapchain_height;
      r->swapchain_image_count = desc->swapchain_image_count;

      r->swapchain_color_clear_value = desc->swapchain_color_clear_value;
      r->swapchain_depth_stencil_clear_value = desc->swapchain_depth_stencil_clear_value;

      r->sample_count = desc->sample_count;
      r->color_attachment_format = desc->color_attachment_format;
      r->depth_stencil_format = desc->depth_stencil_format;

      r->buffering_count = desc->buffering_count;
      r->enable_vsync = desc->buffering_count;

      r->application_name = desc->application_name;

      r->vk_instance_layer_count = desc->vk_instance_layer_count;
      r->vk_instance_layers = desc->vk_instance_layers;

      r->vk_instance_extension_count = desc->vk_instance_extension_count;
      r->vk_instance_extensions = desc->vk_instance_extensions;

      r->vk_device_extension_count = desc->vk_device_extension_count;
      r->vk_device_extensions = desc->vk_device_extensions;

      r->vk_allocation_callbacks = desc->vk_allocation_callbacks;
      r->vk_debug_callback = desc->vk_debug_callback;

      VkApplicationInfo const application_info = {
         .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
         .pNext = NULL,
         .pApplicationName = r->application_name,
         .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
         .pEngineName = NULL,
         .engineVersion = VK_MAKE_VERSION(1, 0, 0),
         .apiVersion = VK_MAKE_VERSION(1, 0, 0)
      };

      VkInstanceCreateInfo const instance_info = {
         .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
         .pNext = NULL,
         .flags = 0 | ((r->api.os == SF_API_OS_MACOS) ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0),
         .pApplicationInfo = &application_info,
         .enabledLayerCount = r->vk_instance_layer_count,
         .ppEnabledLayerNames = r->vk_instance_layers,
         .enabledExtensionCount = r->vk_instance_extension_count,
         .ppEnabledExtensionNames = r->vk_instance_extensions
      };

      if (SF_VULKAN_CHECK(vkCreateInstance(&instance_info, r->vk_allocation_callbacks, &r->vk_instance))) {
         r->vk_create_debug_utils_messenger_ext = SF_VULKAN_PROC(vkCreateDebugUtilsMessengerEXT, r->vk_instance);
         r->vk_destroy_debug_utils_messenger_ext = SF_VULKAN_PROC(vkDestroyDebugUtilsMessengerEXT, r->vk_instance);

         if (r->vk_create_debug_utils_messenger_ext && r->vk_destroy_debug_utils_messenger_ext) {
            VkDebugUtilsMessengerCreateInfoEXT const validator_info = {
               .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
               .pNext = NULL,
               .flags = 0,
               .messageSeverity = 0
                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
               .messageType = 0
                  | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
               .pfnUserCallback = r->vk_debug_callback,
               .pUserData = NULL
            };

            PFN_vkCreateDebugUtilsMessengerEXT const create = r->vk_create_debug_utils_messenger_ext;
            SF_VULKAN_CHECK(create(r->vk_instance, &validator_info, r->vk_allocation_callbacks, &r->vk_validation_messenger));
         }

         if (SF_VULKAN_CHECK(glfwCreateWindowSurface(r->vk_instance, r->api.window, r->vk_allocation_callbacks, &r->vk_surface))) {
            sf_vulkan_physical_device_list list = sf_vulkan_create_physical_device_list(&r->arena, r);

            for (uint32_t i = 0; i < list.size; ++i) {
               r->vk_physical_device = list.data[i];
               if (sf_vulkan_check_physical_device_support(r))
                  break;
               r->vk_physical_device = VK_NULL_HANDLE;
            }
         }

         if (r->vk_surface && r->vk_physical_device) {
            sf_vulkan_surface_format_list surface_format_list = sf_vulkan_create_surface_format_list(&r->arena, r);
            if (surface_format_list.size && surface_format_list.data) {

               r->vk_surface_format = surface_format_list.data[0].format;
               r->vk_surface_color_space = surface_format_list.data[0].colorSpace;

               for (uint32_t i = 0; i < surface_format_list.size; ++i) {
                  VkSurfaceFormatKHR *format = &surface_format_list.data[i];

                  if (format->format == VK_FORMAT_B8G8R8A8_SRGB || format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                     r->vk_surface_format = VK_FORMAT_B8G8R8A8_SRGB;
                     r->vk_surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                     break;
                  }
               }
            }
         }

         if (r->vk_surface && r->vk_physical_device) {
            sf_vulkan_present_mode_list present_mode_list = sf_vulkan_create_present_mode_list(&r->arena, r);
            if (present_mode_list.size && present_mode_list.data) {
               sf_bool found_present_mode = SF_FALSE;

               if (r->enable_vsync)
                  r->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
               else
                  r->vk_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;

               for (uint32_t i = 0; i < present_mode_list.size && !found_present_mode; ++i) {
                  if (present_mode_list.data[i] == r->vk_present_mode) {
                     found_present_mode = SF_TRUE;
                  }
               }

               if (!found_present_mode) {
                  r->enable_vsync = SF_TRUE;
                  r->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
               }
            }
         }
         if(r->vk_surface && r->vk_physical_device) {
            if (sf_vulkan_test_format_features(r, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
               r->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT;
            else if (sf_vulkan_test_format_features(r, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
               r->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
            else if (sf_vulkan_test_format_features(r, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
               r->vk_depth_stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;

            SF_ASSERT(0);
            r->vk_sample_count = VK_SAMPLE_COUNT_2_BIT;
         }

         if (r->vk_physical_device) {
            float const priority = 1.0F;

            VkDeviceQueueCreateInfo const queue_info[2] = {
               {
                  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                  .pNext = NULL,
                  .flags = 0,
                  .queueFamilyIndex = r->graphics_queue.vk_queue_family_index,
                  .queueCount = 1,
                  .pQueuePriorities = &priority
               },
               {
                  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                  .pNext = NULL,
                  .flags = 0,
                  .queueFamilyIndex = r->present_queue.vk_queue_family_index,
                  .queueCount = 1,
                  .pQueuePriorities = &priority
               }
            };

            VkPhysicalDeviceFeatures features = {0};
            vkGetPhysicalDeviceFeatures(r->vk_physical_device, &features);

            VkDeviceCreateInfo const device_info = {
               .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
               .pNext = NULL,
               .flags = 0,
               .queueCreateInfoCount = (r->graphics_queue.vk_queue_family_index == r->present_queue.vk_queue_family_index) ? 1 : SF_SIZE(queue_info),
               .pQueueCreateInfos = queue_info,
               .enabledLayerCount = 0,
               .ppEnabledLayerNames = NULL,
               .enabledExtensionCount = r->vk_device_extension_count,
               .ppEnabledExtensionNames = r->vk_device_extensions,
               .pEnabledFeatures = &features
            };

            if (SF_VULKAN_CHECK(vkCreateDevice(r->vk_physical_device, &device_info, r->vk_allocation_callbacks, &r->vk_device))) {
               vkGetDeviceQueue(r->vk_device, r->graphics_queue.vk_queue_family_index, 0, &r->graphics_queue.vk_queue);
               vkGetDeviceQueue(r->vk_device, r->present_queue.vk_queue_family_index, 0, &r->present_queue.vk_queue);
            }
         }

         r->image_acquired_semaphore_count = r->buffering_count;
         for (uint32_t i = 0; i < r->buffering_count; ++i) {
            r->image_acquired_semaphores[i] = sf_graphics_create_semaphore(r);
         }

         r->in_flight_fence_count = r->buffering_count;
         for (uint32_t i = 0; i < r->buffering_count; ++i) {
            r->in_flight_fences[i] = sf_graphics_create_fence(r);
         }
      }
   }

   return r;
}

void sf_graphics_destroy_renderer(sf_graphics_renderer *r) {
   if (r) {
      sf_graphics_device_wait_idle(r);

      for (uint32_t i = 0; i < SF_SIZE(r->in_flight_fences); ++i) {
         sf_graphics_destroy_fence(r, r->in_flight_fences[i]);
         r->in_flight_fences[i] = SF_NULL_HANDLE;
      }
      r->in_flight_fence_count = 0;

      for (uint32_t i = 0; i < SF_SIZE(r->image_acquired_semaphores); ++i) {
         sf_graphics_destroy_semaphore(r, r->image_acquired_semaphores[i]);
         r->image_acquired_semaphores[i] = SF_NULL_HANDLE;
      }
      r->image_acquired_semaphore_count = 0;

      sf_graphics_destroy_swapchain_resources(r);

      if (r->vk_device) {
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

      sf_graphics_default_init_renderer(r);
   }
}
