#include "sf_graphics.h"

#include <GLFW/glfw3.h>
#include <sf.h>

#define SF_VULKAN_CHECK(e) ((e) == VK_SUCCESS)

SF_INTERNAL void sf_graphics_vulkan_create_instance(struct sf_graphics_renderer *r) {
	VkApplicationInfo application_info = {0};
	VkInstanceCreateInfo info = {0};

	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pNext = NULL;
	application_info.pApplicationName = r->application_name;
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.pEngineName = NULL;
	application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	if (r->api.os == SF_API_OS_MACOS)
		info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	info.pApplicationInfo = &application_info;
	info.enabledLayerCount = r->vk_instance_layer_count;
	info.ppEnabledLayerNames = r->vk_instance_layers;
	info.enabledExtensionCount = r->vk_instance_extension_count;
	info.ppEnabledExtensionNames = r->vk_instance_extensions;

	SF_VULKAN_CHECK(vkCreateInstance(&info, r->vk_allocation_callbacks, &r->vk_instance));
}

#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)
SF_INTERNAL void sf_graphics_vulkan_proc_functions(struct sf_graphics_renderer *r) {
	r->vk_create_debug_utils_messenger_ext = SF_VULKAN_PROC(vkCreateDebugUtilsMessengerEXT, r->vk_instance);
	r->vk_destroy_debug_utils_messenger_ext = SF_VULKAN_PROC(vkDestroyDebugUtilsMessengerEXT, r->vk_instance);
}

SF_INTERNAL void sf_graphics_vulkan_create_validation_messenger(struct sf_graphics_renderer *r) {
	VkDebugUtilsMessengerCreateInfoEXT info = {0};

	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.pNext = NULL;
	info.flags = 0;
	info.messageSeverity = 0;
	info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info.messageType = 0;
	info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info.pfnUserCallback = r->vk_debug_callback;
	info.pUserData = NULL;

	SF_VULKAN_CHECK(r->vk_create_debug_utils_messenger_ext(r->vk_instance, &info, r->vk_allocation_callbacks, &r->vk_validation_messenger));
}

SF_INTERNAL void sf_graphics_vulkan_create_surface(struct sf_graphics_renderer *r) {
	SF_VULKAN_CHECK(glfwCreateWindowSurface(r->vk_instance, r->api.window, r->vk_allocation_callbacks, &r->vk_surface));
}

SF_INTERNAL sf_bool sf_graphics_vulkan_validate_queue_family_indices(struct sf_graphics_renderer *r) {
	return r->graphics_queue.vk_queue_family_index != (sf_u32)-1 && r->present_queue.vk_queue_family_index != (sf_u32)-1;
}

SF_INTERNAL void sf_graphics_vulkan_find_queue_family_indices(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;
	sf_u32 property_count = 0;
	VkQueueFamilyProperties *properties = NULL;

	struct sf_arena *arena = &r->arena;

	vkGetPhysicalDeviceQueueFamilyProperties(r->vk_physical_device, &property_count, NULL);
	if (!property_count)
		return;

	properties = sf_allocate(arena, property_count * sizeof(*properties));
	if (!properties)
		return;

	vkGetPhysicalDeviceQueueFamilyProperties(r->vk_physical_device, &property_count, properties);
	for (i = 0; i < property_count && !sf_graphics_vulkan_validate_queue_family_indices(r); ++i) {
		VkBool32 supports_surface = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(r->vk_physical_device, i, r->vk_surface, &supports_surface);

		if (supports_surface)
			r->present_queue.vk_queue_family_index = i;

		if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			r->graphics_queue.vk_queue_family_index = i;
	}
}

SF_INTERNAL sf_bool sf_graphics_vulkan_check_physical_device_extension_support(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;
	sf_u32 available_extension_count = 0;
	VkExtensionProperties *available_extensions = NULL;

	struct sf_arena *arena = &r->arena;

	if (!SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(r->vk_physical_device, NULL, &available_extension_count, NULL)))
		return SF_FALSE;

	available_extensions = sf_allocate(arena, available_extension_count * sizeof(*available_extensions));
	if (!available_extensions)
		return SF_FALSE;

	if (!SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(r->vk_physical_device, NULL, &available_extension_count, available_extensions)))
		return SF_FALSE;

	for (i = 0; i < r->vk_device_extension_count; ++i) {
		sf_u32 j = 0;
		sf_bool found_current_required_extension = SF_FALSE;
		struct sf_s8 current_required_extension = {0};

		sf_s8_from_non_literal(r->vk_device_extensions[i], &current_required_extension);

		for (j = 0, found_current_required_extension = 0; j < available_extension_count && !found_current_required_extension; ++j) {
			struct sf_s8 current_available_extension = {0};
			sf_s8_from_non_literal(available_extensions[j].extensionName, &current_available_extension);
			found_current_required_extension = sf_s8_compare(&current_required_extension, &current_available_extension, VK_MAX_EXTENSION_NAME_SIZE);
		}

		if (!found_current_required_extension)
			return SF_FALSE;
	}

	return SF_TRUE;
}

SF_INTERNAL sf_bool sf_graphics_vulkan_check_physical_device_swapchain_support(struct sf_graphics_renderer *r) {
	sf_u32 surface_format_count = 0, present_mode_count = 0;

	SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vk_physical_device, r->vk_surface, &surface_format_count, NULL));
	SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vk_physical_device, r->vk_surface, &present_mode_count, NULL));

	return surface_format_count && present_mode_count;
}

SF_INTERNAL sf_bool sf_graphics_vulkan_check_physical_device_support(struct sf_graphics_renderer *r) {
	sf_graphics_vulkan_find_queue_family_indices(r);
	if (!sf_graphics_vulkan_validate_queue_family_indices(r))
		return SF_FALSE;

	if (!sf_graphics_vulkan_check_physical_device_swapchain_support(r))
		return SF_FALSE;

	if (!sf_graphics_vulkan_check_physical_device_extension_support(r))
		return SF_FALSE;

	return SF_TRUE;
}

SF_INTERNAL void sf_graphics_vulkan_pick_physical_device(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;
	sf_u32 physical_device_count = 0;
	VkPhysicalDevice *devices = NULL;

	struct sf_arena *arena = &r->arena;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(r->vk_instance, &physical_device_count, NULL)))
		return;

	devices = sf_allocate(arena, physical_device_count * sizeof(*devices));
	if (!devices)
		return;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(r->vk_instance, &physical_device_count, devices)))
		return;

	for (i = 0; i < physical_device_count; ++i) {
		r->vk_physical_device = devices[i];
		if (sf_graphics_vulkan_check_physical_device_support(r))
			return;
	}

	r->vk_physical_device = VK_NULL_HANDLE;
}

SF_INTERNAL void sf_graphics_vulkan_create_device(struct sf_graphics_renderer *r) {
	float priority = 1.0F;
	VkDeviceCreateInfo info = {0};
	VkPhysicalDeviceFeatures features = {0};
	VkDeviceQueueCreateInfo queue_info[2] = {0};

	vkGetPhysicalDeviceFeatures(r->vk_physical_device, &features);

	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].pNext = NULL;
	queue_info[0].flags = 0;
	queue_info[0].queueFamilyIndex = r->graphics_queue.vk_queue_family_index;
	queue_info[0].queueCount = 1;
	queue_info[0].pQueuePriorities = &priority;

	queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[1].pNext = NULL;
	queue_info[1].flags = 0;
	queue_info[1].queueFamilyIndex = r->present_queue.vk_queue_family_index;
	queue_info[1].queueCount = 1;
	queue_info[1].pQueuePriorities = &priority;

	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	if (r->graphics_queue.vk_queue_family_index == r->present_queue.vk_queue_family_index)
		info.queueCreateInfoCount = 1;
	else
		info.queueCreateInfoCount = SF_SIZE(queue_info);
	info.pQueueCreateInfos = queue_info;
	info.enabledLayerCount = 0;
	info.ppEnabledLayerNames = NULL;
	info.enabledExtensionCount = r->vk_device_extension_count;
	info.ppEnabledExtensionNames = r->vk_device_extensions;
	info.pEnabledFeatures = &features;

	SF_VULKAN_CHECK(vkCreateDevice(r->vk_physical_device, &info, r->vk_allocation_callbacks, &r->vk_device));
}

SF_INTERNAL void sf_graphics_vulkan_set_device_queues(struct sf_graphics_renderer *r) {
	vkGetDeviceQueue(r->vk_device, r->graphics_queue.vk_queue_family_index, 0, &r->graphics_queue.vk_queue);
	vkGetDeviceQueue(r->vk_device, r->present_queue.vk_queue_family_index, 0, &r->present_queue.vk_queue);
}

SF_INTERNAL void sf_graphics_vulkan_pick_surface_format(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;
	sf_u32 format_count = 0;
	VkSurfaceFormatKHR *formats = NULL;

	struct sf_arena *arena = &r->arena;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vk_physical_device, r->vk_surface, &format_count, NULL)))
		return;

	formats = sf_allocate(arena, format_count * sizeof(*formats));
	if (!formats)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r->vk_physical_device, r->vk_surface, &format_count, formats)))
		return;

	r->vk_surface_format = formats[0].format;
	r->vk_surface_color_space = formats[0].colorSpace;

	for (i = 0; i < format_count; ++i) {
		VkSurfaceFormatKHR *format = &formats[i];

		if (format->format == VK_FORMAT_B8G8R8A8_SRGB || format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			r->vk_surface_format = VK_FORMAT_B8G8R8A8_SRGB;
			r->vk_surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			return;
		}
	}
}

SF_INTERNAL void sf_graphics_vulkan_pick_present_mode(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;
	sf_u32 present_mode_count = 0;
	VkPresentModeKHR *present_modes = NULL;
	struct sf_arena *arena = &r->arena;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vk_physical_device, r->vk_surface, &present_mode_count, NULL)))
		return;

	present_modes = sf_allocate(arena, present_mode_count * sizeof(*present_modes));
	if (!present_modes)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(r->vk_physical_device, r->vk_surface, &present_mode_count, NULL)))
		return;

	if (r->enable_vsync)
		r->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	else
		r->vk_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;

	for (i = 0; i < present_mode_count; ++i)
		if (present_modes[i] == r->vk_present_mode)
			return;

	r->enable_vsync = SF_TRUE;
	r->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
}

SF_INTERNAL sf_bool sf_graphics_vulkan_test_format_features(struct sf_graphics_renderer *r, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
	VkFormatProperties properties = {0};
	vkGetPhysicalDeviceFormatProperties(r->vk_physical_device, format, &properties);

	if (VK_IMAGE_TILING_LINEAR == tiling)
		return !!(properties.linearTilingFeatures & features);

	if (VK_IMAGE_TILING_OPTIMAL == tiling)
		return !!(properties.optimalTilingFeatures & features);

	return SF_FALSE;
}

SF_INTERNAL void sf_graphics_vulkan_pick_depth_stencil_format(struct sf_graphics_renderer *r) {
	if (sf_graphics_vulkan_test_format_features(r, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		r->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT;
	else if (sf_graphics_vulkan_test_format_features(r, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		r->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	else if (sf_graphics_vulkan_test_format_features(r, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		r->vk_depth_stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;
}

SF_INTERNAL void sf_graphics_vulkan_pick_sample_count(struct sf_graphics_renderer *r) {
	VkPhysicalDeviceProperties properties = {0};
	VkSampleCountFlags sample_counts = {0};

	vkGetPhysicalDeviceProperties(r->vk_physical_device, &properties);
	sample_counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

	if (0)
		(void)0;
	else if (sample_counts & VK_SAMPLE_COUNT_2_BIT) {
		r->sample_count = SF_GRAPHICS_SAMPLE_COUNT_2;
	}
}

SF_INTERNAL sf_u32 sf_graphics_vulkan_find_device_memory_type_index(VkPhysicalDevice device, VkMemoryPropertyFlags memory_properties, sf_u32 filter) {
	sf_u32 current_type_index = (sf_u32)-1;
	VkPhysicalDeviceMemoryProperties available_properties = {0};

	if (!device)
		return (sf_u32)-1;

	vkGetPhysicalDeviceMemoryProperties(device, &available_properties);

	for (current_type_index = 0; current_type_index < available_properties.memoryTypeCount; ++current_type_index)
		if ((filter & (1 << current_type_index)) && (available_properties.memoryTypes[current_type_index].propertyFlags & memory_properties) == memory_properties)
			return current_type_index;

	return (sf_u32)-1;
}

SF_INTERNAL VkDeviceMemory sf_graphics_vulkan_allocate_memory(struct sf_graphics_renderer *r, VkMemoryPropertyFlags memory_properties, sf_u32 filter, uint64_t size) {
	VkMemoryAllocateInfo info = {0};
	VkDeviceMemory memory = VK_NULL_HANDLE;
	sf_u32 memory_type_index = sf_graphics_vulkan_find_device_memory_type_index(r->vk_physical_device, memory_properties, filter);

	if ((sf_u32)-1 == memory_type_index)
		return VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.pNext = NULL;
	info.allocationSize = size;
	info.memoryTypeIndex = memory_type_index;

	if (!SF_VULKAN_CHECK(vkAllocateMemory(r->vk_device, &info, r->vk_allocation_callbacks, &memory)))
		return VK_NULL_HANDLE;

	return memory;
}

SF_INTERNAL VkDeviceMemory sf_graphics_vulkan_allocate_and_bind_memory_to_image(struct sf_graphics_renderer *r, VkImage image, VkMemoryPropertyFlags properties) {
	VkMemoryRequirements requirements = {0};
	VkDeviceMemory memory = VK_NULL_HANDLE;

	vkGetImageMemoryRequirements(r->vk_device, image, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(r, properties, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindImageMemory(r->vk_device, image, memory, 0))) {
		vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}

SF_INTERNAL VkDeviceMemory sf_graphics_vulkan_allocate_and_bind_memory_to_buffer(struct sf_graphics_renderer *r, VkBuffer buffer, VkMemoryPropertyFlags properties) {
	VkMemoryRequirements requirements = {0};
	VkDeviceMemory memory = VK_NULL_HANDLE;

	vkGetBufferMemoryRequirements(r->vk_device, buffer, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(r, properties, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindBufferMemory(r->vk_device, buffer, memory, 0))) {
		vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}

void sf_graphics_device_wait_idle(struct sf_graphics_renderer *r) {
	vkDeviceWaitIdle(r->vk_device);
}

SF_INTERNAL void sf_graphics_destroy_swapchain_resources(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;

	sf_graphics_device_wait_idle(r);

	for (i = 0; i < SF_SIZE(r->swapchain_render_targets); ++i) {
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

SF_INTERNAL void sf_graphics_vulkan_create_swapchain(struct sf_graphics_renderer *r) {
	VkSurfaceCapabilitiesKHR capabilities = {0};
	VkSwapchainCreateInfoKHR info = {0};
	sf_u32 queue_family_indices[2];

	queue_family_indices[0] = r->graphics_queue.vk_queue_family_index;
	queue_family_indices[1] = r->present_queue.vk_queue_family_index;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->vk_physical_device, r->vk_surface, &capabilities);

	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.surface = r->vk_surface;
	info.minImageCount = SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT;
	info.imageFormat = r->vk_surface_format;
	info.imageColorSpace = r->vk_surface_color_space;
	info.imageExtent.width = r->swapchain_width;
	info.imageExtent.height = r->swapchain_height;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (r->graphics_queue.vk_queue_family_index == r->present_queue.vk_queue_family_index) {
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 1;
		info.pQueueFamilyIndices = queue_family_indices;
	} else {
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = SF_SIZE(queue_family_indices);
		info.pQueueFamilyIndices = queue_family_indices;
	}
	info.preTransform = capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = r->vk_present_mode;
	info.clipped = VK_TRUE;
	info.oldSwapchain = VK_NULL_HANDLE;

	SF_VULKAN_CHECK(vkCreateSwapchainKHR(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_swapchain));
}

SF_INTERNAL void sf_graphics_vulkan_load_swapchain_images(struct sf_graphics_renderer *r) {
	sf_u32 swapchain_image_count = 0;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &swapchain_image_count, NULL)))
		return;

	if (SF_SIZE(r->vk_swapchain_images) < swapchain_image_count)
		return;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &swapchain_image_count, r->vk_swapchain_images)))
		return;

	r->vk_swapchain_image_count = swapchain_image_count;
}

SF_INTERNAL void sf_graphics_default_init_texture(struct sf_graphics_texture *t) {
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

SF_INTERNAL struct sf_graphics_texture *sf_graphics_get_or_allocate_texture(struct sf_graphics_renderer *r) {
	struct sf_graphics_texture *t = NULL;

	if (SF_QUEUE_IS_EMPTY(&r->free_texture_queue)) {
		t = sf_allocate(&r->arena, sizeof(*t));
		if (!t)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_texture_queue);
		SF_QUEUE_REMOVE(q);
		t = SF_QUEUE_DATA(q, struct sf_graphics_texture, queue);
	}
	sf_graphics_default_init_texture(t);
	SF_QUEUE_INSERT_HEAD(&r->texture_queue, &t->queue);
	return t;
}

SF_INTERNAL void sf_graphics_default_init_render_target_description(struct sf_graphics_render_target_description *desc) {
	sf_u32 i = 0;
	desc->sample_count = SF_GRAPHICS_SAMPLE_COUNT_1;
	desc->color_format = SF_GRAPHICS_FORMAT_UNDEFINED;
	desc->depth_stencil_format = SF_GRAPHICS_FORMAT_UNDEFINED;
	desc->width = 0;
	desc->height = 0;
	desc->color_attachment_clear_value_count = 0;

	for (i = 0; i < SF_SIZE(desc->color_attachment_clear_values); ++i)
		desc->color_attachment_clear_values[i].type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;

	desc->depth_stencil_attachment_clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;
	desc->vk_not_owned_color_image = VK_NULL_HANDLE;
}

SF_INTERNAL void sf_graphics_copy_clear_value(struct sf_graphics_clear_value *dst, struct sf_graphics_clear_value *src) {
	if (src->type == SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH) {
		dst->type = SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH;
		dst->data.depth.depth = src->data.depth.depth;
		dst->data.depth.stencil = src->data.depth.stencil;
	} else if (src->type == SF_GRAPHICS_CLEAR_VALUE_TYPE_RGBA) {
		dst->type = SF_GRAPHICS_CLEAR_VALUE_TYPE_RGBA;
		dst->data.rgba.r = src->data.rgba.r;
		dst->data.rgba.g = src->data.rgba.g;
		dst->data.rgba.b = src->data.rgba.b;
		dst->data.rgba.a = src->data.rgba.a;
	} else {
		dst->type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;
		dst->data.rgba.r = 0;
		dst->data.rgba.g = 0;
		dst->data.rgba.b = 0;
		dst->data.rgba.a = 0;
	}
}

SF_INTERNAL void sf_graphics_create_swapchain_render_targets(struct sf_graphics_renderer *r) {
	sf_u32 i = 0, swapchain_image_count = r->vk_swapchain_image_count;

	if (swapchain_image_count > SF_SIZE(r->swapchain_render_targets))
		return;

	for (i = 0; i < swapchain_image_count; ++i) {
		struct sf_graphics_render_target_description desc = {0};
		sf_graphics_default_init_render_target_description(&desc);

		desc.sample_count = r->sample_count;
		desc.color_format = r->color_attachment_format;
		desc.depth_stencil_format = r->depth_stencil_format;
		desc.width = r->swapchain_width;
		desc.height = r->swapchain_height;

		desc.color_attachment_clear_value_count = 1;

		sf_graphics_copy_clear_value(&desc.color_attachment_clear_values[0], &r->swapchain_color_clear_value);
		sf_graphics_copy_clear_value(&desc.depth_stencil_attachment_clear_value, &r->swapchain_depth_stencil_clear_value);

		desc.vk_not_owned_color_image = r->vk_swapchain_images[i];

		r->swapchain_render_targets[i] = sf_graphics_create_render_target(r, &desc);
		if (!r->swapchain_render_targets[i])
			return;
	}

	r->swapchain_render_target_count = swapchain_image_count;
}

SF_INTERNAL void sf_graphics_create_draw_complete_semaphores(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;

	r->draw_complete_semaphore_count = 0;

	for (i = 0; i < r->swapchain_image_count; ++i) {
		r->draw_complete_semaphores[i] = sf_graphics_create_semaphore(r);
		if (!r->draw_complete_semaphores[i])
			return;
	}

	r->draw_complete_semaphore_count = r->swapchain_image_count;
}

SF_INTERNAL void sf_graphics_create_swapchain_resources(struct sf_graphics_renderer *r) {
	sf_graphics_vulkan_create_swapchain(r);
	if (!r->vk_swapchain)
		return;

	sf_graphics_vulkan_load_swapchain_images(r);
	if (!r->vk_swapchain_image_count)
		return;

	sf_graphics_create_swapchain_render_targets(r);
	if (!r->swapchain_render_target_count)
		return;

	sf_graphics_create_draw_complete_semaphores(r);
	if (!r->draw_complete_semaphore_count)
		return;
}

sf_bool sf_graphics_begin_command(struct sf_graphics_renderer *r, sf_handle command_buffer) {
	VkCommandBufferBeginInfo info = {0};
	struct sf_graphics_command_buffer *c = (struct sf_graphics_command_buffer *)command_buffer;

	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.pInheritanceInfo = NULL;

	return SF_VULKAN_CHECK(vkBeginCommandBuffer(c->vk_command_buffer, &info));
}

sf_bool sf_graphics_end_command(struct sf_graphics_renderer *r, sf_handle command_buffer) {
	struct sf_graphics_command_buffer *c = (struct sf_graphics_command_buffer *)command_buffer;
	return SF_VULKAN_CHECK(vkEndCommandBuffer(c->vk_command_buffer));
}

sf_bool sf_graphics_queue_submit_command(struct sf_graphics_renderer *r, sf_handle queue, sf_u32 command_buffer_count, sf_handle *command_buffers, sf_u32 wait_semaphore_count, sf_handle *wait_semaphores, sf_u32 signal_semaphore_count, sf_handle *signal_semaphores) {
	sf_u32 i = 0;
	VkSubmitInfo info = {0};
	VkCommandBuffer vk_command_buffers[SF_GRAPHICS_MAX_COMMAND_BUFFER_SUBMIT_COUNT] = {0};
	VkSemaphore vk_wait_semaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};
	VkPipelineStageFlags vk_stage_flags[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};
	VkSemaphore vk_signal_semaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};

	struct sf_graphics_queue *q = (struct sf_graphics_queue *)queue;

	if (command_buffer_count > SF_SIZE(vk_command_buffers))
		return SF_FALSE;

	if (wait_semaphore_count > SF_SIZE(vk_wait_semaphores))
		return SF_FALSE;

	if (signal_semaphore_count > SF_SIZE(vk_signal_semaphores))
		return SF_FALSE;

	for (i = 0; i < command_buffer_count; i++)
		vk_command_buffers[i] = ((struct sf_graphics_command_buffer *)command_buffers[i])->vk_command_buffer;

	for (i = 0; i < wait_semaphore_count; i++) {
		vk_wait_semaphores[i] = ((struct sf_graphics_semaphore *)wait_semaphores[i])->vk_semaphore;
		vk_stage_flags[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}

	for (i = 0; i < signal_semaphore_count; i++) {
		struct sf_graphics_semaphore *s = (struct sf_graphics_semaphore *)signal_semaphores[i];
		vk_signal_semaphores[i] = s->vk_semaphore;
	}

	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = NULL;
	info.waitSemaphoreCount = wait_semaphore_count;
	info.pWaitSemaphores = vk_wait_semaphores;
	info.pWaitDstStageMask = vk_stage_flags;
	info.commandBufferCount = command_buffer_count;
	info.pCommandBuffers = vk_command_buffers;
	info.signalSemaphoreCount = signal_semaphore_count;
	info.pSignalSemaphores = vk_signal_semaphores;

	return SF_VULKAN_CHECK(vkQueueSubmit(q->vk_queue, 1, &info, VK_NULL_HANDLE));
}

sf_bool sf_graphics_queue_present(struct sf_graphics_renderer *r, sf_handle queue, sf_u32 wait_semaphore_count, sf_handle *wait_semaphores) {
	sf_u32 i = 0;
	VkPresentInfoKHR info = {0};
	VkSemaphore vk_wait_semaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};
	struct sf_graphics_queue *q = (struct sf_graphics_queue *)queue;

	if (wait_semaphore_count > SF_SIZE(vk_wait_semaphores))
		return SF_FALSE;

	for (i = 0; i < wait_semaphore_count; i++) 
		vk_wait_semaphores[i] = ((struct sf_graphics_semaphore *)wait_semaphores[i])->vk_semaphore;

	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.pNext = NULL;
	info.waitSemaphoreCount = wait_semaphore_count;
	info.pWaitSemaphores = vk_wait_semaphores;
	info.swapchainCount = 1;
	info.pSwapchains = &r->vk_swapchain;
	info.pImageIndices = &r->swapchain_current_image_index;
	info.pResults = NULL;

	return SF_VULKAN_CHECK(vkQueuePresentKHR(q->vk_queue, &info));
}

sf_bool sf_graphics_queue_wait_idle(sf_handle queue) {
	struct sf_graphics_queue *q = (struct sf_graphics_queue *)queue;
	return SF_VULKAN_CHECK(vkQueueWaitIdle(q->vk_queue));
}

void sf_graphics_destroy_texture(struct sf_graphics_renderer *r, sf_handle texture) {
	struct sf_graphics_texture *t = (struct sf_graphics_texture *)texture;

	if (!t)
		return;

	SF_QUEUE_REMOVE(&t->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_texture_queue, &t->queue);

	if (t->vk_image_view) {
		vkDestroyImageView(r->vk_device, t->vk_image_view, r->vk_allocation_callbacks);
		t->vk_image_view = VK_NULL_HANDLE;
	}

	if (t->owns_image && t->vk_memory) {
		vkFreeMemory(r->vk_device, t->vk_memory, r->vk_allocation_callbacks);
		t->vk_memory = VK_NULL_HANDLE;
	}

	if (!t->owns_image && t->vk_image) {
		vkDestroyImage(r->vk_device, t->vk_image, r->vk_allocation_callbacks);
		t->vk_image = VK_NULL_HANDLE;
	}
}

SF_INTERNAL void sf_graphics_default_init_semaphore(struct sf_graphics_semaphore *s) {
	SF_QUEUE_INIT(&s->queue);
	s->vk_semaphore = VK_NULL_HANDLE;
}

SF_INTERNAL struct sf_graphics_semaphore *sf_graphics_get_or_allocate_semaphore(struct sf_graphics_renderer *r) {
	struct sf_graphics_semaphore *s = NULL;

	if (SF_QUEUE_IS_EMPTY(&r->free_semaphore_queue)) {
		s = sf_allocate(&r->arena, sizeof(*s));
		if (!s)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_semaphore_queue);
		SF_QUEUE_REMOVE(q);
		s = SF_QUEUE_DATA(q, struct sf_graphics_semaphore, queue);
	}
	sf_graphics_default_init_semaphore(s);
	SF_QUEUE_INSERT_HEAD(&r->semaphore_queue, &s->queue);
	return s;
}

void sf_graphics_destroy_semaphore(struct sf_graphics_renderer *r, sf_handle semaphore) {
	struct sf_graphics_semaphore *s = (struct sf_graphics_semaphore *)semaphore;

	if (!s)
		return;

	SF_QUEUE_REMOVE(&s->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_semaphore_queue, &s->queue);

	if (r->vk_device && s->vk_semaphore) {
		vkDestroySemaphore(r->vk_device, s->vk_semaphore, r->vk_allocation_callbacks);
		s->vk_semaphore = VK_NULL_HANDLE;
	}
}

sf_handle sf_graphics_create_semaphore(struct sf_graphics_renderer *r) {
	VkSemaphoreCreateInfo info = {0};
	struct sf_graphics_semaphore *s = sf_graphics_get_or_allocate_semaphore(r);
	if (!s)
		return SF_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;

	if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vk_device, &info, r->vk_allocation_callbacks, &s->vk_semaphore)))
		goto error;

	return SF_AS_HANDLE(s);

error:
	sf_graphics_destroy_semaphore(r, SF_AS_HANDLE(s));

	return SF_NULL_HANDLE;
}

SF_INTERNAL void sf_graphics_default_init_fence(struct sf_graphics_fence *f) {
	SF_QUEUE_INIT(&f->queue);
	f->vk_fence = VK_NULL_HANDLE;
}

SF_INTERNAL struct sf_graphics_fence *sf_graphics_get_or_allocate_fence(struct sf_graphics_renderer *r) {
	struct sf_graphics_fence *f = NULL;

	if (SF_QUEUE_IS_EMPTY(&r->free_fence_queue)) {
		f = sf_allocate(&r->arena, sizeof(*f));
		if (!f)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_fence_queue);
		SF_QUEUE_REMOVE(q);
		f = SF_QUEUE_DATA(q, struct sf_graphics_fence, queue);
	}
	sf_graphics_default_init_fence(f);
	SF_QUEUE_INSERT_HEAD(&r->fence_queue, &f->queue);
	return f;
}

void sf_graphics_destroy_fence(struct sf_graphics_renderer *r, sf_handle fence) {
	struct sf_graphics_fence *f = (struct sf_graphics_fence *)fence;

	if (!r || !f)
		return;

	SF_QUEUE_REMOVE(&f->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_fence_queue, &f->queue);

	if (r->vk_device && f->vk_fence) {
		vkDestroyFence(r->vk_device, f->vk_fence, r->vk_allocation_callbacks);
		f->vk_fence = VK_NULL_HANDLE;
	}
}

sf_handle sf_graphics_create_fence(struct sf_graphics_renderer *r) {
	VkFenceCreateInfo info = {0};
	struct sf_graphics_fence *f = sf_graphics_get_or_allocate_fence(r);
	if (!f)
		return SF_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (!SF_VULKAN_CHECK(vkCreateFence(r->vk_device, &info, r->vk_allocation_callbacks, &f->vk_fence)))
		goto error;

	return SF_AS_HANDLE(f);

error:
	sf_graphics_destroy_fence(r, SF_AS_HANDLE(f));

	return SF_NULL_HANDLE;
}

SF_INTERNAL void sf_graphics_default_init_command_pool(struct sf_graphics_command_pool *p) {
	SF_QUEUE_INIT(&p->queue);
	p->vk_command_pool = VK_NULL_HANDLE;
}

SF_INTERNAL struct sf_graphics_command_pool *sf_graphics_get_or_allocate_command_pool(struct sf_graphics_renderer *r) {
	struct sf_graphics_command_pool *p = NULL;

	if (SF_QUEUE_IS_EMPTY(&r->free_command_pool_queue)) {
		p = sf_allocate(&r->arena, sizeof(*p));
		if (!p)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_command_pool_queue);
		SF_QUEUE_REMOVE(q);
		p = SF_QUEUE_DATA(q, struct sf_graphics_command_pool, queue);
	}
	sf_graphics_default_init_command_pool(p);
	SF_QUEUE_INSERT_HEAD(&r->command_pool_queue, &p->queue);
	return p;
}

void sf_graphics_destroy_command_pool(struct sf_graphics_renderer *r, sf_handle command_pool) {
	struct sf_graphics_command_pool *p = (struct sf_graphics_command_pool *)command_pool;

	if (!p)
		return;

	SF_QUEUE_REMOVE(&p->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_command_pool_queue, &p->queue);

	if (r->vk_device && p->vk_command_pool) {
		vkDestroyCommandPool(r->vk_device, p->vk_command_pool, r->vk_allocation_callbacks);
		p->vk_command_pool = VK_NULL_HANDLE;
	}
}

sf_handle sf_graphics_create_command_pool(struct sf_graphics_renderer *r, sf_handle queue, sf_bool transient, sf_bool reset) {
	VkCommandPoolCreateInfo info = {0};

	struct sf_graphics_command_pool *p = sf_graphics_get_or_allocate_command_pool(r);
	struct sf_graphics_queue *q = (struct sf_graphics_queue *)queue;

	if (!p)
		return SF_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;

	if (transient)
		info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (reset)
		info.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	info.queueFamilyIndex = q->vk_queue_family_index;

	if (!SF_VULKAN_CHECK(vkCreateCommandPool(r->vk_device, &info, r->vk_allocation_callbacks, &p->vk_command_pool)))
		goto error;

	return SF_AS_HANDLE(p);

error:
	sf_graphics_destroy_command_pool(r, SF_AS_HANDLE(p));

	return SF_NULL_HANDLE;
}

SF_INTERNAL void sf_graphics_default_init_command_buffer(struct sf_graphics_command_buffer *p) {
	SF_QUEUE_INIT(&p->queue);
	p->vk_command_buffer = VK_NULL_HANDLE;
}

SF_INTERNAL struct sf_graphics_command_buffer *sf_graphics_get_or_allocate_command_buffer(struct sf_graphics_renderer *r) {
	struct sf_graphics_command_buffer *cb = NULL;

	if (SF_QUEUE_IS_EMPTY(&r->free_command_buffer_queue)) {
		cb = sf_allocate(&r->arena, sizeof(*cb));
		if (!cb)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_command_buffer_queue);
		SF_QUEUE_REMOVE(q);
		cb = SF_QUEUE_DATA(q, struct sf_graphics_command_buffer, queue);
	}
	sf_graphics_default_init_command_buffer(cb);
	SF_QUEUE_INSERT_HEAD(&r->command_buffer_queue, &cb->queue);
	return cb;
}

void sf_graphics_destroy_command_buffer(struct sf_graphics_renderer *r, sf_handle command_pool, sf_handle command_buffer) {
	struct sf_graphics_command_buffer *cb = (struct sf_graphics_command_buffer *)command_buffer;
	struct sf_graphics_command_pool *p = (struct sf_graphics_command_pool *)command_pool;

	if (!cb)
		return;

	SF_QUEUE_REMOVE(&cb->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_command_pool_queue, &cb->queue);

	if (r->vk_device && p->vk_command_pool && cb->vk_command_buffer) {
		vkFreeCommandBuffers(r->vk_device, p->vk_command_pool, 1, &cb->vk_command_buffer);
		cb->vk_command_buffer = VK_NULL_HANDLE;
	}
}

sf_handle sf_graphics_create_command_buffer(struct sf_graphics_renderer *r, sf_handle command_pool, sf_bool secondary) {
	VkCommandBufferAllocateInfo info = {0};
	struct sf_graphics_command_pool *p = (struct sf_graphics_command_pool *)command_pool;
	struct sf_graphics_command_buffer *cb = sf_graphics_get_or_allocate_command_buffer(r);

	if (!cb)
		return SF_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = NULL;
	info.commandPool = p->vk_command_pool;

	if (!secondary)
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	else
		info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	info.commandBufferCount = 1;

	if (!SF_VULKAN_CHECK(vkAllocateCommandBuffers(r->vk_device, &info, &cb->vk_command_buffer)))
		goto error;

	return SF_AS_HANDLE(cb);

error:
	sf_graphics_destroy_command_buffer(r, command_pool, SF_AS_HANDLE(cb));

	return SF_NULL_HANDLE;
}

SF_INTERNAL VkImageType sf_graphics_vulkan_as_vulkan_image_type(enum sf_graphics_texture_type type) {
	switch (type) {
		case SF_GRAPHICS_TEXTURE_TYPE_1D: return VK_IMAGE_TYPE_1D;
		case SF_GRAPHICS_TEXTURE_TYPE_2D: return VK_IMAGE_TYPE_2D;
		case SF_GRAPHICS_TEXTURE_TYPE_3D: return VK_IMAGE_TYPE_3D;
		case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_TYPE_2D;
	}
}

SF_INTERNAL VkFormat sf_graphics_vulkan_as_vulkan_format(enum sf_graphics_format format) {
	switch (format) {
		case SF_GRAPHICS_FORMAT_UNDEFINED: return VK_FORMAT_UNDEFINED;
		case SF_GRAPHICS_FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
		case SF_GRAPHICS_FORMAT_R16_UNORM: return VK_FORMAT_R16_UNORM;
		case SF_GRAPHICS_FORMAT_R16_UINT: return VK_FORMAT_R16_UINT;
		case SF_GRAPHICS_FORMAT_R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32_UINT: return VK_FORMAT_R32_UINT;
		case SF_GRAPHICS_FORMAT_R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
		case SF_GRAPHICS_FORMAT_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
		case SF_GRAPHICS_FORMAT_R8G8B8_UNORM: return VK_FORMAT_R8G8B8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16_UNORM: return VK_FORMAT_R16G16B16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT: return VK_FORMAT_R16G16B16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
		case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case SF_GRAPHICS_FORMAT_D16_UNORM: return VK_FORMAT_D16_UNORM;
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: return VK_FORMAT_X8_D24_UNORM_PACK32;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
		case SF_GRAPHICS_FORMAT_S8_UINT: return VK_FORMAT_S8_UINT;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT: return VK_FORMAT_D16_UNORM_S8_UINT;
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default: return VK_FORMAT_UNDEFINED;
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
		case SF_GRAPHICS_SAMPLE_COUNT_1: return VK_SAMPLE_COUNT_1_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_2: return VK_SAMPLE_COUNT_2_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_4: return VK_SAMPLE_COUNT_4_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_8: return VK_SAMPLE_COUNT_8_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_16: return VK_SAMPLE_COUNT_16_BIT;
	}
}

SF_INTERNAL VkImageViewType sf_graphics_vulkan_as_image_view_type(enum sf_graphics_texture_type type) {
	switch (type) {
		case SF_GRAPHICS_TEXTURE_TYPE_1D: return VK_IMAGE_VIEW_TYPE_1D;
		case SF_GRAPHICS_TEXTURE_TYPE_2D: return VK_IMAGE_VIEW_TYPE_2D;
		case SF_GRAPHICS_TEXTURE_TYPE_3D: return VK_IMAGE_VIEW_TYPE_3D;
		case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
	}
}

SF_INTERNAL VkImageAspectFlags sf_graphics_vulkan_find_aspect_flags(VkFormat format) {
	switch (format) {
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;

		case VK_FORMAT_S8_UINT: return VK_IMAGE_ASPECT_STENCIL_BIT;

		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

		default: return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

sf_handle sf_graphics_create_texture(struct sf_graphics_renderer *r, struct sf_graphics_texture_description *desc) {
	VkImageCreateInfo image_info = {0};
	VkImageViewCreateInfo image_view_info = {0};

	struct sf_graphics_texture *t = sf_graphics_get_or_allocate_texture(r);
	if (!t)
		goto error;

	t->type = desc->type;
	t->sample_count = desc->sample_count;
	t->format = desc->format;
	t->usage = desc->usage;
	t->width = desc->width;
	t->height = desc->height;
	t->depth = desc->depth;
	t->mips = desc->mips;
	t->mapped = desc->mapped;
	sf_graphics_copy_clear_value(&t->clear_value, &desc->clear_value);

	if (desc->vk_not_owned_image) {
		t->owns_image = SF_FALSE;
		t->vk_image = desc->vk_not_owned_image;
		t->mapped = SF_FALSE;
	} else {

		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = NULL;
		image_info.flags = 0;
		image_info.imageType = sf_graphics_vulkan_as_vulkan_image_type(t->type);
		image_info.format = sf_graphics_vulkan_as_vulkan_format(t->format);
		image_info.extent.width = t->width;
		image_info.extent.height = t->height;
		image_info.extent.depth = t->depth;
		image_info.mipLevels = t->mips;
		image_info.arrayLayers = 1;
		image_info.samples = sf_graphics_vulkan_as_vulkan_sample_count(t->sample_count);
		image_info.tiling = t->mapped ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = sf_graphics_vulkan_as_vulkan_usage(t->usage);
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.queueFamilyIndexCount = 0;
		image_info.pQueueFamilyIndices = NULL;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// TODO(samsal): check limits
		if (!SF_VULKAN_CHECK(vkCreateImage(r->vk_device, &image_info, r->vk_allocation_callbacks, &t->vk_image)))
			goto error;

		t->vk_memory = sf_graphics_vulkan_allocate_and_bind_memory_to_image(r, t->vk_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | (t->mapped ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0));
		if (!t->vk_memory)
			goto error;

		if (t->mapped && !SF_VULKAN_CHECK(vkMapMemory(r->vk_device, t->vk_memory, 0, VK_WHOLE_SIZE, 0, &t->mapped_data)))
			goto error;
	}

	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.pNext = NULL;
	image_view_info.flags = 0;
	image_view_info.image = t->vk_image;
	image_view_info.viewType = sf_graphics_vulkan_as_image_view_type(t->type);
	image_view_info.format = sf_graphics_vulkan_as_vulkan_format(t->format);
	image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	image_view_info.subresourceRange.aspectMask = sf_graphics_vulkan_find_aspect_flags(sf_graphics_vulkan_as_vulkan_format(t->format));
	image_view_info.subresourceRange.baseMipLevel = 0;
	image_view_info.subresourceRange.levelCount = t->mips;
	image_view_info.subresourceRange.baseArrayLayer = 0;
	image_view_info.subresourceRange.layerCount = 1;

	if (!SF_VULKAN_CHECK(vkCreateImageView(r->vk_device, &image_view_info, r->vk_allocation_callbacks, &t->vk_image_view)))
		goto error;

	t->vk_aspect = image_view_info.subresourceRange.aspectMask;
	t->vk_layout = (SF_GRAPHICS_TEXTURE_USAGE_STORAGE & t->usage) == SF_GRAPHICS_TEXTURE_USAGE_STORAGE ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	return SF_AS_HANDLE(t);

error:
	sf_graphics_destroy_texture(r, SF_AS_HANDLE(t));

	return SF_NULL_HANDLE;
}

SF_INTERNAL sf_u32 sf_graphics_calculate_mip_levels(sf_u32 width, sf_u32 height) {
	sf_u32 result = 1;

	if (width == 0 || height == 0)
		return 0;

	while (width > 1 || height > 1) {
		width >>= 1;
		height >>= 1;
		result++;
	}

	return result;
}

SF_INTERNAL void sf_graphics_default_init_render_target(struct sf_graphics_render_target *t) {
	sf_u32 i = 0;

	SF_QUEUE_INIT(&t->queue);

	t->sample_count = SF_GRAPHICS_SAMPLE_COUNT_1;
	t->color_format = SF_GRAPHICS_FORMAT_UNDEFINED;
	t->depth_stencil_format = SF_GRAPHICS_FORMAT_UNDEFINED;
	t->width = 0;
	t->height = 0;
	t->color_attachment_clear_value_count = 0;

	for (i = 0; i < SF_SIZE(t->color_attachment_clear_values); ++i)
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

SF_INTERNAL struct sf_graphics_render_target *sf_graphics_get_or_allocate_render_target(struct sf_graphics_renderer *r) {
	struct sf_graphics_render_target *t = NULL;

	if (SF_QUEUE_IS_EMPTY(&r->free_render_target_queue)) {
		t = sf_allocate(&r->arena, sizeof(*t));
		if (!t)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_render_target_queue);
		SF_QUEUE_REMOVE(q);
		t = SF_QUEUE_DATA(q, struct sf_graphics_render_target, queue);
	}
	sf_graphics_default_init_render_target(t);
	SF_QUEUE_INSERT_HEAD(&r->render_target_queue, &t->queue);
	return t;
}

void sf_graphics_destroy_render_target(struct sf_graphics_renderer *r, sf_handle render_target) {
	sf_u32 i = 0;
	struct sf_graphics_render_target *t = (struct sf_graphics_render_target *)render_target;

	if (!t)
		return;

	SF_QUEUE_REMOVE(&t->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_texture_queue, &t->queue);

	if (r->vk_device && t->vk_framebuffer) {
		vkDestroyFramebuffer(r->vk_device, t->vk_framebuffer, r->vk_allocation_callbacks);
		t->vk_framebuffer = VK_NULL_HANDLE;
	}

	if (r->vk_device && t->vk_render_pass) {
		vkDestroyRenderPass(r->vk_device, t->vk_render_pass, r->vk_allocation_callbacks);
		t->vk_render_pass = VK_NULL_HANDLE;
	}

	sf_graphics_destroy_texture(r, t->depth_stencil_multisampling_attachment);
	sf_graphics_destroy_texture(r, t->depth_stencil_attachment);

	for (i = 0; i < SF_SIZE(t->color_multisample_attachments); ++i) {
		sf_graphics_destroy_texture(r, t->color_multisample_attachments[i]);
		t->color_multisample_attachments[i] = SF_NULL_HANDLE;
	}
	t->color_multisample_attachment_count = 0;

	for (i = 0; i < SF_SIZE(t->color_attachments); ++i) {
		sf_graphics_destroy_texture(r, t->color_attachments[i]);
		t->color_attachments[i] = SF_NULL_HANDLE;
	}
	t->color_attachment_count = 0;
}

sf_handle sf_graphics_create_render_target(struct sf_graphics_renderer *r, struct sf_graphics_render_target_description *desc) {
	struct sf_arena *arena = NULL;
	sf_u32 i = 0, total_attachment_count = 0;

	VkSubpassDescription subpass_description = {0};
	VkSubpassDependency subpass_dependency = {0};
	VkRenderPassCreateInfo render_pass_info = {0};
	VkFramebufferCreateInfo framebuffer_info = {0};

	VkAttachmentDescription *attachment_descriptions = NULL;
	VkAttachmentReference *color_attachment_references = NULL;
	VkAttachmentReference *color_resolve_attachment_references = NULL;
	VkAttachmentReference depth_stencil_reference = {0};
	VkImageView *attachment_image_views = NULL;

	struct sf_graphics_render_target *t = sf_graphics_get_or_allocate_render_target(r);
	if (!t)
		goto error;

	arena = &r->render_target_arena;

	t->sample_count = desc->sample_count;
	t->color_format = desc->color_format;
	t->depth_stencil_format = desc->depth_stencil_format;
	t->width = desc->width;
	t->height = desc->height;

	if (t->color_attachment_clear_value_count > SF_SIZE(t->color_attachment_clear_values))
		goto error;

	for (i = 0; i < desc->color_attachment_clear_value_count; ++i)
		sf_graphics_copy_clear_value(&t->color_attachment_clear_values[i], &desc->color_attachment_clear_values[i]);

	sf_graphics_copy_clear_value(&t->depth_stencil_attachment_clear_value, &desc->depth_stencil_attachment_clear_value);
	t->vk_swapchain_image = desc->vk_not_owned_color_image;

	if (t->sample_count == SF_GRAPHICS_SAMPLE_COUNT_1) {
		total_attachment_count = t->color_attachment_count + t->depth_stencil_format == SF_GRAPHICS_FORMAT_UNDEFINED ? 0 : 1;

		attachment_descriptions = sf_allocate(arena, total_attachment_count * sizeof(*attachment_descriptions));
		if (!attachment_descriptions)
			goto error;

		color_attachment_references = sf_allocate(arena, t->color_attachment_count * sizeof(*color_attachment_references));
		if (!color_attachment_references)
			goto error;

		attachment_image_views = sf_allocate(arena, total_attachment_count * sizeof(*attachment_image_views));
		if (!attachment_image_views)
			goto error;

		for (i = 0; i < desc->color_attachment_clear_value_count; ++i) {
			struct sf_graphics_texture_description texture_desc = {0};
			sf_u32 color_index = i;
			struct sf_graphics_clear_value *clear_value = &desc->color_attachment_clear_values[i];

			texture_desc.type = SF_GRAPHICS_TEXTURE_TYPE_2D;
			texture_desc.width = t->width;
			texture_desc.height = t->height;
			texture_desc.sample_count = t->sample_count;
			texture_desc.format = t->color_format;
			texture_desc.mips = 1;
			texture_desc.mapped = SF_FALSE;
			texture_desc.usage = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED;
			texture_desc.vk_not_owned_image = t->vk_swapchain_image;
			sf_graphics_copy_clear_value(&texture_desc.clear_value, clear_value);

			t->color_attachments[i] = sf_graphics_create_texture(r, &texture_desc);
			if (t->color_attachments[i])
				goto error;

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

			attachment_image_views[color_index] = ((struct sf_graphics_texture *)t->color_attachments[i])->vk_image_view;
		}
		t->color_attachment_count = desc->color_attachment_clear_value_count;
	} else {
		total_attachment_count = 2 * t->color_attachment_count + t->depth_stencil_format == SF_GRAPHICS_FORMAT_UNDEFINED ? 0 : 1;

		attachment_descriptions = sf_allocate(arena, total_attachment_count * sizeof(*attachment_descriptions));
		if (!attachment_descriptions)
			goto error;

		color_attachment_references = sf_allocate(arena, t->color_attachment_count * sizeof(*color_attachment_references));
		if (!color_attachment_references)
			goto error;

		color_resolve_attachment_references = sf_allocate(arena, t->color_attachment_count * sizeof(*color_attachment_references));
		if (!color_resolve_attachment_references)
			goto error;

		attachment_image_views = sf_allocate(arena, total_attachment_count * sizeof(*attachment_image_views));
		if (!attachment_image_views)
			goto error;

		for (i = 0; i < t->color_attachment_count; ++i) {
			struct sf_graphics_texture_description color_texture_desc = {0};
			struct sf_graphics_texture_description resolve_texture_desc = {0};
			sf_u32 color_index = 2 * i;
			sf_u32 resolve_index = color_index + 1;
			struct sf_graphics_clear_value *clear_value = &desc->color_attachment_clear_values[i];

			color_texture_desc.type = SF_GRAPHICS_TEXTURE_TYPE_2D;
			color_texture_desc.width = t->width;
			color_texture_desc.height = t->height;
			color_texture_desc.sample_count = t->sample_count;
			color_texture_desc.format = t->color_format;
			color_texture_desc.mips = 1;
			color_texture_desc.mapped = SF_FALSE;
			color_texture_desc.usage = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED;
			color_texture_desc.vk_not_owned_image = t->vk_swapchain_image;
			sf_graphics_copy_clear_value(&color_texture_desc.clear_value, clear_value);

			resolve_texture_desc.type = SF_GRAPHICS_TEXTURE_TYPE_2D;
			resolve_texture_desc.width = t->width;
			resolve_texture_desc.height = t->height;
			resolve_texture_desc.sample_count = t->sample_count;
			resolve_texture_desc.format = t->color_format;
			resolve_texture_desc.mips = 1;
			resolve_texture_desc.mapped = SF_FALSE;
			resolve_texture_desc.usage = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED;
			resolve_texture_desc.vk_not_owned_image = VK_NULL_HANDLE;
			sf_graphics_copy_clear_value(&resolve_texture_desc.clear_value, clear_value);

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

			attachment_image_views[color_index] = ((struct sf_graphics_texture *)t->color_attachments[i])->vk_image_view;
			attachment_image_views[resolve_index] = ((struct sf_graphics_texture *)t->color_multisample_attachments[i])->vk_image_view;
		}
		t->color_attachment_count = desc->color_attachment_clear_value_count;
		t->color_multisample_attachment_count = desc->color_attachment_clear_value_count;
	}

	if (t->depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED) {
		struct sf_graphics_texture_description texture_desc = {0};
		sf_u32 depth_stencil_index = total_attachment_count - 1;

		texture_desc.type = SF_GRAPHICS_TEXTURE_TYPE_2D;
		texture_desc.width = t->width;
		texture_desc.height = t->height;
		texture_desc.sample_count = t->sample_count;
		texture_desc.format = t->depth_stencil_format;
		texture_desc.mips = 1;
		texture_desc.mapped = SF_FALSE;
		texture_desc.usage = SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT | SF_GRAPHICS_TEXTURE_USAGE_SAMPLED;
		texture_desc.vk_not_owned_image = VK_NULL_HANDLE;
		sf_graphics_copy_clear_value(&texture_desc.clear_value, &desc->depth_stencil_attachment_clear_value);

		t->depth_stencil_attachment = sf_graphics_create_texture(r, &texture_desc);
		if (!t->depth_stencil_attachment)
			goto error;

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

		attachment_image_views[depth_stencil_index] = ((struct sf_graphics_texture *)t->depth_stencil_attachment)->vk_image_view;
	}

	subpass_description.flags = 0;
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.inputAttachmentCount = 0;
	subpass_description.pInputAttachments = NULL;
	subpass_description.colorAttachmentCount = t->color_attachment_count;
	subpass_description.pColorAttachments = color_attachment_references;
	subpass_description.pResolveAttachments = color_resolve_attachment_references;
	subpass_description.pDepthStencilAttachment = t->depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED ? &depth_stencil_reference : NULL;
	subpass_description.preserveAttachmentCount = 0;
	subpass_description.pPreserveAttachments = NULL;

	subpass_dependency.srcSubpass = 0;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pNext = NULL;
	render_pass_info.flags = 0;
	render_pass_info.attachmentCount = total_attachment_count;
	render_pass_info.pAttachments = attachment_descriptions;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_description;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &subpass_dependency;

	if (!SF_VULKAN_CHECK(vkCreateRenderPass(r->vk_device, &render_pass_info, r->vk_allocation_callbacks, &t->vk_render_pass)))
		goto error;

	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.pNext = NULL;
	framebuffer_info.flags = 0;
	framebuffer_info.renderPass = t->vk_render_pass;
	framebuffer_info.attachmentCount = total_attachment_count;
	framebuffer_info.pAttachments = attachment_image_views;
	framebuffer_info.width = t->width;
	framebuffer_info.height = t->height;
	framebuffer_info.layers = 1;

	if (!SF_VULKAN_CHECK(vkCreateFramebuffer(r->vk_device, &framebuffer_info, r->vk_allocation_callbacks, &t->vk_framebuffer)))
		goto error;

	sf_arena_clear(arena);

	return SF_AS_HANDLE(t);

error:
	sf_arena_clear(arena);
	sf_graphics_destroy_render_target(r, SF_AS_HANDLE(t));

	return SF_NULL_HANDLE;
}

SF_INTERNAL void sf_graphics_default_init_program(struct sf_graphics_program *p) {
	SF_QUEUE_INIT(&p->queue);
	p->stages = 0;
	p->vk_vertex_shader = VK_NULL_HANDLE;
	p->vk_tesselation_control_shader = VK_NULL_HANDLE;
	p->vk_tesselation_evaluation_shader = VK_NULL_HANDLE;
	p->vk_geometry_shader = VK_NULL_HANDLE;
	p->vk_compute_shader = VK_NULL_HANDLE;
	p->vk_fragment_shader = VK_NULL_HANDLE;
}

SF_INTERNAL struct sf_graphics_program *sf_graphics_get_or_allocate_program(struct sf_graphics_renderer *r) {
	struct sf_graphics_program *p = NULL;

	if (SF_QUEUE_IS_EMPTY(&r->free_program_queue)) {
		p = sf_allocate(&r->arena, sizeof(*p));
		if (!p)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_program_queue);
		SF_QUEUE_REMOVE(q);
		p = SF_QUEUE_DATA(q, struct sf_graphics_program, queue);
	}
	sf_graphics_default_init_program(p);
	SF_QUEUE_INSERT_HEAD(&r->program_queue, &p->queue);
	return p;
}

SF_INTERNAL VkShaderModule sf_graphics_vulkan_create_shader_module(struct sf_graphics_renderer *r, sf_u32 code_size_in_bytes, void const *code) {
	VkShaderModuleCreateInfo info = {0};
	VkShaderModule shader = {0};

	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.codeSize = code_size_in_bytes;
	info.pCode = code;

	if (SF_VULKAN_CHECK(vkCreateShaderModule(r->vk_device, &info, r->vk_allocation_callbacks, &shader)))
		return shader;

	return VK_NULL_HANDLE;
}

void sf_graphics_destroy_program(struct sf_graphics_renderer *r, sf_handle program) {
	struct sf_graphics_program *p = (struct sf_graphics_program *)program;

	if (!p)
		return;

	SF_QUEUE_REMOVE(&p->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_program_queue, &p->queue);

	if (r->vk_device && p->vk_compute_shader) {
		vkDestroyShaderModule(r->vk_device, p->vk_compute_shader, r->vk_allocation_callbacks);
		p->vk_compute_shader = VK_NULL_HANDLE;
	}

	if (r->vk_device && p->vk_fragment_shader) {
		vkDestroyShaderModule(r->vk_device, p->vk_fragment_shader, r->vk_allocation_callbacks);
		p->vk_fragment_shader = VK_NULL_HANDLE;
	}

	if (r->vk_device && p->vk_geometry_shader) {
		vkDestroyShaderModule(r->vk_device, p->vk_geometry_shader, r->vk_allocation_callbacks);
		p->vk_geometry_shader = VK_NULL_HANDLE;
	}

	if (r->vk_device && p->vk_tesselation_evaluation_shader) {
		vkDestroyShaderModule(r->vk_device, p->vk_tesselation_evaluation_shader, r->vk_allocation_callbacks);
		p->vk_tesselation_evaluation_shader = VK_NULL_HANDLE;
	}

	if (r->vk_device && p->vk_tesselation_control_shader) {
		vkDestroyShaderModule(r->vk_device, p->vk_tesselation_control_shader, r->vk_allocation_callbacks);
		p->vk_tesselation_control_shader = VK_NULL_HANDLE;
	}

	if (r->vk_device && p->vk_vertex_shader) {
		vkDestroyShaderModule(r->vk_device, p->vk_vertex_shader, r->vk_allocation_callbacks);
		p->vk_vertex_shader = VK_NULL_HANDLE;
	}
}

sf_handle sf_graphics_create_program(struct sf_graphics_renderer *r, struct sf_graphics_program_description *desc) {
	struct sf_graphics_program *p = sf_graphics_get_or_allocate_program(r);
	if (!p)
		return SF_NULL_HANDLE;

	if (desc->vertex_code_size) {
		p->stages |= SF_GRAPHICS_SHADER_STAGE_VERTEX;
		p->vk_vertex_shader = sf_graphics_vulkan_create_shader_module(r, desc->vertex_code_size, desc->vertex_code);
		if (!p->vk_vertex_shader)
			goto error;
	}

	if (desc->tesselation_control_code_size) {
		p->stages |= SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL;
		p->vk_tesselation_control_shader = sf_graphics_vulkan_create_shader_module(r, desc->tesselation_control_code_size, desc->tesselation_control_code);
		if (!p->vk_tesselation_control_shader)
			goto error;
	}

	if (desc->tesselation_evaluation_code_size) {
		p->stages |= SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION;
		p->vk_tesselation_evaluation_shader = sf_graphics_vulkan_create_shader_module(r, desc->tesselation_evaluation_code_size, desc->tesselation_evaluation_code);
		if (!p->vk_tesselation_evaluation_shader)
			goto error;
	}

	if (desc->geometry_code_size) {
		p->stages |= SF_GRAPHICS_SHADER_STAGE_GEOMETRY;
		p->vk_geometry_shader = sf_graphics_vulkan_create_shader_module(r, desc->geometry_code_size, desc->geometry_code);
		if (!p->vk_geometry_shader)
			goto error;
	}

	if (desc->fragment_code_size) {
		p->stages |= SF_GRAPHICS_SHADER_STAGE_FRAGMENT;
		p->vk_fragment_shader = sf_graphics_vulkan_create_shader_module(r, desc->fragment_code_size, desc->fragment_code);
		if (!p->vk_fragment_shader)
			goto error;
	}

	if (desc->compute_code_size) {
		p->stages |= SF_GRAPHICS_SHADER_STAGE_COMPUTE;
		p->vk_compute_shader = sf_graphics_vulkan_create_shader_module(r, desc->compute_code_size, desc->compute_code);
		if (!p->vk_compute_shader)
			goto error;
	}

	return SF_AS_HANDLE(p);

error:
	sf_graphics_destroy_program(r, SF_AS_HANDLE(p));

	return SF_NULL_HANDLE;
}

SF_INTERNAL VkDescriptorType sf_graphics_vulkan_as_vulkan_descriptor_type(enum sf_graphics_descriptor_type type) {
	switch (type) {
		case SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
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

SF_INTERNAL void sf_graphics_default_init_descriptor(struct sf_graphics_descriptor *d) {
	d->type = SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER;
	d->stages = 0;
	d->binding = 0;
	d->entry_count = 0;
	SF_ARRAY_INIT(d->entries, SF_NULL_HANDLE);
}

SF_INTERNAL void sf_graphics_default_init_descriptor_set(struct sf_graphics_descriptor_set *ds) {
	sf_u32 i = 0;
	SF_QUEUE_INIT(&ds->queue);

	ds->descriptor_count = 0;
	for (i = 0; SF_SIZE(ds->descriptors); ++i)
		sf_graphics_default_init_descriptor(&ds->descriptors[i]);

	ds->vk_descriptor_set_layout = VK_NULL_HANDLE;
	ds->vk_descriptor_pool = VK_NULL_HANDLE;
	ds->vk_descriptor_set = VK_NULL_HANDLE;
}

SF_INTERNAL struct sf_graphics_descriptor_set *sf_graphics_get_or_allocate_descriptor_set(struct sf_graphics_renderer *r) {
	struct sf_graphics_descriptor_set *ds = 0;

	if (SF_QUEUE_IS_EMPTY(&r->free_descriptor_set_queue)) {
		ds = sf_allocate(&r->arena, sizeof(*ds));
		if (!ds)
			return NULL;
	} else {
		struct sf_queue *q = SF_QUEUE_HEAD(&r->free_descriptor_set_queue);
		SF_QUEUE_REMOVE(q);
		ds = SF_QUEUE_DATA(q, struct sf_graphics_descriptor_set, queue);
	}
	sf_graphics_default_init_descriptor_set(ds);
	SF_QUEUE_INSERT_HEAD(&r->descriptor_set_queue, &ds->queue);
	return ds;
}

void sf_graphics_destroy_descriptor_set(struct sf_graphics_renderer *r, sf_handle descriptor_set) {
	struct sf_graphics_descriptor_set *ds = (struct sf_graphics_descriptor_set *)descriptor_set;
	if (!r || !ds)
		return;

	SF_QUEUE_REMOVE(&ds->queue);
	SF_QUEUE_INSERT_HEAD(&r->free_program_queue, &ds->queue);

	if (r->vk_device && ds->vk_descriptor_pool && ds->vk_descriptor_set) {
		vkFreeDescriptorSets(r->vk_device, ds->vk_descriptor_pool, 1, &ds->vk_descriptor_set);
		ds->vk_descriptor_set = VK_NULL_HANDLE;
	}

	if (r->vk_device && ds->vk_descriptor_pool) {
		vkDestroyDescriptorPool(r->vk_device, ds->vk_descriptor_pool, r->vk_allocation_callbacks);
		ds->vk_descriptor_pool = VK_NULL_HANDLE;
	}

	if (r->vk_device && ds->vk_descriptor_set_layout) {
		vkDestroyDescriptorSetLayout(r->vk_device, ds->vk_descriptor_set_layout, r->vk_allocation_callbacks);
		ds->vk_descriptor_set_layout = VK_NULL_HANDLE;
	}
}

SF_INTERNAL void sf_graphics_copy_descriptor(struct sf_graphics_descriptor *dst, struct sf_graphics_descriptor *src) {
	SF_MEMORY_COPY(dst, src, sizeof(src));
}

sf_handle sf_graphics_create_descriptor_set(struct sf_graphics_renderer *r, struct sf_graphics_descriptor_set_description *desc) {
	sf_u32 i = 0;
	struct sf_graphics_descriptor_set *ds = NULL;
	VkDescriptorPoolCreateInfo pool_info = {0};
	VkDescriptorSetLayoutCreateInfo layout_info = {0};
	VkDescriptorSetAllocateInfo set_info = {0};
	sf_u64 descriptor_count_by_type[SF_GRAPHICS_DESCRIPTOR_TYPE_COUNT] = {0};
	VkDescriptorPoolSize pool_sizes[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT] = {0};
	VkDescriptorSetLayoutBinding bindings[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT] = {0};

	if (!r || !desc || desc->descriptor_count > SF_SIZE(desc->descriptors))
		return SF_NULL_HANDLE;

	ds = sf_graphics_get_or_allocate_descriptor_set(r);
	if (!ds)
		return SF_NULL_HANDLE;

	ds->descriptor_count = desc->descriptor_count;
	for (i = 0; i < ds->descriptor_count; ++i)
		sf_graphics_copy_descriptor(&ds->descriptors[i], &desc->descriptors[i]);

	for (i = 0; i < ds->descriptor_count; ++i)
		++descriptor_count_by_type[desc->descriptors[i].type];

	for (i = 0; i < ds->descriptor_count; ++i) {
		struct sf_graphics_descriptor *d = &ds->descriptors[i];

		pool_sizes[i].type = sf_graphics_vulkan_as_vulkan_descriptor_type(d->type);
		pool_sizes[i].descriptorCount = descriptor_count_by_type[d->type];

		bindings[i].binding = d->binding;
		bindings[i].descriptorType = sf_graphics_vulkan_as_vulkan_descriptor_type(d->type);
		bindings[i].descriptorCount = d->entry_count;
		bindings[i].stageFlags = sf_graphics_vulkan_as_vulkan_shader_stages(d->stages);
		bindings[i].pImmutableSamplers = NULL;
	}

	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.pNext = NULL;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1;
	pool_info.poolSizeCount = ds->descriptor_count;
	pool_info.pPoolSizes = pool_sizes;

	if (!SF_VULKAN_CHECK(vkCreateDescriptorPool(r->vk_device, &pool_info, r->vk_allocation_callbacks, &ds->vk_descriptor_pool)))
		goto error;

	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.pNext = NULL;
	layout_info.flags = 0;
	layout_info.bindingCount = ds->descriptor_count;
	layout_info.pBindings = bindings;

	if (!SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(r->vk_device, &layout_info, r->vk_allocation_callbacks, &ds->vk_descriptor_set_layout)))
		goto error;

	set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	set_info.pNext = NULL;
	set_info.descriptorPool = ds->vk_descriptor_pool;
	set_info.descriptorSetCount = 1;
	set_info.pSetLayouts = &ds->vk_descriptor_set_layout;

	if (!SF_VULKAN_CHECK(vkAllocateDescriptorSets(r->vk_device, &set_info, &ds->vk_descriptor_set)))
		goto error;

	return SF_AS_HANDLE(ds);

error:
	sf_graphics_destroy_descriptor_set(r, SF_AS_HANDLE(ds));
	return SF_NULL_HANDLE;
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

SF_INTERNAL void sf_graphics_create_image_acquired_semaphores(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;

	r->image_acquired_semaphore_count = 0;

	for (i = 0; i < r->buffering_count; ++i) {
		r->image_acquired_semaphores[i] = sf_graphics_create_semaphore(r);
		if (!r->image_acquired_semaphores[i])
			return;
	}

	r->image_acquired_semaphore_count = r->buffering_count;
}

SF_INTERNAL void sf_graphics_create_in_flight_fences(struct sf_graphics_renderer *r) {
	sf_u32 i = 0;

	r->in_flight_fence_count = 0;

	for (i = 0; i < r->buffering_count; ++i) {
		r->in_flight_fences[i] = sf_graphics_create_fence(r);
		if (!r->in_flight_fences[i])
			return;
	}

	r->in_flight_fence_count = r->buffering_count;
}

struct sf_graphics_renderer *sf_graphics_create_renderer(struct sf_graphics_renderer_description *desc) {
	struct sf_graphics_renderer *r = sf_allocate(desc->arena, sizeof(struct sf_graphics_renderer));
	if (!r)
		goto error;

	sf_graphics_default_init_renderer(r);

	r->api.os = desc->api.os;
	r->api.window = desc->api.window;

	r->swapchain_width = desc->swapchain_width;
	r->swapchain_height = desc->swapchain_height;
	r->swapchain_image_count = desc->swapchain_image_count;

	sf_graphics_copy_clear_value(&r->swapchain_color_clear_value, &desc->swapchain_color_clear_value);
	sf_graphics_copy_clear_value(&r->swapchain_depth_stencil_clear_value, &desc->swapchain_depth_stencil_clear_value);

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

	sf_scratch(desc->arena, 1024 * 128, &r->arena);
	if (!r->arena.data)
		goto error;

	sf_scratch(desc->arena, 1024 * 128, &r->render_target_arena);
	if (!r->render_target_arena.data)
		goto error;

	sf_graphics_vulkan_create_instance(r);
	if (!r->vk_instance)
		goto error;

	sf_graphics_vulkan_proc_functions(r);
	if (!r->vk_create_debug_utils_messenger_ext || !r->vk_destroy_debug_utils_messenger_ext)
		goto error;

	sf_graphics_vulkan_create_validation_messenger(r);
	if (!r->vk_validation_messenger)
		goto error;

	sf_graphics_vulkan_create_surface(r);
	if (!r->vk_surface)
		goto error;

	sf_graphics_vulkan_pick_physical_device(r);
	if (!r->vk_physical_device)
		goto error;

	sf_graphics_vulkan_create_device(r);
	if (!r->vk_device)
		goto error;

	sf_graphics_vulkan_set_device_queues(r);
	sf_graphics_vulkan_pick_surface_format(r);
	sf_graphics_vulkan_pick_present_mode(r);
	sf_graphics_vulkan_pick_depth_stencil_format(r);
	sf_graphics_vulkan_pick_sample_count(r);

	sf_graphics_create_swapchain_resources(r);
	if (!r->vk_swapchain || !r->vk_swapchain_image_count || !r->swapchain_render_target_count || !r->draw_complete_semaphore_count)
		goto error;

	sf_graphics_create_image_acquired_semaphores(r);
	if (!r->image_acquired_semaphore_count)
		goto error;

	sf_graphics_create_in_flight_fences(r);
	if (!r->in_flight_fence_count)
		goto error;

	return r;

error:
	sf_graphics_destroy_renderer(r);

	return NULL;
}

void sf_graphics_destroy_renderer(struct sf_graphics_renderer *r) {
	sf_u32 i;

	if (!r)
		return;

	sf_graphics_device_wait_idle(r);

	for (i = 0; i < SF_SIZE(r->in_flight_fences); ++i) {
		sf_graphics_destroy_fence(r, r->in_flight_fences[i]);
		r->in_flight_fences[i] = SF_NULL_HANDLE;
	}
	r->in_flight_fence_count = 0;

	for (i = 0; i < SF_SIZE(r->image_acquired_semaphores); ++i) {
		sf_graphics_destroy_semaphore(r, r->image_acquired_semaphores[i]);
		r->image_acquired_semaphores[i] = SF_NULL_HANDLE;
	}
	r->image_acquired_semaphore_count = 0;

	sf_graphics_destroy_swapchain_resources(r);

	if (r->vk_device) {
		vkDestroyDevice(r->vk_device, r->vk_allocation_callbacks);
		r->vk_device = VK_NULL_HANDLE;
	}

	if (r->vk_instance && r->vk_surface) {
		vkDestroySurfaceKHR(r->vk_instance, r->vk_surface, r->vk_allocation_callbacks);
		r->vk_surface = VK_NULL_HANDLE;
	}

	if (r->vk_instance && r->vk_validation_messenger) {
		r->vk_destroy_debug_utils_messenger_ext(r->vk_instance, r->vk_validation_messenger, r->vk_allocation_callbacks);
		r->vk_validation_messenger = VK_NULL_HANDLE;
	}

	if (r->vk_instance) {
		vkDestroyInstance(r->vk_instance, r->vk_allocation_callbacks);
		r->vk_instance = VK_NULL_HANDLE;
	}

	sf_graphics_default_init_renderer(r);
}
