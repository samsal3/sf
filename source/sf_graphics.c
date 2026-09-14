#include "sf_graphics.h"

#include "sf_image.h"

#include <stdio.h>

#define SF_VULKAN_CHECK(e) sf_graphics_vulkan_check((e), #e, __LINE__, __FILE__)
#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)

sf_private char const *sf_graphics_string_from_vulkan_result(VkResult vk_result) {
	char const *result = NULL;

	switch (vk_result) {
		case VK_SUCCESS:
			result = "VK_SUCCESS";
			break;
		case VK_NOT_READY:
			result = "VK_NOT_READY";
			break;
		case VK_TIMEOUT:
			result = "VK_TIMEOUT";
			break;
		case VK_EVENT_SET:
			result = "VK_EVENT_SET";
			break;
		case VK_EVENT_RESET:
			result = "VK_EVENT_RESET";
			break;
		case VK_INCOMPLETE:
			result = "VK_INCOMPLETE";
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			result = "VK_ERROR_OUT_OF_HOST_MEMORY";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			result = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			result = "VK_ERROR_INITIALIZATION_FAILED";
			break;
		case VK_ERROR_DEVICE_LOST:
			result = "VK_ERROR_DEVICE_LOST";
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			result = "VK_ERROR_MEMORY_MAP_FAILED";
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			result = "VK_ERROR_LAYER_NOT_PRESENT";
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			result = "VK_ERROR_EXTENSION_NOT_PRESENT";
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			result = "VK_ERROR_FEATURE_NOT_PRESENT";
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			result = "VK_ERROR_INCOMPATIBLE_DRIVER";
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			result = "VK_ERROR_TOO_MANY_OBJECTS";
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			result = "VK_ERROR_FORMAT_NOT_SUPPORTED";
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			result = "VK_ERROR_FRAGMENTED_POOL";
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			result = "VK_ERROR_OUT_OF_POOL_MEMORY";
			break;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			result = "VK_ERROR_INVALID_EXTERNAL_HANDLE";
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			result = "VK_ERROR_SURFACE_LOST_KHR";
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			result = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
			break;
		case VK_SUBOPTIMAL_KHR:
			result = "VK_SUBOPTIMAL_KHR";
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			result = "VK_ERROR_OUT_OF_DATE_KHR";
			break;

		case VK_ERROR_UNKNOWN:
		default:
			result = "VK_ERROR_UNKNOWN";
			break;
	}

	return result;
}

sf_private sf_bool sf_graphics_vulkan_check(VkResult result, char const *what, int line, char const *file) {
	char const *result_string = sf_graphics_string_from_vulkan_result(result);
	fprintf(stderr, "%s - %s - %s:%i\n", result_string, what, file, line);
	return result == VK_SUCCESS;
}

sf_private sf_bool sf_graphics_vulkan_is_extension_available(char const *required_extension, u32 available_extension_count, VkExtensionProperties *available_extensions) {
	u32 i = 0;
	struct sf_string required = {0};
	sf_string_from_non_literal(required_extension, VK_MAX_EXTENSION_NAME_SIZE, &required);

	for (i = 0; i < available_extension_count; ++i) {
		struct sf_string available = {0};
		sf_string_from_non_literal(available_extensions[i].extensionName, VK_MAX_EXTENSION_NAME_SIZE, &available);

		if (sf_string_compare(&required, &available, VK_MAX_EXTENSION_NAME_SIZE))
			return SF_TRUE;
	}
	return SF_FALSE;
}

sf_private sf_bool sf_graphics_vulkan_are_extensions_available(u32 required_extension_count, char const **required_extensions, u32 available_extension_count, VkExtensionProperties *available_extensions) {
	u32 i = 0;

	for (i = 0; i < required_extension_count; ++i) {
		if (!sf_graphics_vulkan_is_extension_available(required_extensions[i], available_extension_count, available_extensions))
			return SF_FALSE;
	}
	return SF_TRUE;
}

struct sf_graphics_vulkan_extension_properties_array {
	u32 size;
	VkExtensionProperties *data;
};

sf_private void sf_graphics_vulkan_load_available_instance_extensions(struct sf_arena *arena, struct sf_graphics_vulkan_extension_properties_array *array) {
	u32 count = 0;

	if (!SF_VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, NULL)))
		return;

	array->data = sf_arena_allocate(arena, sizeof(*array->data) * count);
	if (!array->data)
		return;

	if (!SF_VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, array->data)))
		return;

	array->size = count;
}

sf_private void sf_graphics_vulkan_load_available_device_extensions(struct sf_arena *arena, VkPhysicalDevice device, struct sf_graphics_vulkan_extension_properties_array *array) {
	u32 count = 0;

	if (!SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL)))
		return;

	array->data = sf_arena_allocate(arena, sizeof(*array->data) * count);
	if (!array->data)
		return;

	if (!SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &count, array->data)))
		return;

	array->size = count;
}

sf_private sf_bool sf_graphics_vulkan_are_instance_extensions_available(struct sf_arena *arena, u32 required_extension_count, char const **required_extensions) {
	struct sf_graphics_vulkan_extension_properties_array available_extensions = {0};

	sf_graphics_vulkan_load_available_instance_extensions(arena, &available_extensions);
	if (!available_extensions.size || !available_extensions.data)
		return SF_FALSE;

	return sf_graphics_vulkan_are_extensions_available(required_extension_count, required_extensions, available_extensions.size, available_extensions.data);
}

sf_private sf_bool sf_graphics_vulkan_are_device_extensions_available(struct sf_arena *arena, VkPhysicalDevice device, u32 required_extension_count, char const **required_extensions) {
	struct sf_graphics_vulkan_extension_properties_array available_extensions = {0};

	sf_graphics_vulkan_load_available_device_extensions(arena, device, &available_extensions);
	if (!available_extensions.size || !available_extensions.data)
		return SF_FALSE;

	return sf_graphics_vulkan_are_extensions_available(required_extension_count, required_extensions, available_extensions.size, available_extensions.data);
}

static VkBool32 VKAPI_CALL sf_graphics_vulkan_log(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) {
	(void)messageTypes;
	(void)userData;

	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		default:
			fprintf(stderr, "%s\n", callbackData->pMessage);
			break;
	}

	return VK_TRUE;
}

sf_private void sf_graphics_vulkan_instance_init(struct sf_arena *arena, struct sf_graphics_renderer *r, struct sf_graphics_renderer_info *info) {
	u32 vk_api_version = 0;
	VkApplicationInfo app_info = {0};
	VkInstanceCreateInfo instance_info = {0};
	struct sf_string null_terminated_app_name = {0};

	if (!arena || !r || !info || r->vk_instance)
		return;

	r->vk_instance = VK_NULL_HANDLE;

	vkEnumerateInstanceVersion(&vk_api_version);
	if (vk_api_version < VK_MAKE_API_VERSION(0, 1, 0, 0))
		return;

	sf_string_null_terminate(arena, &info->application_name, &null_terminated_app_name);
	if (!null_terminated_app_name.data || !null_terminated_app_name.size)
		return;

	if (!sf_graphics_vulkan_are_instance_extensions_available(arena, info->vk_instance_extension_count, info->vk_instance_extensions))
		return;

	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = null_terminated_app_name.data;
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "sf";
	app_info.engineVersion = 1;
	app_info.apiVersion = VK_API_VERSION_1_0;

	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pNext = NULL;
	instance_info.flags = 0;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = info->vk_instance_layer_count;
	instance_info.ppEnabledLayerNames = info->vk_instance_layers;
	instance_info.enabledExtensionCount = info->vk_instance_extension_count;
	instance_info.ppEnabledExtensionNames = info->vk_instance_extensions;

	if (!SF_VULKAN_CHECK(vkCreateInstance(&instance_info, r->vk_allocation_callbacks, &r->vk_instance)))
		r->vk_instance = VK_NULL_HANDLE;
}

sf_private void sf_graphics_vulkan_load_functions(struct sf_graphics_renderer *r) {
	if (!r || !r->vk_instance)
		return;

	r->vk_create_debug_utils_messenger_ext = SF_VULKAN_PROC(vkCreateDebugUtilsMessengerEXT, r->vk_instance);
	r->vk_destroy_debug_utils_messenger_ext = SF_VULKAN_PROC(vkDestroyDebugUtilsMessengerEXT, r->vk_instance);
}

sf_private void sf_graphics_vulkan_validation_messenger_init(struct sf_graphics_renderer *r) {
	VkDebugUtilsMessengerCreateInfoEXT info = {0};

	if (!r || !r->vk_instance || !r->vk_create_debug_utils_messenger_ext || !r->vk_destroy_debug_utils_messenger_ext || r->vk_validation_messenger)
		return;

	r->vk_validation_messenger = VK_NULL_HANDLE;

	if (!r->vk_create_debug_utils_messenger_ext || !r->vk_destroy_debug_utils_messenger_ext)
		return;

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
	info.pfnUserCallback = sf_graphics_vulkan_log;
	info.pUserData = NULL;

	if (!SF_VULKAN_CHECK(r->vk_create_debug_utils_messenger_ext(r->vk_instance, &info, r->vk_allocation_callbacks, &r->vk_validation_messenger)))
		r->vk_validation_messenger = VK_NULL_HANDLE;
}

sf_private sf_bool sf_graphics_are_queue_family_indices_valid(struct sf_graphics_renderer *r) {
	return r->vk_graphics_queue_family_index != (u32)-1 && r->vk_present_queue_family_index != (u32)-1;
}

struct sf_graphics_queue_family_properties_array {
	u32 size;
	VkQueueFamilyProperties *data;
};

sf_private void sf_graphics_load_queue_family_properties(struct sf_arena *arena, VkPhysicalDevice device, struct sf_graphics_queue_family_properties_array *array) {
	if (!arena || !device || !array)
		return;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &array->size, NULL);
	if (!array->size)
		return;

	array->data = sf_arena_allocate(arena, array->size * sizeof(*array->data));
	if (!array->data)
		return;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &array->size, array->data);
}

sf_private void sf_graphics_vulkan_find_suitable_queue_family_indices(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, u32 *graphics_queue_family_index, u32 *present_queue_family_index) {
	u32 i = 0;
	struct sf_graphics_queue_family_properties_array properties = {0};

	if (!arena || !device || !surface || !graphics_queue_family_index || !present_queue_family_index)
		return;

	*graphics_queue_family_index = (u32)-1;
	*present_queue_family_index = (u32)-1;

	sf_graphics_load_queue_family_properties(arena, device, &properties);

	if (!properties.size || !properties.data)
		return;

	for (i = 0; i < properties.size && (*graphics_queue_family_index == (u32)-1 || *present_queue_family_index == (u32)-1); ++i) {
		VkBool32 supports_surface = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_surface);

		if (supports_surface)
			*present_queue_family_index = i;

		if (properties.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			*graphics_queue_family_index = i;
	}
}

struct sf_graphics_vulkan_surface_format_array {
	u32 size;
	VkSurfaceFormatKHR *data;
};

sf_private void sf_graphics_vulkan_load_surface_formats(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, struct sf_graphics_vulkan_surface_format_array *array) {
	u32 count = 0;

	if (!arena || !device || !surface)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL)))
		return;

	array->data = sf_arena_allocate(arena, count * sizeof(*array->data));
	if (!array->data)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, array->data)))
		return;

	array->size = count;
}

struct sf_graphics_vulkan_present_mode_array {
	u32 size;
	VkPresentModeKHR *data;
};

sf_private void sf_graphics_vulkan_load_present_modes(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, struct sf_graphics_vulkan_present_mode_array *array) {
	u32 count = 0;

	if (!arena || !device || !surface || !array)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL)))
		return;

	array->data = sf_arena_allocate(arena, count * sizeof(*array->data));
	if (!array->data)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, array->data)))
		return;

	array->size = count;
}

sf_private void sf_graphics_vulkan_swapchain_find_present_mode(struct sf_arena *arena, struct sf_graphics_renderer *r) {
	u32 i = 0;
	struct sf_graphics_vulkan_present_mode_array present_modes = {0};
	VkPresentModeKHR requested_present_mode = VK_PRESENT_MODE_FIFO_KHR;

	if (!arena || !r || !r->vk_physical_device || !r->vk_surface)
		return; // NOTE(samuel): Guaranteed to be present;

	r->vk_swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

	requested_present_mode = r->requested_enable_vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

	sf_graphics_vulkan_load_present_modes(arena, r->vk_physical_device, r->vk_surface, &present_modes);
	if (!present_modes.data || !present_modes.size)
		return;

	for (i = 0; i < present_modes.size; ++i) {
		if (requested_present_mode == present_modes.data[i]) {
			r->vk_swapchain_present_mode = requested_present_mode;
			break;
		}
	}

	if (r->vk_swapchain_present_mode == VK_PRESENT_MODE_FIFO_KHR)
		r->vk_swapchain_enable_vsync = SF_TRUE;
	else
		r->vk_swapchain_enable_vsync = SF_FALSE;
}

sf_private VkFormat sf_graphics_vulkan_find_format(VkPhysicalDevice device, u32 available_format_count, VkFormat *available_formats, VkImageTiling tiling, VkFormatFeatureFlags2 features) {
	u32 i = 0;

	if (!device || !available_format_count || !available_formats)
		return VK_FORMAT_UNDEFINED;

	for (i = 0; i < available_format_count; ++i) {
		VkFormatProperties properties = {0};
		VkFormat format = available_formats[i];

		vkGetPhysicalDeviceFormatProperties(device, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			return format;

		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			return format;
	}

	return VK_FORMAT_UNDEFINED;
}

sf_private void sf_graphics_vulkan_find_supported_color_format(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR *format) {
	u32 i = 0;
	VkSurfaceFormatKHR requested_format = {0};
	VkSurfaceFormatKHR default_format = {0};
	struct sf_graphics_vulkan_surface_format_array candidates = {0};

	requested_format.format = VK_FORMAT_R8G8B8A8_UNORM;
	requested_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	default_format.format = VK_FORMAT_B8G8R8A8_UNORM;
	default_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	*format = default_format;

	if (!arena || !device || !surface || !format)
		return;

	sf_graphics_vulkan_load_surface_formats(arena, device, surface, &candidates);
	if (!candidates.size || !candidates.data)
		return;

	for (i = 0; i < candidates.size; ++i) {
		VkSurfaceFormatKHR *current = &candidates.data[i];

		if (current->format == requested_format.format && current->colorSpace == requested_format.colorSpace) {
			*format = *current;
			return;
		}
	}
}

struct sf_graphics_physical_device_array {
	u32 size;
	VkPhysicalDevice *data;
};

sf_private void sf_graphics_vulkan_load_physical_devices(struct sf_arena *arena, VkInstance instance, struct sf_graphics_physical_device_array *array) {
	u32 count = 0;

	if (!arena || !instance || !array)
		return;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &count, NULL)))
		return;

	array->data = sf_arena_allocate(arena, count * sizeof(*array->data));
	if (!array->data)
		return;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &count, array->data)))
		return;

	array->size = count;
}

sf_private void sf_graphics_vulkan_find_suitable_physical_device(struct sf_arena *arena, struct sf_graphics_renderer *r, struct sf_graphics_renderer_info *info) {
	u32 i = 0;
	VkPhysicalDevice last_candidate = VK_NULL_HANDLE;
	struct sf_graphics_physical_device_array physical_devices = {0};

	if (!arena || !r->vk_instance || !r->vk_surface || !info)
		return;

	r->vk_physical_device = VK_NULL_HANDLE;
	r->vk_graphics_queue_family_index = (u32)-1;
	r->vk_present_queue_family_index = (u32)-1;

	sf_graphics_vulkan_load_physical_devices(arena, r->vk_instance, &physical_devices);
	if (!physical_devices.data || !physical_devices.size)
		return;

	for (i = 0; i < physical_devices.size; ++i) {
		u32 graphics_queue_family_index = (u32)-1;
		u32 present_queue_family_index = (u32)-1;

		VkPhysicalDeviceProperties properties = {0};
		VkPhysicalDeviceFeatures features = {0};
		VkPhysicalDevice device = physical_devices.data[i];

		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		sf_graphics_vulkan_find_suitable_queue_family_indices(arena, device, r->vk_surface, &graphics_queue_family_index, &present_queue_family_index);
		if (graphics_queue_family_index == (u32)-1 || present_queue_family_index == (u32)-1)
			continue;

		if (!sf_graphics_vulkan_are_device_extensions_available(arena, device, info->vk_device_extension_count, info->vk_device_extensions))
			continue;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			r->vk_physical_device = device;
			goto finish_physical_device_setup;
		} else {
			last_candidate = device; // NOTE(samuel): Found a candidate, but keep looking for a discrete GPU.
		}
	}

	r->vk_physical_device = last_candidate;

finish_physical_device_setup:
	if (r->vk_physical_device)
		sf_graphics_vulkan_find_suitable_queue_family_indices(arena, r->vk_physical_device, r->vk_surface, &r->vk_graphics_queue_family_index, &r->vk_present_queue_family_index);
}

sf_private void sf_graphics_vulkan_device_init(struct sf_graphics_renderer *r, struct sf_graphics_renderer_info *info) {
	VkPhysicalDeviceFeatures features = {0};
	float priority = 1.0F;
	VkDeviceQueueCreateInfo queue_infos[2] = {0};
	VkDeviceCreateInfo device_info = {0};

	if (!r || !r->vk_physical_device || !info || r->vk_device)
		return;

	r->vk_device = VK_NULL_HANDLE;

	vkGetPhysicalDeviceFeatures(r->vk_physical_device, &features);

	queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_infos[0].pNext = NULL;
	queue_infos[0].flags = 0;
	queue_infos[0].queueFamilyIndex = r->vk_graphics_queue_family_index;
	queue_infos[0].queueCount = 1;
	queue_infos[0].pQueuePriorities = &priority;

	queue_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_infos[1].pNext = NULL;
	queue_infos[1].flags = 0;
	queue_infos[1].queueFamilyIndex = r->vk_present_queue_family_index;
	queue_infos[1].queueCount = 1;
	queue_infos[1].pQueuePriorities = &priority;

	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.flags = 0;
	if (r->vk_graphics_queue_family_index == r->vk_present_queue_family_index)
		device_info.queueCreateInfoCount = 1;
	else
		device_info.queueCreateInfoCount = SF_SIZE(queue_infos);
	device_info.pQueueCreateInfos = queue_infos;
	device_info.enabledLayerCount = 0;	// NOTE(samuel): deprecated
	device_info.ppEnabledLayerNames = NULL; // NOTE(samuel): deprecated
	device_info.enabledExtensionCount = info->vk_device_extension_count;
	device_info.ppEnabledExtensionNames = info->vk_device_extensions;
	device_info.pEnabledFeatures = &features; // NOTE(samuel): legacy, but we are still using version 1.0.0;

	if (!SF_VULKAN_CHECK(vkCreateDevice(r->vk_physical_device, &device_info, r->vk_allocation_callbacks, &r->vk_device)))
		r->vk_device = VK_NULL_HANDLE;
}

sf_private void sf_graphics_vulkan_load_device_queues(struct sf_graphics_renderer *r) {
	vkGetDeviceQueue(r->vk_device, r->vk_graphics_queue_family_index, 0, &r->vk_graphics_queue);
	vkGetDeviceQueue(r->vk_device, r->vk_present_queue_family_index, 0, &r->vk_present_queue);
}

sf_private struct sf_graphics_image *sf_graphics_get_image_from_pool(struct sf_graphics_renderer *r) {
	u32 i = 0;

	for (i = 1; i < SF_SIZE(r->image_pool); ++i) {
		struct sf_graphics_image *current = &r->image_pool[i];

		if (!current->base.is_occupied) {
			SF_MEMORY_SET(current, 0, sizeof(*current));
			current->base.is_occupied = SF_TRUE;
			return current;
		}
	}

	return NULL;
}


sf_private sf_handle sf_graphics_handle_from_image(struct sf_graphics_renderer *r, struct sf_graphics_image *image) {
	if (!image)
		return SF_NULL_HANDLE;

	return (sf_handle)(image - &r->image_pool[0]);
}

sf_private struct sf_graphics_image *sf_graphics_image_from_handle(struct sf_graphics_renderer *r, sf_handle handle) {
	if (handle >= SF_SIZE(r->image_pool))
		return NULL;

	return &r->image_pool[handle];
}

sf_private struct sf_graphics_buffer *sf_graphics_get_buffer_from_pool(struct sf_graphics_renderer *r) {
	u32 i = 0;

	for (i = 1; i < SF_SIZE(r->buffer_pool); ++i) {
		struct sf_graphics_buffer *current = &r->buffer_pool[i];

		if (!current->base.is_occupied) {
			SF_MEMORY_SET(current, 0, sizeof(*current));
			current->base.is_occupied = SF_TRUE;
			return current;
		}
	}

	return NULL;
}

sf_private sf_handle sf_graphics_handle_from_buffer(struct sf_graphics_renderer *r, struct sf_graphics_buffer *buffer) {
	if (!buffer)
		return SF_NULL_HANDLE;

	return (sf_handle)(buffer - &r->buffer_pool[0]);
}

sf_private struct sf_graphics_buffer *sf_graphics_buffer_from_handle(struct sf_graphics_renderer *r, sf_handle handle) {
	if (handle >= SF_SIZE(r->buffer_pool))
		return NULL;

	return &r->buffer_pool[handle];
}

sf_private struct sf_graphics_command_buffer *sf_graphics_get_command_buffer_from_pool(struct sf_graphics_renderer *r) {
	u32 i = 0;

	for (i = 1; i < SF_SIZE(r->command_buffer_pool); ++i) {
		struct sf_graphics_command_buffer *current = &r->command_buffer_pool[i];

		if (!current->base.is_occupied) {
			SF_MEMORY_SET(current, 0, sizeof(*current));
			current->base.is_occupied = SF_TRUE;
			return current;
		}
	}

	return NULL;
}

sf_private sf_handle sf_graphics_handle_from_command_buffer(struct sf_graphics_renderer *r, struct sf_graphics_command_buffer *command_buffer) {
	if (!command_buffer)
		return SF_NULL_HANDLE;

	return (sf_handle)(command_buffer - &r->command_buffer_pool[0]);
}

sf_private struct sf_graphics_command_buffer *sf_graphics_command_buffer_from_handle(struct sf_graphics_renderer *r, sf_handle handle) {
	if (handle >= SF_SIZE(r->command_buffer_pool))
		return NULL;

	return &r->command_buffer_pool[handle];
}

sf_private struct sf_graphics_render_target *sf_graphics_get_render_target_from_pool(struct sf_graphics_renderer *r) {
	u32 i = 0;

	for (i = 1; i < SF_SIZE(r->render_target_pool); ++i) {
		struct sf_graphics_render_target *current = &r->render_target_pool[i];

		if (!current->base.is_occupied) {
			SF_MEMORY_SET(current, 0, sizeof(*current));
			current->base.is_occupied = SF_TRUE;
			return current;
		}
	}

	return NULL;
}

sf_private sf_handle sf_graphics_handle_from_render_target(struct sf_graphics_renderer *r, struct sf_graphics_render_target *render_target) {
	if (!render_target)
		return SF_NULL_HANDLE;

	return (sf_handle)(render_target - &r->render_target_pool[0]);
}

sf_private struct sf_graphics_render_target *sf_graphics_render_target_from_handle(struct sf_graphics_renderer *r, sf_handle handle) {
	if (handle >= SF_SIZE(r->render_target_pool))
		return NULL;

	return &r->render_target_pool[handle];
}

sf_private u32 sf_graphics_vulkan_find_memory_type_index(VkPhysicalDevice device, VkMemoryPropertyFlags memory_properties, u32 filter) {
	u32 i = 0;
	VkPhysicalDeviceMemoryProperties available = {0};

	vkGetPhysicalDeviceMemoryProperties(device, &available);

	for (i = 0; i < available.memoryTypeCount; ++i)
		if ((filter & (1 << i)) && (available.memoryTypes[i].propertyFlags & memory_properties) == memory_properties)
			return i;

	return (u32)-1;
}

sf_private VkMemoryPropertyFlags sf_graphics_vulkan_memory_property_flags_from_memory_property_flags(sf_graphics_memory_property_flags flags) {
	VkMemoryPropertyFlags result = 0;

	if (flags & SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_LOCAL)
		result |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	if (flags & SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE) {
		result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		result |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}

	if (flags & SF_GRAPHICS_MEMORY_PROPERTY_LAZILY_ALLOCATED)
		result |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

	if (flags & SF_GRAPHICS_MEMORY_PROPERTY_PROTECTED)
		result |= VK_MEMORY_PROPERTY_PROTECTED_BIT;

	return result;
}

sf_private VkDeviceMemory sf_graphics_vulkan_allocate_memory(struct sf_graphics_renderer *r, sf_graphics_memory_property_flags memory_properties, u32 filter, u64 size) {
	u32 memory_type_index = (u32)-1;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkMemoryAllocateInfo info = {0};
	VkMemoryAllocateFlagsInfo flags_info = {0};

	if (!r || !r->vk_physical_device || !r->vk_device || !size)
		return VK_NULL_HANDLE;

	memory_type_index = sf_graphics_vulkan_find_memory_type_index(r->vk_physical_device, sf_graphics_vulkan_memory_property_flags_from_memory_property_flags(memory_properties), filter);
	if (memory_type_index == (u32)-1)
		return VK_NULL_HANDLE;

	flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flags_info.pNext = NULL;
	flags_info.flags = memory_properties & SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_ADDRESS ? VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT : 0;
	flags_info.deviceMask = 0;

	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.pNext = &flags_info;
	info.allocationSize = size;
	info.memoryTypeIndex = memory_type_index;
	if (SF_VULKAN_CHECK(vkAllocateMemory(r->vk_device, &info, r->vk_allocation_callbacks, &memory)))
		return memory;

	return VK_NULL_HANDLE;
}

sf_private VkDeviceMemory sf_graphics_vulkan_allocate_memory_for_image(struct sf_graphics_renderer *r, VkImage image, VkMemoryPropertyFlags memory_flags) {
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkMemoryRequirements requirements = {0};

	if (!r || !r->vk_device || !image)
		return VK_NULL_HANDLE;

	vkGetImageMemoryRequirements(r->vk_device, image, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(r, memory_flags, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindImageMemory(r->vk_device, image, memory, 0))) {
		vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}

sf_private VkDeviceMemory sf_graphics_vulkan_allocate_memory_for_buffer(struct sf_graphics_renderer *r, VkBuffer buffer, sf_graphics_memory_property_flags memory_flags) {
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkMemoryRequirements requirements = {0};

	if (!r || !r->vk_device || !buffer)
		return VK_NULL_HANDLE;

	vkGetBufferMemoryRequirements(r->vk_device, buffer, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(r, memory_flags, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindBufferMemory(r->vk_device, buffer, memory, 0))) {
		vkFreeMemory(r->vk_device, memory, r->vk_allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}

sf_private VkBufferUsageFlags sf_graphics_vulkan_buffer_usage_flags_from_buffer_usage_flags(sf_graphics_buffer_usage_flags flags) {
	VkBufferUsageFlags result = 0;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_TRANSFER_SOURCE)
		result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_TRANSFER_DESTINATION)
		result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER)
		result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_STORAGE_TEXEL_BUFFER)
		result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_UNIFORM_BUFFER)
		result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_STORAGE_BUFFER)
		result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_INDEX_BUFFER)
		result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_VERTEX_BUFFER)
		result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_INDIRECT_BUFFER)
		result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_SHADER_DEVICE_ADDRESS)
		result |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	if (flags & SF_GRAPHICS_BUFFER_USAGE_DESCRIPTOR_HEAP)
		result |= VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

	return result;
}

sf_private VkBuffer sf_graphics_vulkan_buffer_create(struct sf_graphics_renderer *r, u64 size, sf_graphics_buffer_usage_flags buffer_flags) {
	VkBufferCreateInfo info = {0};
	VkBuffer buffer = VK_NULL_HANDLE;

	if (!r || !r->vk_device || !size)
		return VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.size = size;
	info.usage = sf_graphics_vulkan_buffer_usage_flags_from_buffer_usage_flags(buffer_flags);
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = NULL;

	if (SF_VULKAN_CHECK(vkCreateBuffer(r->vk_device, &info, r->vk_allocation_callbacks, &buffer)))
		return buffer;

	return VK_NULL_HANDLE;
}

sf_private VkDeviceSize sf_graphics_vulkan_get_buffer_device_address(struct sf_graphics_renderer *r, VkBuffer buffer) {
	VkBufferDeviceAddressInfo info = {0};

	info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	info.pNext = NULL;
	info.buffer = buffer;

	// NOTE(samuel): https://docs.vulkan.org/refpages/latest/refpages/source/vkGetBufferDeviceAddress.html 0 is null
	return vkGetBufferDeviceAddress(r->vk_device, &info);
}

sf_private void sf_graphics_vulkan_buffer_deinit(struct sf_graphics_renderer *r, struct sf_graphics_buffer *buffer) {
	if (!r || !buffer)
		return;

	buffer->base.is_occupied = SF_FALSE;
	buffer->cpu_mapped_data = NULL;

	if (r->vk_device) {
		if (r->vk_global_descriptor_pool) {
			if (buffer->vk_descriptor_set) {
				vkFreeDescriptorSets(r->vk_device, r->vk_global_descriptor_pool, 1, &buffer->vk_descriptor_set);
				buffer->vk_descriptor_set = VK_NULL_HANDLE;
			}
		}

		if (buffer->vk_memory) {
			vkFreeMemory(r->vk_device, buffer->vk_memory, r->vk_allocation_callbacks);
			buffer->vk_memory = VK_NULL_HANDLE;
		}

		if (buffer->vk_buffer) {
			vkDestroyBuffer(r->vk_device, buffer->vk_buffer, r->vk_allocation_callbacks);
			buffer->vk_buffer = VK_NULL_HANDLE;
		}
	}
}


sf_private VkDescriptorSet sf_graphics_vulkan_buffer_descriptor_set_create_and_write(struct sf_graphics_renderer *r, enum sf_graphics_buffer_usage usage, VkBuffer vk_buffer) {
	VkWriteDescriptorSet write = {0};
	VkDescriptorBufferInfo buffer_info = {0};
	VkDescriptorSetAllocateInfo info = {0};
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;

	if (!(usage & SF_GRAPHICS_BUFFER_USAGE_UNIFORM_BUFFER))
		return VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.pNext = NULL;
	info.descriptorPool = r->vk_global_descriptor_pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = usage & SF_GRAPHICS_BUFFER_USAGE_UNIFORM_BUFFER ? &r->vk_dynamic_uniform_descriptor_set_layout : &r->vk_dynamic_uniform_descriptor_set_layout;

	if (!SF_VULKAN_CHECK(vkAllocateDescriptorSets(r->vk_device, &info, &descriptor_set)))
		return VK_NULL_HANDLE;

	buffer_info.buffer = vk_buffer;
	buffer_info.offset = 0;
	buffer_info.range = VK_WHOLE_SIZE;

	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = NULL;
	write.dstSet = descriptor_set;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = NULL;
	write.pBufferInfo = &buffer_info;
	write.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(r->vk_device, 1, &write, 0, NULL);
}


sf_private struct sf_graphics_buffer *sf_graphics_vulkan_buffer_init(struct sf_graphics_renderer *r, u64 size, sf_graphics_buffer_usage_flags buffer_flags, sf_graphics_memory_property_flags memory_flags) {
	struct sf_graphics_buffer *buffer = sf_graphics_get_buffer_from_pool(r);
	if (!buffer)
		return NULL;

	buffer->vk_buffer = VK_NULL_HANDLE;
	buffer->vk_memory = VK_NULL_HANDLE;
	buffer->vk_address = 0;
	buffer->cpu_mapped_data = NULL;

	buffer->vk_buffer = sf_graphics_vulkan_buffer_create(r, size, buffer_flags);
	if (!buffer->vk_buffer)
		goto error;

	buffer->vk_memory = sf_graphics_vulkan_allocate_memory_for_buffer(r, buffer->vk_buffer, memory_flags);
	if (!buffer->vk_memory)
		goto error;

	// TODO(samuel): Maybe remove this
	if ((buffer_flags & SF_GRAPHICS_BUFFER_USAGE_SHADER_DEVICE_ADDRESS) == SF_GRAPHICS_BUFFER_USAGE_SHADER_DEVICE_ADDRESS) {
		buffer->vk_address = sf_graphics_vulkan_get_buffer_device_address(r, buffer->vk_buffer);
		if (!buffer->vk_address)
			goto error;
	}

	if ((memory_flags & SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE) == SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE) {
		if (!SF_VULKAN_CHECK(vkMapMemory(r->vk_device, buffer->vk_memory, 0, VK_WHOLE_SIZE, 0, &buffer->cpu_mapped_data)))
			goto error;
	}


	if (buffer_flags & SF_GRAPHICS_BUFFER_USAGE_UNIFORM_BUFFER) {
		buffer->vk_descriptor_set = sf_graphics_vulkan_buffer_descriptor_set_create_and_write(r, buffer_flags, buffer->vk_buffer);
		if (!buffer->vk_descriptor_set)
			goto error; 
	} 

	return buffer;

error:
	sf_graphics_vulkan_buffer_deinit(r, buffer);
	return NULL;
}

sf_public sf_handle sf_graphics_buffer_init(struct sf_graphics_renderer *r, u64 size, sf_graphics_buffer_usage_flags buffer_flags, sf_graphics_memory_property_flags memory_flags) {
	struct sf_graphics_buffer *buffer = SF_NULL_HANDLE;

	buffer = sf_graphics_vulkan_buffer_init(r, size, buffer_flags, memory_flags);
	if (!buffer)
		return SF_NULL_HANDLE;

	return sf_graphics_handle_from_buffer(r, buffer);
}

sf_public void sf_graphics_buffer_deinit(struct sf_graphics_renderer *r, sf_handle handle) {
	struct sf_graphics_buffer *buffer = sf_graphics_buffer_from_handle(r, handle);
	sf_graphics_vulkan_buffer_deinit(r, buffer);
}

sf_public void *sf_graphics_buffer_get_cpu_mapped_data(struct sf_graphics_renderer *r, sf_handle handle) {
	struct sf_graphics_buffer *buffer = sf_graphics_buffer_from_handle(r, handle);

	if (!buffer)
		return NULL;

	return buffer->cpu_mapped_data;
}

sf_public sf_handle sf_graphics_buffer_init_for_staging(struct sf_graphics_renderer *r, u64 data_size_in_bytes, void const *data) {
	void *mapped_data = NULL;
	sf_graphics_buffer_usage_flags usage = SF_GRAPHICS_BUFFER_USAGE_TRANSFER_SOURCE | SF_GRAPHICS_BUFFER_USAGE_TRANSFER_DESTINATION;
	sf_graphics_memory_property_flags mem_properties = SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE;
	sf_handle handle = sf_graphics_buffer_init(r, data_size_in_bytes, usage, mem_properties);

	if (!handle)
		return SF_NULL_HANDLE;

	if (data) {
		mapped_data = sf_graphics_buffer_get_cpu_mapped_data(r, handle);
		if (!mapped_data)
			goto error;

		SF_MEMORY_COPY(mapped_data, data, data_size_in_bytes);
	}

	return handle;

error:
	sf_graphics_buffer_deinit(r, handle);
	return SF_NULL_HANDLE;
}

sf_private VkCommandBufferUsageFlags sf_graphics_vulkan_command_buffer_usage_flags_from_command_buffer_flags(sf_graphics_command_buffer_usage_flags flags) {
	VkCommandBufferUsageFlags result = 0;

	if (flags & SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT)
		result = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	return result;
}

sf_private VkCommandPoolCreateFlags sf_graphics_vulkan_command_pool_create_flags_from_command_buffer_flags(sf_graphics_command_buffer_usage_flags flags) {
	VkCommandPoolCreateFlags result = 0;

	if (flags & SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT)
		result = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	// NOTE(samuel): all command buffers can be reset
	result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	return result;
}

sf_private VkCommandPool sf_graphics_vulkan_command_pool_create(struct sf_graphics_renderer *r, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo info = {0};
	VkCommandPool pool = VK_NULL_HANDLE;

	if (!r || !r->vk_device)
		return VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = NULL;
	info.flags = flags;
	info.queueFamilyIndex = r->vk_graphics_queue_family_index;

	if (SF_VULKAN_CHECK(vkCreateCommandPool(r->vk_device, &info, r->vk_allocation_callbacks, &pool)))
		return pool;

	return VK_NULL_HANDLE;
}

sf_private VkCommandBuffer sf_graphics_vulkan_allocate_command_buffer(struct sf_graphics_renderer *r, VkCommandPool vk_command_pool, VkCommandBufferUsageFlags flags) {
	VkCommandBufferAllocateInfo info = {0};
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;

	SF_UNUSED(flags);

	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = NULL;
	info.commandPool = vk_command_pool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;

	if (SF_VULKAN_CHECK(vkAllocateCommandBuffers(r->vk_device, &info, &command_buffer)))
		return command_buffer;

	return VK_NULL_HANDLE;
}

sf_private void sf_graphics_vulkan_command_buffer_deinit(struct sf_graphics_renderer *r, struct sf_graphics_command_buffer *command_buffer) {
	if (!r || !command_buffer)
		return;

	command_buffer->base.is_occupied = SF_FALSE;

	if (r->vk_device) {
		if (command_buffer->vk_command_pool) {
			if (command_buffer->vk_command_buffer) {
				vkFreeCommandBuffers(r->vk_device, command_buffer->vk_command_pool, 1, &command_buffer->vk_command_buffer);
				command_buffer->vk_command_buffer = VK_NULL_HANDLE;
			}

			vkDestroyCommandPool(r->vk_device, command_buffer->vk_command_pool, r->vk_allocation_callbacks);
			command_buffer->vk_command_pool = VK_NULL_HANDLE;
		}
	}
}

sf_private struct sf_graphics_command_buffer *sf_graphics_vulkan_command_buffer_init(struct sf_graphics_renderer *r, sf_graphics_command_buffer_usage_flags flags) {
	struct sf_graphics_command_buffer *command_buffer = NULL;
	VkCommandBufferUsageFlags vk_command_buffer_usage_flags = sf_graphics_vulkan_command_buffer_usage_flags_from_command_buffer_flags(flags);
	VkCommandPoolCreateFlags vk_command_pool_create_flags = sf_graphics_vulkan_command_pool_create_flags_from_command_buffer_flags(flags);

	if (!r)
		return NULL;

	command_buffer = sf_graphics_get_command_buffer_from_pool(r);
	if (!command_buffer)
		return NULL;

	command_buffer->vk_command_pool = sf_graphics_vulkan_command_pool_create(r, vk_command_pool_create_flags);
	if (!command_buffer->vk_command_pool)
		goto error;

	command_buffer->vk_command_buffer = sf_graphics_vulkan_allocate_command_buffer(r, command_buffer->vk_command_pool, vk_command_buffer_usage_flags);
	if (!command_buffer->vk_command_buffer)
		goto error;

	command_buffer->usage = flags;

	return command_buffer;

error:
	sf_graphics_vulkan_command_buffer_deinit(r, command_buffer);
	return NULL;
}

sf_public sf_handle sf_graphics_command_buffer_init(struct sf_graphics_renderer *r, sf_graphics_command_buffer_usage_flags flags) {
	struct sf_graphics_command_buffer *command_buffer = NULL;

	command_buffer = sf_graphics_vulkan_command_buffer_init(r, flags);
	if (!command_buffer)
		return SF_NULL_HANDLE;

	return sf_graphics_handle_from_command_buffer(r, command_buffer);
}

sf_public void sf_graphics_command_buffer_deinit(struct sf_graphics_renderer *r, sf_handle handle) {
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, handle);
	sf_graphics_vulkan_command_buffer_deinit(r, command_buffer);
}

sf_private sf_bool sf_graphics_command_buffer_is_recording(struct sf_graphics_renderer *r, sf_handle handle) {
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, handle);

	if (!command_buffer)
		return SF_FALSE;

	return command_buffer->is_recording;
}

sf_private void sf_graphics_vulkan_command_buffer_reset(struct sf_graphics_renderer *r, sf_handle handle) {
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, handle);

	if (!command_buffer || !command_buffer->vk_command_pool)
		return;

	if (SF_VULKAN_CHECK(vkResetCommandPool(r->vk_device, command_buffer->vk_command_pool, 0))) {
		command_buffer->is_recording = SF_FALSE;
		command_buffer->is_executable = SF_FALSE;
	}
}

sf_public void sf_graphics_command_buffer_reset(struct sf_graphics_renderer *r, sf_handle handle) {
	sf_graphics_vulkan_command_buffer_reset(r, handle);
}

sf_private void sf_graphics_vulkan_command_buffer_begin(struct sf_graphics_renderer *r, sf_handle handle) {
	VkCommandBufferBeginInfo info = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, handle);

	if (!command_buffer || !command_buffer->vk_command_buffer || command_buffer->is_executable || command_buffer->is_recording)
		return;

	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	if (command_buffer->usage & SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT)
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	else
		info.flags = 0;
	info.pInheritanceInfo = NULL;

	command_buffer->is_recording = SF_VULKAN_CHECK(vkBeginCommandBuffer(command_buffer->vk_command_buffer, &info));
}

sf_private void sf_graphics_vulkan_command_buffer_end(struct sf_graphics_renderer *r, sf_handle handle) {
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, handle);

	if (!command_buffer || !command_buffer->vk_command_buffer)
		return;

	// https://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html#commandbuffers-lifecycle
	if (command_buffer->is_recording) {
		if (SF_VULKAN_CHECK(vkEndCommandBuffer(command_buffer->vk_command_buffer))) {
			command_buffer->is_recording = SF_FALSE;
			command_buffer->is_executable = SF_TRUE;
		} else {
			sf_graphics_vulkan_command_buffer_reset(r, handle);
		}
	}
}

sf_private void sf_graphics_vulkan_command_buffer_submit_and_block(struct sf_graphics_renderer *r, sf_handle handle) {
	VkCommandBufferSubmitInfo command_buffer_info = {0};
	VkSubmitInfo2 info = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, handle);

	if (!command_buffer || !r || !r->vk_device || !r->vk_graphics_queue || !command_buffer->is_executable)
		return;

	command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	command_buffer_info.pNext = NULL;
	command_buffer_info.commandBuffer = command_buffer->vk_command_buffer;
	command_buffer_info.deviceMask = 0;

	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	info.pNext = NULL;
	info.flags = 0;
	info.waitSemaphoreInfoCount = 0;
	info.pWaitSemaphoreInfos = NULL;
	info.commandBufferInfoCount = 1;
	info.pCommandBufferInfos = &command_buffer_info;
	info.signalSemaphoreInfoCount = 0;
	info.pSignalSemaphoreInfos = NULL;

	SF_VULKAN_CHECK(vkQueueSubmit2(r->vk_graphics_queue, 1, &info, VK_NULL_HANDLE));
	SF_VULKAN_CHECK(vkDeviceWaitIdle(r->vk_device));
}

sf_public void sf_graphics_command_buffer_begin(struct sf_graphics_renderer *r, sf_handle handle) {
	sf_graphics_vulkan_command_buffer_begin(r, handle);
}

sf_public void sf_graphics_command_buffer_end(struct sf_graphics_renderer *r, sf_handle handle) {
	sf_graphics_vulkan_command_buffer_end(r, handle);
}

sf_public void sf_graphics_command_buffer_submit_and_block(struct sf_graphics_renderer *r, sf_handle handle) {
	sf_graphics_vulkan_command_buffer_submit_and_block(r, handle);
}

sf_public sf_handle sf_graphics_command_buffer_init_for_single_use_and_begin(struct sf_graphics_renderer *r) {
	sf_handle command_buffer = sf_graphics_command_buffer_init(r, SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT);

	if (!command_buffer)
		return SF_NULL_HANDLE;

	sf_graphics_command_buffer_begin(r, command_buffer);
	if (!sf_graphics_command_buffer_is_recording(r, command_buffer))
		goto error;

	return command_buffer;

error:
	sf_graphics_command_buffer_deinit(r, command_buffer);
	return SF_NULL_HANDLE;
}

sf_public void sf_graphics_command_buffer_end_submit_and_deinit(struct sf_graphics_renderer *r, sf_handle command_buffer_handle) {
	sf_graphics_command_buffer_end(r, command_buffer_handle);
	sf_graphics_command_buffer_submit_and_block(r, command_buffer_handle);
	sf_graphics_command_buffer_deinit(r, command_buffer_handle);
}

sf_private void sf_graphics_vulkan_command_transition_to_initial_layout(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, u32 mips, VkImageAspectFlags vk_aspect_flags, sf_handle image_handle) {
	VkImageMemoryBarrier2 barrier = {0};
	VkDependencyInfo dependency = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);
	struct sf_graphics_image *image = sf_graphics_image_from_handle(r, image_handle);

	if (!r || !command_buffer || !command_buffer->vk_command_buffer || !image || !image->vk_image)
		return;

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = NULL;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
	barrier.srcAccessMask = VK_ACCESS_2_NONE;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image->vk_image;
	barrier.subresourceRange.aspectMask = vk_aspect_flags;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mips;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency.pNext = NULL;
	dependency.dependencyFlags = 0;
	dependency.memoryBarrierCount = 0;
	dependency.pMemoryBarriers = NULL;
	dependency.bufferMemoryBarrierCount = 0;
	dependency.pBufferMemoryBarriers = NULL;
	dependency.imageMemoryBarrierCount = 1;
	dependency.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(command_buffer->vk_command_buffer, &dependency);
}

sf_private void sf_graphics_vulkan_command_transition_swapchain_layout_for_rendering(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, u32 mips, VkImageAspectFlags vk_aspect_flags, VkImage vk_image) {
	VkImageMemoryBarrier2 barrier = {0};
	VkDependencyInfo dependency = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);

	if (!r || !command_buffer || !command_buffer->vk_command_buffer)
		return;

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = NULL;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.srcAccessMask = VK_ACCESS_2_NONE;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vk_image;
	barrier.subresourceRange.aspectMask = vk_aspect_flags;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mips;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency.pNext = NULL;
	dependency.dependencyFlags = 0;
	dependency.memoryBarrierCount = 0;
	dependency.pMemoryBarriers = NULL;
	dependency.bufferMemoryBarrierCount = 0;
	dependency.pBufferMemoryBarriers = NULL;
	dependency.imageMemoryBarrierCount = 1;
	dependency.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(command_buffer->vk_command_buffer, &dependency);
}

sf_private VkAccessFlags2 sf_graphics_vulkan_access_mask_from_pipeline_stage(VkPipelineStageFlags2 stage, sf_bool is_source) {
	VkAccessFlags2 result = 0;

	if ((stage & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT) || (stage & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) || (stage & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT)) {
		if (is_source)
			result |= (VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT);
		else
			result |= VK_ACCESS_2_SHADER_READ_BIT;
	}

	if (stage & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT)
		result |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;

	if (stage & VK_PIPELINE_STAGE_2_TRANSFER_BIT) {
		if (is_source)
			result |= VK_ACCESS_2_TRANSFER_READ_BIT;
		else
			result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
	}

	return result;
}

struct sf_graphics_buffer_memory_barrier {
	VkBuffer vk_buffer;
	VkPipelineStageFlags2 vk_source_stage_flags;
	VkPipelineStageFlags2 vk_destination_stage_flags;
	VkAccessFlags2 vk_source_access_flags;
	VkAccessFlags2 vk_destination_access_flags;
	VkDeviceSize offset;
	VkDeviceSize size;
	u32 vk_source_queue_family_index;
	u32 vk_destination_queue_family_index;
};

sf_private void sf_graphics_vulkan_command_buffer_memory_barrier(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, struct sf_graphics_buffer_memory_barrier *barrier) {
	VkBufferMemoryBarrier2 vk_barrier = {0};
	VkDependencyInfo vk_dependency = {0};
	VkAccessFlags2 vk_source_access_flags = 0;
	VkAccessFlags2 vk_destination_access_flags = 0;
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);

	if (!r || !command_buffer || !command_buffer->vk_command_buffer)
		return;

	if (!barrier->vk_source_access_flags)
		vk_source_access_flags = sf_graphics_vulkan_access_mask_from_pipeline_stage(barrier->vk_source_stage_flags, SF_TRUE);

	if (!barrier->vk_destination_access_flags)
		vk_destination_access_flags = sf_graphics_vulkan_access_mask_from_pipeline_stage(barrier->vk_destination_stage_flags, SF_FALSE);

	vk_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
	vk_barrier.pNext = NULL;
	vk_barrier.srcStageMask = barrier->vk_source_stage_flags;
	vk_barrier.srcAccessMask = vk_source_access_flags;
	vk_barrier.dstStageMask = barrier->vk_destination_stage_flags;
	vk_barrier.dstAccessMask = vk_destination_access_flags;
	vk_barrier.srcQueueFamilyIndex = barrier->vk_source_queue_family_index;
	vk_barrier.dstQueueFamilyIndex = barrier->vk_destination_queue_family_index;
	vk_barrier.buffer = barrier->vk_buffer;
	vk_barrier.offset = barrier->offset;
	vk_barrier.size = barrier->size;

	vk_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	vk_dependency.pNext = NULL;
	vk_dependency.dependencyFlags = 0;
	vk_dependency.memoryBarrierCount = 0;
	vk_dependency.pMemoryBarriers = NULL;
	vk_dependency.bufferMemoryBarrierCount = 1;
	vk_dependency.pBufferMemoryBarriers = &vk_barrier;
	vk_dependency.imageMemoryBarrierCount = 0;
	vk_dependency.pImageMemoryBarriers = NULL;

	vkCmdPipelineBarrier2(command_buffer->vk_command_buffer, &vk_dependency);
}

sf_private VkShaderModule sf_graphics_vulkan_shader_create(struct sf_graphics_renderer *r, u32 code_size, void const *code) {
	VkShaderModuleCreateInfo info = {0};
	VkShaderModule shader = VK_NULL_HANDLE;

	if (!r || !r->vk_device)
		return VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.codeSize = code_size;
	info.pCode = (u32 const *)code;

	if (SF_VULKAN_CHECK(vkCreateShaderModule(r->vk_device, &info, r->vk_allocation_callbacks, &shader)))
		return shader;

	return VK_NULL_HANDLE;
}

sf_private void sf_graphics_vulkan_command_transition_swapchain_layout_for_presenting(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, u32 mips, VkImageAspectFlags vk_aspect_flags, VkImage vk_image) {
	VkImageMemoryBarrier2 barrier = {0};
	VkDependencyInfo dependency = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);

	if (!r || !command_buffer || !command_buffer->vk_command_buffer)
		return;

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = NULL;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.dstAccessMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vk_image;
	barrier.subresourceRange.aspectMask = vk_aspect_flags;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mips;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency.pNext = NULL;
	dependency.dependencyFlags = 0;
	dependency.memoryBarrierCount = 0;
	dependency.pMemoryBarriers = NULL;
	dependency.bufferMemoryBarrierCount = 1;
	dependency.pBufferMemoryBarriers = NULL;
	dependency.imageMemoryBarrierCount = 1;
	dependency.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(command_buffer->vk_command_buffer, &dependency);
}

sf_private void sf_graphics_vulkan_command_bind_shaders(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, VkShaderEXT vertex, VkShaderEXT fragment) {
	VkShaderEXT shaders[5] = {0};
	VkShaderStageFlagBits stages[5] = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);

	if (!r || !command_buffer || !command_buffer->vk_command_buffer)
		return;

	shaders[0] = vertex;
	shaders[1] = VK_NULL_HANDLE;
	shaders[2] = VK_NULL_HANDLE;
	shaders[3] = VK_NULL_HANDLE;
	shaders[4] = fragment;

	stages[0] = VK_SHADER_STAGE_VERTEX_BIT;
	stages[1] = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	stages[2] = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	stages[3] = VK_SHADER_STAGE_GEOMETRY_BIT;
	stages[4] = VK_SHADER_STAGE_FRAGMENT_BIT;

	vkCmdBindShadersEXT(command_buffer->vk_command_buffer, SF_SIZE(stages), stages, shaders);
}

sf_private void sf_graphics_vulkan_command_copy_buffer_to_buffer(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, u64 source_offset, u64 destination_offset, u64 size, sf_handle source_buffer_handle, sf_handle destination_buffer_handle) {
	VkBufferCopy copy = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);
	struct sf_graphics_buffer *source_buffer = sf_graphics_buffer_from_handle(r, source_buffer_handle);
	struct sf_graphics_buffer *destination_buffer = sf_graphics_buffer_from_handle(r, destination_buffer_handle);

	if (!r || !command_buffer || !source_buffer || !destination_buffer)
		return;

	copy.srcOffset = source_offset;
	copy.dstOffset = destination_offset;
	copy.size = size;

	vkCmdCopyBuffer(command_buffer->vk_command_buffer, source_buffer->vk_buffer, destination_buffer->vk_buffer, 1, &copy);
}

sf_private void sf_graphics_vulkan_command_copy_buffer_to_image(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, u64 buffer_offset, u32 width, u32 height, u32 depth, u32 mips, sf_handle source_buffer_handle, sf_handle destination_image_handle) {
	VkBufferImageCopy copy = {0};
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);
	struct sf_graphics_buffer *source_buffer = sf_graphics_buffer_from_handle(r, source_buffer_handle);
	struct sf_graphics_image *destination_image = sf_graphics_image_from_handle(r, destination_image_handle);

	if (!r || !command_buffer || !source_buffer || !destination_image)
		return;

	copy.bufferOffset = buffer_offset;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.imageSubresource.mipLevel = mips;
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageOffset.x = 0;
	copy.imageOffset.y = 0;
	copy.imageOffset.z = 0;
	copy.imageExtent.width = width;
	copy.imageExtent.height = height;
	copy.imageExtent.depth = depth;

	vkCmdCopyBufferToImage(command_buffer->vk_command_buffer, source_buffer->vk_buffer, destination_image->vk_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
}

sf_private void sf_graphics_vulkan_device_wait_idle(struct sf_graphics_renderer *r) {
	if (!r || !r->vk_device)
		return;

	SF_VULKAN_CHECK(vkDeviceWaitIdle(r->vk_device));
}

sf_private u64 sf_graphics_stride_from_format(enum sf_graphics_format format) {
	switch (format) {
		// 1 channel
		case SF_GRAPHICS_FORMAT_R8_UNORM:
			return 1;
		case SF_GRAPHICS_FORMAT_R16_UNORM:
			return 2;
		case SF_GRAPHICS_FORMAT_R16_UINT:
			return 2;
		case SF_GRAPHICS_FORMAT_R16_SFLOAT:
			return 2;
		case SF_GRAPHICS_FORMAT_R32_UINT:
			return 4;
		case SF_GRAPHICS_FORMAT_R32_SFLOAT:
			return 4;
		// 2 CHANNEL
		case SF_GRAPHICS_FORMAT_R8G8_UNORM:
			return 2;
		case SF_GRAPHICS_FORMAT_R16G16_UNORM:
			return 4;
		case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:
			return 4;
		case SF_GRAPHICS_FORMAT_R32G32_UINT:
			return 8;
		case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:
			return 8;
		// 3 CHANNEL
		case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:
			return 3;
		case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:
			return 6;
		case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:
			return 6;
		case SF_GRAPHICS_FORMAT_R32G32B32_UINT:
			return 12;
		case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:
			return 12;
		// 4 CHANNEL
		case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:
			return 4;
		case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:
			return 4;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:
			return 8;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT:
			return 8;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:
			return 16;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT:
			return 16;
		// DEPTH/STENCIL
		case SF_GRAPHICS_FORMAT_D16_UNORM:
			return 0;
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32:
			return 0;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT:
			return 0;
		case SF_GRAPHICS_FORMAT_S8_UINT:
			return 0;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:
			return 0;
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:
			return 0;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:
			return 0;
		default:
			return 0;
	}
}

sf_private sf_handle sf_graphics_buffer_init_and_upload_data(struct sf_graphics_renderer *r, sf_graphics_buffer_usage_flags usage, sf_graphics_memory_property_flags memory_properties, u64 data_size_in_bytes, void const *data) {
	sf_handle buffer = SF_NULL_HANDLE;
	sf_handle staging_buffer = SF_NULL_HANDLE;
	sf_handle command_buffer = SF_NULL_HANDLE;

	staging_buffer = sf_graphics_buffer_init_for_staging(r, data_size_in_bytes, data);
	if (!staging_buffer)
		goto error;

	buffer = sf_graphics_buffer_init(r, data_size_in_bytes, usage | SF_GRAPHICS_BUFFER_USAGE_TRANSFER_DESTINATION, memory_properties);
	if (!buffer)
		goto error;

	command_buffer = sf_graphics_command_buffer_init_for_single_use_and_begin(r);
	if (!command_buffer)
		goto error;

	sf_graphics_vulkan_command_copy_buffer_to_buffer(r, command_buffer, 0, 0, data_size_in_bytes, staging_buffer, buffer);

	goto cleanup;

error:
	sf_graphics_vulkan_device_wait_idle(r);

	sf_graphics_buffer_deinit(r, buffer);
	buffer = SF_NULL_HANDLE;

cleanup:
	sf_graphics_command_buffer_end_submit_and_deinit(r, command_buffer);
	sf_graphics_buffer_deinit(r, staging_buffer);

	return buffer;
}

sf_private VkImageViewType sf_graphics_vulkan_image_view_type_from_image_type(enum sf_graphics_image_type type) {
	VkImageViewType result = VK_IMAGE_VIEW_TYPE_1D;

	switch (type) {
		case SF_GRAPHICS_IMAGE_TYPE_1D:
			result = VK_IMAGE_VIEW_TYPE_1D;
			break;
		case SF_GRAPHICS_IMAGE_TYPE_2D:
			result = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case SF_GRAPHICS_IMAGE_TYPE_3D:
			result = VK_IMAGE_VIEW_TYPE_3D;
			break;
		case SF_GRAPHICS_IMAGE_TYPE_CUBE:
			result = VK_IMAGE_VIEW_TYPE_CUBE;
			break;
		default:
			result = VK_IMAGE_VIEW_TYPE_2D;
			break;
	}

	return result;
}

sf_private VkImageType sf_graphics_vulkan_image_type_from_image_type(enum sf_graphics_image_type type) {
	VkImageType result = VK_IMAGE_TYPE_1D;

	switch (type) {
		case SF_GRAPHICS_IMAGE_TYPE_1D:
			result = VK_IMAGE_TYPE_1D;
			break;
		case SF_GRAPHICS_IMAGE_TYPE_2D:
			result = VK_IMAGE_TYPE_2D;
			break;
		case SF_GRAPHICS_IMAGE_TYPE_3D:
			result = VK_IMAGE_TYPE_3D;
			break;
		case SF_GRAPHICS_IMAGE_TYPE_CUBE:
			result = VK_IMAGE_TYPE_2D;
			break;
		default:
			result = VK_IMAGE_TYPE_2D;
			break;
	}

	return result;
}

sf_private VkFormat sf_graphics_vulkan_format_from_format(enum sf_graphics_format format) {
	VkFormat result = VK_FORMAT_UNDEFINED;

	switch (format) {
		// 1 channel
		case SF_GRAPHICS_FORMAT_R8_UNORM:
			result = VK_FORMAT_R8_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16_UNORM:
			result = VK_FORMAT_R16_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16_UINT:
			result = VK_FORMAT_R16_UINT;
			break;
		case SF_GRAPHICS_FORMAT_R16_SFLOAT:
			result = VK_FORMAT_R16_SFLOAT;
			break;
		case SF_GRAPHICS_FORMAT_R32_UINT:
			result = VK_FORMAT_R32_UINT;
			break;
		case SF_GRAPHICS_FORMAT_R32_SFLOAT:
			result = VK_FORMAT_R32_SFLOAT;
			break;
		// 2 channel
		case SF_GRAPHICS_FORMAT_R8G8_UNORM:
			result = VK_FORMAT_R8G8_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16G16_UNORM:
			result = VK_FORMAT_R16G16_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:
			result = VK_FORMAT_R16G16_SFLOAT;
			break;
		case SF_GRAPHICS_FORMAT_R32G32_UINT:
			result = VK_FORMAT_R32G32_UINT;
			break;
		case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:
			result = VK_FORMAT_R32G32_SFLOAT;
			break;
		// 3 channel
		case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:
			result = VK_FORMAT_R8G8B8_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:
			result = VK_FORMAT_R16G16B16_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:
			result = VK_FORMAT_R16G16B16_SFLOAT;
			break;
		case SF_GRAPHICS_FORMAT_R32G32B32_UINT:
			result = VK_FORMAT_R32G32B32_UINT;
			break;
		case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:
			result = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		// 4 channel
		case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:
			result = VK_FORMAT_B8G8R8A8_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_B8G8R8A8_SRGB:
			result = VK_FORMAT_B8G8R8A8_SRGB;
			break;
		case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:
			result = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:
			result = VK_FORMAT_R16G16B16A16_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT:
			result = VK_FORMAT_R16G16B16A16_SFLOAT;
			break;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:
			result = VK_FORMAT_R32G32B32A32_UINT;
			break;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT:
			result = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		// Depth/stencil
		case SF_GRAPHICS_FORMAT_D16_UNORM:
			result = VK_FORMAT_D16_UNORM;
			break;
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32:
			result = VK_FORMAT_X8_D24_UNORM_PACK32;
			break;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT:
			result = VK_FORMAT_D32_SFLOAT;
			break;
		case SF_GRAPHICS_FORMAT_S8_UINT:
			result = VK_FORMAT_S8_UINT;
			break;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:
			result = VK_FORMAT_D16_UNORM_S8_UINT;
			break;
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:
			result = VK_FORMAT_D24_UNORM_S8_UINT;
			break;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:
			result = VK_FORMAT_D32_SFLOAT_S8_UINT;
			break;
		default:
			result = VK_FORMAT_UNDEFINED;
			break;
	}

	return result;
}

sf_private enum sf_graphics_format sf_graphics_format_from_vulkan_format(VkFormat format) {
	enum sf_graphics_format result = SF_GRAPHICS_FORMAT_UNDEFINED;

	switch (format) {
		// 1 channel
		case VK_FORMAT_R8_UNORM:
			result = SF_GRAPHICS_FORMAT_R8_UNORM;
			break;
		case VK_FORMAT_R16_UNORM:
			result = SF_GRAPHICS_FORMAT_R16_UNORM;
			break;
		case VK_FORMAT_R16_UINT:
			result = SF_GRAPHICS_FORMAT_R16_UINT;
			break;
		case VK_FORMAT_R16_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R16_SFLOAT;
			break;
		case VK_FORMAT_R32_UINT:
			result = SF_GRAPHICS_FORMAT_R32_UINT;
			break;
		case VK_FORMAT_R32_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R32_SFLOAT;
			break;
		// 2 channel
		case VK_FORMAT_R8G8_UNORM:
			result = SF_GRAPHICS_FORMAT_R8G8_UNORM;
			break;
		case VK_FORMAT_R16G16_UNORM:
			result = SF_GRAPHICS_FORMAT_R16G16_UNORM;
			break;
		case VK_FORMAT_R16G16_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R16G16_SFLOAT;
			break;
		case VK_FORMAT_R32G32_UINT:
			result = SF_GRAPHICS_FORMAT_R32G32_UINT;
			break;
		case VK_FORMAT_R32G32_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R32G32_SFLOAT;
			break;
		// 3 channel
		case VK_FORMAT_R8G8B8_UNORM:
			result = SF_GRAPHICS_FORMAT_R8G8B8_UNORM;
			break;
		case VK_FORMAT_R16G16B16_UNORM:
			result = SF_GRAPHICS_FORMAT_R16G16B16_UNORM;
			break;
		case VK_FORMAT_R16G16B16_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT;
			break;
		case VK_FORMAT_R32G32B32_UINT:
			result = SF_GRAPHICS_FORMAT_R32G32B32_UINT;
			break;
		case VK_FORMAT_R32G32B32_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT;
			break;
		// 4 channel
		case VK_FORMAT_B8G8R8A8_UNORM:
			result = SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM;
			break;
		case VK_FORMAT_B8G8R8A8_SRGB:
			result = SF_GRAPHICS_FORMAT_B8G8R8A8_SRGB;
			break;
		case VK_FORMAT_R8G8B8A8_UNORM:
			result = SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM;
			break;
		case VK_FORMAT_R16G16B16A16_UNORM:
			result = SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM;
			break;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT;
			break;
		case VK_FORMAT_R32G32B32A32_UINT:
			result = SF_GRAPHICS_FORMAT_R32G32B32A32_UINT;
			break;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			result = SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT;
			break;
		// Depth/stencil
		case VK_FORMAT_D16_UNORM:
			result = SF_GRAPHICS_FORMAT_D16_UNORM;
			break;
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			result = SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32;
			break;
		case VK_FORMAT_D32_SFLOAT:
			result = SF_GRAPHICS_FORMAT_D32_SFLOAT;
			break;
		case VK_FORMAT_S8_UINT:
			result = SF_GRAPHICS_FORMAT_S8_UINT;
			break;
		case VK_FORMAT_D16_UNORM_S8_UINT:
			result = SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT;
			break;
		case VK_FORMAT_D24_UNORM_S8_UINT:
			result = SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT;
			break;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			result = SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT;
			break;
		default:
			result = SF_GRAPHICS_FORMAT_UNDEFINED;
			break;
	}

	return result;
}

sf_private VkImageAspectFlags sf_graphics_vulkan_image_aspect_flags_from_format(enum sf_graphics_format format) {
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
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT:
			result = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		// Depth/stencil
		case SF_GRAPHICS_FORMAT_D16_UNORM:
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32:
		case SF_GRAPHICS_FORMAT_D32_SFLOAT:
			result = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		case SF_GRAPHICS_FORMAT_S8_UINT:
			result = VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:
			result = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		default:
			result = 0;
			break;
	}

	return result;
}

sf_private VkSampleCountFlags sf_graphics_vulkan_sample_count_from_sample_count(enum sf_graphics_sample_count samples) {
	VkSampleCountFlagBits result = VK_SAMPLE_COUNT_1_BIT;
	switch (samples) {
		case SF_GRAPHICS_SAMPLE_COUNT_1:
			result = VK_SAMPLE_COUNT_1_BIT;
			break;
		case SF_GRAPHICS_SAMPLE_COUNT_2:
			result = VK_SAMPLE_COUNT_2_BIT;
			break;
		case SF_GRAPHICS_SAMPLE_COUNT_4:
			result = VK_SAMPLE_COUNT_4_BIT;
			break;
		case SF_GRAPHICS_SAMPLE_COUNT_8:
			result = VK_SAMPLE_COUNT_8_BIT;
			break;
		case SF_GRAPHICS_SAMPLE_COUNT_16:
			result = VK_SAMPLE_COUNT_16_BIT;
			break;
		default:
			result = VK_SAMPLE_COUNT_1_BIT;
			break;
	}
	return result;
}

sf_private enum sf_graphics_sample_count sf_graphics_sample_count_from_vulkan_sample_count(VkSampleCountFlags samples) {
	enum sf_graphics_sample_count result = SF_GRAPHICS_SAMPLE_COUNT_1;
	switch (samples) {
		case VK_SAMPLE_COUNT_1_BIT:
			result = SF_GRAPHICS_SAMPLE_COUNT_1;
			break;
		case VK_SAMPLE_COUNT_2_BIT:
			result = SF_GRAPHICS_SAMPLE_COUNT_2;
			break;
		case VK_SAMPLE_COUNT_4_BIT:
			result = SF_GRAPHICS_SAMPLE_COUNT_4;
			break;
		case VK_SAMPLE_COUNT_8_BIT:
			result = SF_GRAPHICS_SAMPLE_COUNT_8;
			break;
		case VK_SAMPLE_COUNT_16_BIT:
			result = SF_GRAPHICS_SAMPLE_COUNT_16;
			break;
		default:
			result = SF_GRAPHICS_SAMPLE_COUNT_1;
			break;
	}
	return result;
}

sf_private VkImageUsageFlags sf_graphics_vulkan_image_usage_from_image_usage(sf_graphics_image_usage_flags usage) {
	VkImageUsageFlags result = 0;

	if (usage & SF_GRAPHICS_IMAGE_USAGE_TRANSFER_SOURCE) {
		result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (usage & SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION) {
		result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (usage & SF_GRAPHICS_IMAGE_USAGE_SAMPLED) {
		result |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (usage & SF_GRAPHICS_IMAGE_USAGE_STORAGE) {
		result |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (usage & SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT) {
		result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (usage & SF_GRAPHICS_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT) {
		result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (usage & SF_GRAPHICS_IMAGE_USAGE_RESOLVE_SOURCE) {
		result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (usage & SF_GRAPHICS_IMAGE_USAGE_RESOLVE_DESTINATION) {
		result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	return result;
}

sf_public VkImage sf_graphics_vulkan_image_create(struct sf_graphics_renderer *r, enum sf_graphics_image_type image_type, sf_graphics_image_usage_flags image_usage, enum sf_graphics_format format, enum sf_graphics_sample_count samples, u32 width, u32 height, u32 mips) {
	VkImageCreateInfo info = {0};
	VkImage image = VK_NULL_HANDLE;

	if (!r || !r->vk_device)
		return VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.imageType = sf_graphics_vulkan_image_type_from_image_type(image_type);
	info.format = sf_graphics_vulkan_format_from_format(format);
	info.extent.width = width;
	info.extent.height = height;
	info.extent.depth = 1;
	info.mipLevels = mips;
	info.arrayLayers = 1;
	info.samples = sf_graphics_vulkan_sample_count_from_sample_count(samples);
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = sf_graphics_vulkan_image_usage_from_image_usage(image_usage);
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = NULL;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (SF_VULKAN_CHECK(vkCreateImage(r->vk_device, &info, r->vk_allocation_callbacks, &image)))
		return image;

	return VK_NULL_HANDLE;
}

sf_private VkImageView sf_graphics_vulkan_image_view_create(struct sf_graphics_renderer *r, VkImage vk_image, enum sf_graphics_image_type image_type, enum sf_graphics_format format, u32 mips) {
	VkImageViewCreateInfo info = {0};
	VkImageView image_view = VK_NULL_HANDLE;

	if (!r || !r->vk_device || !vk_image)
		return VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.image = vk_image;
	info.viewType = sf_graphics_vulkan_image_view_type_from_image_type(image_type);
	info.format = sf_graphics_vulkan_format_from_format(format);
	info.components.r = VK_COMPONENT_SWIZZLE_R;
	info.components.g = VK_COMPONENT_SWIZZLE_G;
	info.components.b = VK_COMPONENT_SWIZZLE_B;
	info.components.a = VK_COMPONENT_SWIZZLE_A;
	info.subresourceRange.aspectMask = sf_graphics_vulkan_image_aspect_flags_from_format(format);
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = mips;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;

	if (SF_VULKAN_CHECK(vkCreateImageView(r->vk_device, &info, r->vk_allocation_callbacks, &image_view)))
		return image_view;

	return VK_NULL_HANDLE;
}

sf_private VkDescriptorSet sf_graphics_vulkan_image_descriptor_set_create_and_write(struct sf_graphics_renderer *r, VkImageView vk_image_view) {
	VkWriteDescriptorSet write = {0};
	VkDescriptorImageInfo image_info = {0};
	VkDescriptorSetAllocateInfo info = {0};
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;

	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.pNext = NULL;
	info.descriptorPool = r->vk_global_descriptor_pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &r->vk_image_descriptor_set_layout;

	if (!SF_VULKAN_CHECK(vkAllocateDescriptorSets(r->vk_device, &info, &descriptor_set)))
		return VK_NULL_HANDLE;

	image_info.sampler = r->vk_linear_sampler; // TODO(samuel): For now default to this, later build a sampler cache
	image_info.imageView = vk_image_view;
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = NULL;
	write.dstSet = descriptor_set;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &image_info;
	write.pBufferInfo = NULL;
	write.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(r->vk_device,1 , &write, 0, NULL);
}

sf_private void sf_graphics_vulkan_image_deinit(struct sf_graphics_renderer *r, struct sf_graphics_image *image) {
	if (!r || !image)
		return;

	image->base.is_occupied = SF_FALSE;

	if (r->vk_device) {
		if (r->vk_global_descriptor_pool) {
			if (image->vk_descriptor_set) {
				vkFreeDescriptorSets(r->vk_device, r->vk_global_descriptor_pool, 1, &image->vk_descriptor_set);
				image->vk_descriptor_set = VK_NULL_HANDLE;
			}
		}

		if (image->vk_image_view) {
			vkDestroyImageView(r->vk_device, image->vk_image_view, r->vk_allocation_callbacks);
			image->vk_image_view = VK_NULL_HANDLE;
		}

		if (image->vk_owns_image) {
			if (image->vk_image) {
				vkDestroyImage(r->vk_device, image->vk_image, r->vk_allocation_callbacks);
				image->vk_image = VK_NULL_HANDLE;
			}
		}
	}
}

sf_private void sf_graphics_vulkan_transition_image_layout(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, sf_handle image_handle, VkImageLayout old_layout, VkImageLayout new_layout) {
	VkImageMemoryBarrier barrier = {0};
	VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_NONE;
	VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_NONE;
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);
	struct sf_graphics_image *image = sf_graphics_image_from_handle(r, image_handle);

	if (!command_buffer)
		return;

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = VK_ACCESS_NONE;
	barrier.dstAccessMask = VK_ACCESS_NONE;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image->vk_image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(command_buffer->vk_command_buffer, source_stage, destination_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
}


sf_private sf_bool sf_graphics_vulkan_check_mip_map_support(struct sf_graphics_renderer *r, enum sf_graphics_format format) {
	VkFormatProperties properties = {0};
	
	if (!r || !r->vk_physical_device)
		return SF_FALSE;

	vkGetPhysicalDeviceFormatProperties(r->vk_physical_device, sf_graphics_vulkan_format_from_format(format), &properties);
	return !(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
}

sf_private void sf_graphics_vulkan_image_generate_mip_maps(struct sf_graphics_renderer *r, sf_handle command_buffer_handle, sf_handle image_handle) {
	VkImageMemoryBarrier barrier = {0};
	i32 current_width = 0;
	i32 current_height = 0;
	u32 i = 0;
	struct sf_graphics_command_buffer *command_buffer = sf_graphics_command_buffer_from_handle(r, command_buffer_handle);
	struct sf_graphics_image *image = sf_graphics_image_from_handle(r, image_handle);

	if (!command_buffer || !image || !sf_graphics_vulkan_check_mip_map_support(r, image->format) || !image->vk_image)
		return;

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = VK_ACCESS_NONE;
	barrier.dstAccessMask = VK_ACCESS_NONE;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image->vk_image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	current_width = image->width;
	current_height = image->height;

	for (i = 1; i < image->mips; i++) {
		VkImageBlit blit = {0};

		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer->vk_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
	
		blit.srcOffsets[0].x = 0;
		blit.srcOffsets[0].y = 0;
		blit.srcOffsets[0].z = 0;
		blit.srcOffsets[1].x = current_width;
		blit.srcOffsets[1].y = current_height;
		blit.srcOffsets[1].z = 1;

		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		blit.dstOffsets[0].x = 0;
		blit.dstOffsets[0].y = 0;
		blit.dstOffsets[0].z = 0;

		blit.dstOffsets[1].x = current_width > 1 ? current_width/ 2 : 1;
		blit.dstOffsets[1].y = current_height > 1 ? current_height/ 2 : 1;
		blit.dstOffsets[1].z = 1;

		vkCmdBlitImage(command_buffer->vk_command_buffer, image->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer->vk_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		if (current_width > 1)
			current_width /= 2;

		if (current_height > 1)
			current_height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = image->mips - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command_buffer->vk_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
}

sf_private struct sf_graphics_image *sf_graphics_vulkan_image_init_with_image(struct sf_graphics_renderer *r, enum sf_graphics_image_type image_type, sf_graphics_image_usage_flags image_usage, enum sf_graphics_format format, enum sf_graphics_sample_count samples, u32 width, u32 height, u32 mips, VkImage vk_not_owned_image) {
	struct sf_graphics_image *image = sf_graphics_get_image_from_pool(r);

	if (!image)
		return SF_NULL_HANDLE;

	image->usage = image_usage;
	image->format = format;
	image->mips = sf_graphics_vulkan_check_mip_map_support(r, format) ? mips : 1;
	image->width = width;
	image->height = height;
	image->samples = samples;

	if (vk_not_owned_image) {
		image->vk_owns_image = SF_FALSE;
		image->vk_image = vk_not_owned_image;
	} else {
		image->vk_owns_image = SF_TRUE;
		image->vk_image = sf_graphics_vulkan_image_create(r, image_type, image_usage, format, samples, width, height, mips);
		if (!image->vk_image)
			goto error;
	}

	image->vk_image_view = sf_graphics_vulkan_image_view_create(r, image->vk_image, image_type, format, mips);
	if (!image->vk_image_view)
		goto error;

	if (image_usage & SF_GRAPHICS_IMAGE_USAGE_SAMPLED) {
		image->vk_descriptor_set = sf_graphics_vulkan_image_descriptor_set_create_and_write(r, image->vk_image_view);
		if (!image->vk_descriptor_set)
			goto error;
	}

	return image;

error:
	sf_graphics_vulkan_image_deinit(r, image);
	return NULL;
}

sf_public sf_handle sf_graphics_image_init(struct sf_graphics_renderer *r, enum sf_graphics_image_type image_type, sf_graphics_image_usage_flags image_usage, enum sf_graphics_format format, enum sf_graphics_sample_count samples, u32 width, u32 height, u32 mips) {
	struct sf_graphics_image *image = sf_graphics_vulkan_image_init_with_image(r, image_type, image_usage, format, samples, width, height, mips, VK_NULL_HANDLE);

	if (!image)
		return SF_NULL_HANDLE;

	return sf_graphics_handle_from_image(r, image);
}

sf_public sf_handle sf_graphics_image_init_and_upload_data(struct sf_graphics_renderer *r, enum sf_graphics_image_type image_type, sf_graphics_image_usage_flags image_usage, enum sf_graphics_format format, enum sf_graphics_sample_count samples, u32 width, u32 height, u32 mips, void const *data) {
	sf_handle image = SF_NULL_HANDLE;
	sf_handle staging_buffer = SF_NULL_HANDLE;
	sf_handle command_buffer = SF_NULL_HANDLE;
	u64 data_size_in_bytes = width * height * sf_graphics_stride_from_format(format);

	staging_buffer = sf_graphics_buffer_init_for_staging(r, data_size_in_bytes, data);
	if (!staging_buffer)
		goto error;

	image = sf_graphics_image_init(r, image_type, image_usage | SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION, format, samples, width, height, mips);
	if (!image)
		goto error;

	command_buffer = sf_graphics_command_buffer_init_for_single_use_and_begin(r);
	if (!command_buffer)
		goto error;

	sf_graphics_vulkan_transition_image_layout(r, command_buffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	sf_graphics_vulkan_command_copy_buffer_to_image(r, command_buffer, 0, width, height, 1, mips, staging_buffer, image);
	sf_graphics_vulkan_transition_image_layout(r, command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);	
	sf_graphics_vulkan_image_generate_mip_maps(r, command_buffer, image);	

	goto cleanup;

error:
	sf_graphics_vulkan_device_wait_idle(r);

	sf_graphics_image_deinit(r, image);
	image = SF_NULL_HANDLE;

cleanup:
	sf_graphics_command_buffer_end_submit_and_deinit(r, command_buffer);
	sf_graphics_buffer_deinit(r, staging_buffer);

	return image;
}

sf_public sf_handle sf_graphics_image_init_from_file(struct sf_graphics_renderer *r, struct sf_string *path) {
	u32 width = 0;
	u32 height = 0;
	enum sf_graphics_format format = SF_GRAPHICS_FORMAT_UNDEFINED;
	void *data = sf_image_load_from_file(&r->arena, path, &width, &height, &format);

	if (!data)
		return SF_NULL_HANDLE;

	return sf_graphics_image_init_and_upload_data(r, SF_GRAPHICS_IMAGE_TYPE_2D, SF_GRAPHICS_IMAGE_USAGE_SAMPLED, format, SF_GRAPHICS_SAMPLE_COUNT_1, width, height, 1, data);
}

sf_public void sf_graphics_image_deinit(struct sf_graphics_renderer *r, sf_handle image_handle) {
	struct sf_graphics_image *image = sf_graphics_image_from_handle(r, image_handle);

	if (!image)
		return;

	sf_graphics_vulkan_image_deinit(r, image);
}


sf_private void sf_graphics_vulkan_swapchain_init(struct sf_graphics_renderer *r) {
	VkSwapchainCreateInfoKHR info = {0};
	u32 queue_family_indices[2] = {r->vk_graphics_queue_family_index, r->vk_present_queue_family_index};

	// NOTE(samuel): No vk_swapchain reuse
	if (!r || !r->vk_device || !r->vk_surface || !r->vk_physical_device || r->vk_swapchain)
		return;

	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.surface = r->vk_surface;
	info.minImageCount = r->vk_swapchain_requested_image_count;
	info.imageFormat = r->vk_swapchain_color_format;
	info.imageColorSpace = r->vk_swapchain_color_space;
	info.imageExtent.width = r->vk_swapchain_width;
	info.imageExtent.height = r->vk_swapchain_height;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (r->vk_graphics_queue_family_index == r->vk_present_queue_family_index) {
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 1;
		info.pQueueFamilyIndices = queue_family_indices;
	} else {
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = SF_SIZE(queue_family_indices);
		info.pQueueFamilyIndices = queue_family_indices;
	}
	info.preTransform = r->vk_surface_capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = r->vk_swapchain_present_mode;
	info.clipped = VK_TRUE;
	info.oldSwapchain = VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkCreateSwapchainKHR(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_swapchain)))
		r->vk_swapchain = VK_NULL_HANDLE;
}

sf_private void sf_graphics_vulkan_swapchain_load_images(struct sf_graphics_renderer *r) {
	u32 image_count = 0;

	if (!r || !r->vk_swapchain)
		return;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &image_count, NULL)))
		return;

	if (image_count > SF_SIZE(r->vk_swapchain_images))
		return;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(r->vk_device, r->vk_swapchain, &image_count, r->vk_swapchain_images)))
		return;

	r->vk_swapchain_image_count = image_count;
}

sf_private void sf_graphics_vulkan_swapchain_find_color_format(struct sf_arena *arena, struct sf_graphics_renderer *r) {
	u32 i = 0;
	VkSurfaceFormatKHR requested_format = {0};
	struct sf_graphics_vulkan_surface_format_array candidates = {0};

	requested_format.format = VK_FORMAT_R8G8B8A8_UNORM;
	requested_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	if (!arena || !r || !r->vk_physical_device || !r->vk_surface)
		return;

	r->vk_swapchain_color_format = VK_FORMAT_UNDEFINED;
	r->vk_swapchain_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	sf_graphics_vulkan_load_surface_formats(arena, r->vk_physical_device, r->vk_surface, &candidates);
	if (!candidates.size || !candidates.data)
		return;

	for (i = 0; i < candidates.size; ++i) {
		VkSurfaceFormatKHR *current = &candidates.data[i];

		if (current->format == requested_format.format && current->colorSpace == requested_format.colorSpace) {
			r->vk_swapchain_color_format = current->format;
			r->vk_swapchain_color_space = current->colorSpace;
			return;
		}
	}
}

sf_private void sf_graphics_vulkan_swapchain_find_depth_stencil_format(struct sf_graphics_renderer *r) {
	VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

	if (!r)
		return;

	r->vk_swapchain_depth_stencil_format = VK_FORMAT_UNDEFINED;

	if (!r->vk_physical_device)
		return;

	r->vk_swapchain_depth_stencil_format = sf_graphics_vulkan_find_format(r->vk_physical_device, SF_SIZE(candidates), candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

sf_private void sf_graphics_vulkan_swapchain_resources_deinit(struct sf_graphics_renderer *r) {
	u32 i = 0;

	if (!r || !r->vk_device)
		return;

	for (i = 0; i < SF_SIZE(r->vk_swapchain_in_flight_fences); ++i) {
		VkFence fence = r->vk_swapchain_in_flight_fences[i];

		if (!fence)
			continue;

		vkDestroyFence(r->vk_device, fence, r->vk_allocation_callbacks);
		r->vk_swapchain_in_flight_fences[i] = VK_NULL_HANDLE;
	}
	r->vk_swapchain_in_flight_fence_count = 0;

	for (i = 0; i < SF_SIZE(r->vk_swapchain_image_acquired_semaphores); ++i) {
		VkSemaphore semaphore = r->vk_swapchain_image_acquired_semaphores[i];

		if (!semaphore)
			continue;

		vkDestroySemaphore(r->vk_device, semaphore, r->vk_allocation_callbacks);
		r->vk_swapchain_image_acquired_semaphores[i] = VK_NULL_HANDLE;
	}
	r->vk_swapchain_image_acquired_semaphore_count = 0;

	for (i = 0; i < SF_SIZE(r->vk_swapchain_draw_complete_semaphores); ++i) {
		VkSemaphore semaphore = r->vk_swapchain_draw_complete_semaphores[i];

		if (!semaphore)
			continue;

		vkDestroySemaphore(r->vk_device, semaphore, r->vk_allocation_callbacks);
		r->vk_swapchain_draw_complete_semaphores[i] = VK_NULL_HANDLE;
	}
	r->vk_swapchain_draw_complete_semaphore_count = 0;

	for (i = 0; i < SF_SIZE(r->swapchain_attachments); ++i) {
		sf_handle attachment = r->swapchain_attachments[i];
		sf_graphics_image_deinit(r, attachment);
		r->swapchain_attachments[i] = SF_NULL_HANDLE;
	}
	r->swapchain_attachment_count = 0;

	SF_ARRAY_INIT(r->vk_swapchain_images, VK_NULL_HANDLE);
	r->vk_swapchain_image_count = 0;

	if (r->vk_swapchain) {
		vkDestroySwapchainKHR(r->vk_device, r->vk_swapchain, r->vk_allocation_callbacks);
		r->vk_swapchain = VK_NULL_HANDLE;
	}
}

sf_private void sf_graphics_vulkan_swapchain_find_surface_capabilities(struct sf_graphics_renderer *r) {
	u32 min_image_count = 0;
	u32 max_image_count = 0;
	u32 req_image_count = 3;

	if (!r || !r->vk_physical_device || !r->vk_surface)
		return;

	r->vk_swapchain_requested_frames = 3; // TODO(samuel): Set from info or somewhere else

	r->vk_swapchain_width = 0;
	r->vk_swapchain_height = 0;

	r->vk_swapchain_min_image_count = 0;
	r->vk_swapchain_max_image_count = 0;
	r->vk_swapchain_requested_image_count = 0;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->vk_physical_device, r->vk_surface, &r->vk_surface_capabilities)))
		return;

	r->vk_swapchain_width = r->vk_surface_capabilities.currentExtent.width;
	r->vk_swapchain_height = r->vk_surface_capabilities.currentExtent.height;

	min_image_count = r->vk_surface_capabilities.minImageCount;
	max_image_count = r->vk_surface_capabilities.maxImageCount;

	r->vk_swapchain_min_image_count = min_image_count;
	r->vk_swapchain_max_image_count = max_image_count ? SF_MIN(max_image_count, SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT) : SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT;
	r->vk_swapchain_requested_image_count = SF_CLAMP(req_image_count, r->vk_swapchain_min_image_count, r->vk_swapchain_max_image_count);
}

sf_private void sf_graphics_vulkan_swapchain_attachments_init(struct sf_graphics_renderer *r) {
	u32 i = 0;

	if (!r || !r->vk_device)
		return;

	r->swapchain_attachment_count = 0;

	for (i = 0; i < r->vk_swapchain_image_count; ++i) {
		// FIXME(samuel): hate this
		struct sf_graphics_image *image = sf_graphics_vulkan_image_init_with_image(r, SF_GRAPHICS_IMAGE_TYPE_2D, SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT, r->vk_swapchain_color_format, r->vk_swapchain_samples, r->vk_swapchain_width, r->vk_swapchain_height, 1, r->vk_swapchain_images[i]);
		r->swapchain_attachments[i] = sf_graphics_handle_from_image(r, image);
		if (!r->swapchain_attachments[i])
			return;
	}

	r->swapchain_attachment_count = r->vk_swapchain_image_count;
}

sf_private void sf_graphics_vulkan_swapchain_draw_complete_semaphores_init(struct sf_graphics_renderer *r) {
	u32 i = 0;
	VkSemaphoreCreateInfo info = {0};

	for (i = 0; i < r->vk_swapchain_image_count; ++i) {
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;

		if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_swapchain_draw_complete_semaphores[i]))) {
			r->vk_swapchain_draw_complete_semaphores[i] = VK_NULL_HANDLE;
			return;
		}
	}

	r->vk_swapchain_draw_complete_semaphore_count = r->vk_swapchain_image_count;
}

sf_private void sf_graphics_vulkan_swapchain_image_acquired_semaphores_init(struct sf_graphics_renderer *r) {
	u32 i = 0;
	VkSemaphoreCreateInfo info = {0};

	for (i = 0; i < r->vk_swapchain_requested_frames; ++i) {
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;

		if (!SF_VULKAN_CHECK(vkCreateSemaphore(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_swapchain_image_acquired_semaphores[i]))) {
			r->vk_swapchain_image_acquired_semaphores[i] = VK_NULL_HANDLE;
			return;
		}
	}

	r->vk_swapchain_image_acquired_semaphore_count = r->vk_swapchain_requested_frames;
}

sf_private void sf_graphics_vulkan_swapchain_in_flight_fences_init(struct sf_graphics_renderer *r) {
	u32 i = 0;
	VkFenceCreateInfo info = {0};

	for (i = 0; i < r->vk_swapchain_requested_frames; ++i) {
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;

		if (!SF_VULKAN_CHECK(vkCreateFence(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_swapchain_in_flight_fences[i]))) {
			r->vk_swapchain_image_acquired_semaphores[i] = VK_NULL_HANDLE;
			return;
		}
	}

	r->vk_swapchain_in_flight_fence_count = r->vk_swapchain_requested_frames;
}

sf_private void sf_graphics_vulkan_swapchain_resources_init(struct sf_arena *arena, struct sf_graphics_renderer *r) {
	if (!arena || !r || !r->vk_physical_device || !r->vk_surface || !r->vk_device)
		return;

	r->vk_swapchain = VK_NULL_HANDLE;

	sf_graphics_vulkan_swapchain_find_color_format(arena, r);
	sf_graphics_vulkan_swapchain_find_depth_stencil_format(r);
	sf_graphics_vulkan_swapchain_find_present_mode(arena, r);
	sf_graphics_vulkan_swapchain_find_surface_capabilities(r);

	sf_graphics_vulkan_swapchain_init(r);
	if (!r->vk_swapchain)
		goto error;

	sf_graphics_vulkan_swapchain_load_images(r);
	if (!r->vk_swapchain_image_count)
		goto error;

	sf_graphics_vulkan_swapchain_attachments_init(r);
	if (!r->swapchain_attachment_count)
		goto error;

	sf_graphics_vulkan_swapchain_draw_complete_semaphores_init(r);
	if (!r->vk_swapchain_draw_complete_semaphore_count)
		goto error;

	sf_graphics_vulkan_swapchain_image_acquired_semaphores_init(r);
	if (!r->vk_swapchain_image_acquired_semaphore_count)
		goto error;

	sf_graphics_vulkan_swapchain_in_flight_fences_init(r);
	if (!r->vk_swapchain_in_flight_fence_count)
		goto error;

	return;

error:
	sf_graphics_vulkan_swapchain_resources_deinit(r);
}

sf_private void sf_graphics_vulkan_render_target_deinit(struct sf_graphics_renderer *r, struct sf_graphics_render_target *render_target) {
	u32 i = 0;

	if (!r || !render_target)
		return;

	render_target->base.is_occupied = SF_FALSE;

	if (r->vk_device) {
		if (render_target->vk_framebuffer) {
			vkDestroyFramebuffer(r->vk_device, render_target->vk_framebuffer, r->vk_allocation_callbacks);
			render_target->vk_framebuffer = VK_NULL_HANDLE;
		}

		if (render_target->vk_render_pass) {
			vkDestroyRenderPass(r->vk_device, render_target->vk_render_pass, r->vk_allocation_callbacks);
			render_target->vk_render_pass = VK_NULL_HANDLE;
		}
	}

	if (render_target->owns_color_attachments) {
		for (i = 0; i < SF_SIZE(render_target->color_attachments); ++i) {
			sf_handle attachment = render_target->color_attachments[i];

			if (!attachment)
				continue;

			sf_graphics_image_deinit(r, attachment);
			render_target->color_attachments[i] = SF_NULL_HANDLE;
		}
	} else {
		SF_ARRAY_INIT(render_target->color_attachments, SF_NULL_HANDLE);
	}
	
	render_target->owns_color_attachments = SF_FALSE;
	render_target->color_attachment_count = 0;

	if (render_target->depth_stencil_attachment) {
		sf_graphics_image_deinit(r, render_target->depth_stencil_attachment);
		render_target->depth_stencil_attachment = SF_NULL_HANDLE;
	}
}

sf_private void sf_graphics_vulkan_linear_sampler_init(struct sf_graphics_renderer *r) {
	VkSamplerCreateInfo info = {0};

	if (!r || !r->vk_device)
		return;

	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.magFilter = VK_FILTER_LINEAR;
	info.minFilter = VK_FILTER_LINEAR;
	info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.mipLodBias = 0.0F;
	info.anisotropyEnable = VK_FALSE;
	info.maxAnisotropy = 1.0F;
	info.compareEnable = VK_FALSE;
	info.compareOp = VK_COMPARE_OP_ALWAYS;
	info.minLod = 0.0F;
	info.maxLod = VK_LOD_CLAMP_NONE;
	info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	info.unnormalizedCoordinates = VK_FALSE;

	if (SF_VULKAN_CHECK(vkCreateSampler(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_linear_sampler)))
		r->vk_linear_sampler = VK_NULL_HANDLE;
}

sf_private u64 sf_u64_align_up(u64 value, u64 alignment) {
	if (!alignment)
		return value;

	return ((value + alignment - 1) / alignment) * alignment;
}

sf_private u32 sf_graphics_value_or_else(u32 value, u32 default_value) {
	return value ? value : default_value;
}

#define SF_GRAPHICS_MAX_RENDER_TARGET_TOTAL_ATTACHMENT_COUNT (SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT * 2 + 1)

struct sf_graphics_render_target_attachment_array {
	u32 size;
	VkImageView data[SF_GRAPHICS_MAX_RENDER_TARGET_TOTAL_ATTACHMENT_COUNT];
};

sf_private void sf_graphics_vulkan_render_target_load_attachment_array(struct sf_graphics_renderer *r, struct sf_graphics_render_target *render_target, struct sf_graphics_render_target_attachment_array *array) {
	SF_STATIC_ASSERT(SF_SIZE(render_target->color_attachments) == SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT, render_target_color_attachment_size_doesnt_match);
	SF_STATIC_ASSERT(SF_SIZE(render_target->resolve_attachments) == SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT, render_target_resolve_attachment_size_doesnt_match);

	u32 i = 0;

	array->size = 0;
	SF_ARRAY_INIT(array->data, VK_NULL_HANDLE);

	if (render_target->samples != SF_GRAPHICS_SAMPLE_COUNT_1)
		if (render_target->color_attachment_count != render_target->resolve_attachment_count)
			return;

	for (i = 0; i < render_target->color_attachment_count; ++i) {
		if (render_target->samples != SF_GRAPHICS_SAMPLE_COUNT_1) {
			struct sf_graphics_image *resolve_image = sf_graphics_image_from_handle(r, render_target->resolve_attachments[i]);
			array->data[array->size++] = resolve_image->vk_image_view;
		}
	
		struct sf_graphics_image *color_image = sf_graphics_image_from_handle(r, render_target->color_attachments[i]);
		array->data[array->size++] = color_image->vk_image_view;
	}

	if (render_target->depth_stencil_attachment) {
		struct sf_graphics_image *image = sf_graphics_image_from_handle(r, render_target->depth_stencil_attachment);
		array->data[array->size++] = image->vk_image_view;
	}
}

sf_private void sf_graphics_vulkan_render_target_render_pass_init(struct sf_graphics_renderer *r, struct sf_graphics_render_target *render_target) {
	u32 i = 0;
	u32 global_attachment_count = 0;

	VkAttachmentDescription attachments[SF_GRAPHICS_MAX_RENDER_TARGET_TOTAL_ATTACHMENT_COUNT] = {0};
	VkAttachmentReference color_references[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT] = {0};
	VkAttachmentReference resolve_references[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT] = {0};
	VkAttachmentReference depth_stencil_reference = {0};
	VkSubpassDescription subpass = {0};
	VkSubpassDependency subpass_dependency = {0};
	VkRenderPassCreateInfo render_pass_info = {0};

	render_target->vk_render_pass = VK_NULL_HANDLE;

	for (i = 0; i < render_target->color_attachment_count; ++i) {
		if (render_target->samples != SF_GRAPHICS_SAMPLE_COUNT_1) {
			attachments[global_attachment_count].flags = 0;
			attachments[global_attachment_count].format = sf_graphics_vulkan_format_from_format(render_target->color_format);
			attachments[global_attachment_count].samples = sf_graphics_vulkan_sample_count_from_sample_count(render_target->samples);
			attachments[global_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[global_attachment_count].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[global_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[global_attachment_count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[global_attachment_count].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[global_attachment_count].finalLayout = render_target->will_be_presented ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			resolve_references[i].attachment = global_attachment_count;
			resolve_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // FIXME(samuel): is this layout right?

			++global_attachment_count;
		}
	

		attachments[global_attachment_count].flags = 0;
		attachments[global_attachment_count].format = sf_graphics_vulkan_format_from_format(render_target->color_format);
		attachments[global_attachment_count].samples = sf_graphics_vulkan_sample_count_from_sample_count(SF_GRAPHICS_SAMPLE_COUNT_1);
		attachments[global_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[global_attachment_count].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[global_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[global_attachment_count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[global_attachment_count].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (render_target->samples == SF_GRAPHICS_SAMPLE_COUNT_1)
			attachments[global_attachment_count].finalLayout = render_target->will_be_presented ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else
			attachments[global_attachment_count].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		color_references[i].attachment = global_attachment_count;
		color_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // FIXME(samuel): is this layout right?

		++global_attachment_count;
	}
	
	if (render_target->depth_stencil_format) {
		attachments[global_attachment_count].flags = 0;
		attachments[global_attachment_count].format = sf_graphics_vulkan_format_from_format(render_target->depth_stencil_format);
		attachments[global_attachment_count].samples = sf_graphics_vulkan_sample_count_from_sample_count(render_target->samples);
		attachments[global_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[global_attachment_count].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[global_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[global_attachment_count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[global_attachment_count].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[global_attachment_count].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depth_stencil_reference.attachment = global_attachment_count;
		depth_stencil_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		++global_attachment_count;
	}

	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = render_target->color_attachment_count;
	subpass.pColorAttachments = color_references;
	subpass.pResolveAttachments = resolve_references;
	subpass.pDepthStencilAttachment = &depth_stencil_reference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

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
	render_pass_info.attachmentCount = global_attachment_count;
	render_pass_info.pAttachments = attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &subpass_dependency;

	if (!SF_VULKAN_CHECK(vkCreateRenderPass(r->vk_device, &render_pass_info, r->vk_allocation_callbacks, &render_target->vk_render_pass)))
		render_target->vk_render_pass = VK_NULL_HANDLE;
}

sf_private void sf_graphics_vulkan_render_target_framebuffer_init(struct sf_graphics_renderer *r, struct sf_graphics_render_target *render_target) {
	struct sf_graphics_render_target_attachment_array attachments = {0};
	VkFramebufferCreateInfo info = {0};

	if (!r || !render_target || !r->vk_device || render_target->vk_framebuffer)
		return;

	render_target->vk_framebuffer = VK_NULL_HANDLE;
	
	sf_graphics_vulkan_render_target_load_attachment_array(r, render_target, &attachments);

	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.renderPass = render_target->vk_render_pass;
	info.attachmentCount = attachments.size;
	info.pAttachments = attachments.data;
	info.width = render_target->width;
	info.height = render_target->height;
	info.layers = 1;

	if (!SF_VULKAN_CHECK(vkCreateFramebuffer(r->vk_device, &info, r->vk_allocation_callbacks, &render_target->vk_framebuffer)))
		render_target->vk_framebuffer = VK_NULL_HANDLE;
}

sf_private struct sf_graphics_render_target *sf_graphics_vulkan_render_target_init(struct sf_graphics_renderer *r, u32 width, u32 height, enum sf_graphics_format color_attachment_format, u32 color_attachment_count, sf_handle *color_attachments, enum sf_graphics_format depth_stencil_format, enum sf_graphics_sample_count samples) {
	u32 i = 0;
	struct sf_graphics_render_target *render_target = sf_graphics_get_render_target_from_pool(r);

	if (!render_target || !width || !height)
		return NULL;

	render_target->owns_color_attachments = !color_attachments;
	render_target->width = width;
	render_target->height = height;
	render_target->samples = samples;
	render_target->depth_stencil_format = depth_stencil_format;

	if (color_attachment_count > SF_SIZE(render_target->color_attachments))
		goto error;

	for (i = 0; i < color_attachment_count; ++i) {
		if (!color_attachments) {
			sf_graphics_image_usage_flags usage = SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_IMAGE_USAGE_TRANSFER_SOURCE | SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION | SF_GRAPHICS_IMAGE_USAGE_SAMPLED;
			render_target->color_attachments[i] = sf_graphics_image_init(r, SF_GRAPHICS_IMAGE_TYPE_2D, usage, color_attachment_format, samples, width, height, 1);
		} else {
			render_target->color_attachments[i] = color_attachments[i];
		}

		if (!render_target->color_attachments[i])
			goto error;
		

		// FIXME(SamueL):  there needs to be a second view forced to have a 1 in the alpha channel??
	}
	render_target->color_attachment_count = color_attachment_count;

	if (samples != SF_GRAPHICS_SAMPLE_COUNT_1) {
		for (i = 0; i < color_attachment_count; ++i) {
			sf_graphics_image_usage_flags usage = SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT | SF_GRAPHICS_IMAGE_USAGE_TRANSFER_SOURCE | SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION | SF_GRAPHICS_IMAGE_USAGE_SAMPLED;
			render_target->resolve_attachments[i] = sf_graphics_image_init(r, SF_GRAPHICS_IMAGE_TYPE_2D, usage, color_attachment_format, SF_GRAPHICS_SAMPLE_COUNT_1, width, height, 1);
			if (!render_target->resolve_attachments[i])
				goto error;
		}
		render_target->resolve_attachment_count = color_attachment_count;
	}


	if (depth_stencil_format != SF_GRAPHICS_FORMAT_UNDEFINED) {
		sf_graphics_image_usage_flags usage = SF_GRAPHICS_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT | SF_GRAPHICS_IMAGE_USAGE_SAMPLED;
		render_target->depth_stencil_attachment = sf_graphics_image_init(r, SF_GRAPHICS_IMAGE_TYPE_2D, usage, depth_stencil_format, samples, width, height, 1);

		if (!render_target->depth_stencil_attachment)
			goto error;
	}

	sf_graphics_vulkan_render_target_render_pass_init(r, render_target);
	if (!render_target->vk_render_pass)
		goto error;

	sf_graphics_vulkan_render_target_framebuffer_init(r, render_target);
	if (!render_target->vk_framebuffer)
		goto error;

	return render_target;

error:
	sf_graphics_vulkan_render_target_deinit(r, render_target);
	return NULL;
}

sf_private void sf_graphics_vulkan_descriptor_set_layouts_deinit(struct sf_graphics_renderer *r) {
	if (r && r->vk_device) {
		if (r->vk_dynamic_uniform_descriptor_set_layout) {
			vkDestroyDescriptorSetLayout(r->vk_device, r->vk_dynamic_uniform_descriptor_set_layout, r->vk_allocation_callbacks);
			r->vk_dynamic_uniform_descriptor_set_layout = VK_NULL_HANDLE;
		}

		if (r->vk_uniform_descriptor_set_layout) {
			vkDestroyDescriptorSetLayout(r->vk_device, r->vk_uniform_descriptor_set_layout, r->vk_allocation_callbacks);
			r->vk_uniform_descriptor_set_layout = VK_NULL_HANDLE;
		}

		if (r->vk_image_descriptor_set_layout) {
			vkDestroyDescriptorSetLayout(r->vk_device, r->vk_image_descriptor_set_layout, r->vk_allocation_callbacks);
			r->vk_image_descriptor_set_layout = VK_NULL_HANDLE;
		}
	}
}

sf_private void sf_graphics_vulkan_descriptor_set_layouts_init(struct sf_graphics_renderer *r) {
	VkDescriptorSetLayoutBinding image_bindings[1] = {0};
	VkDescriptorSetLayoutBinding uniform_bindings[1] = {0};
	VkDescriptorSetLayoutBinding dynamic_uniform_bindings[1] = {0};
	VkDescriptorSetLayoutCreateInfo info = {0};

	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;

	image_bindings[0].binding = 0;
	image_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	image_bindings[0].descriptorCount = 1;
	image_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	image_bindings[0].pImmutableSamplers = NULL;

	info.bindingCount = SF_SIZE(image_bindings);
	info.pBindings = image_bindings;

	if (SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_image_descriptor_set_layout)))
		goto error;

	uniform_bindings[0].binding = 0;
	uniform_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_bindings[0].descriptorCount = 1;
	uniform_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uniform_bindings[0].pImmutableSamplers = NULL;

	info.bindingCount = SF_SIZE(uniform_bindings);
	info.pBindings = uniform_bindings;

	if (SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_uniform_descriptor_set_layout)))
		goto error;

	dynamic_uniform_bindings[0].binding = 0;
	dynamic_uniform_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	dynamic_uniform_bindings[0].descriptorCount = 1;
	dynamic_uniform_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	dynamic_uniform_bindings[0].pImmutableSamplers = NULL;

	info.bindingCount = SF_SIZE(dynamic_uniform_bindings);
	info.pBindings = dynamic_uniform_bindings;

	if (SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_dynamic_uniform_descriptor_set_layout)))
		goto error;

	return;
error:
	sf_graphics_vulkan_descriptor_set_layouts_deinit(r);
}

sf_private void sf_graphics_vulkan_pipeline_layouts_deinit(struct sf_graphics_renderer *r) {
	if (r && r->vk_device) {
		if (r->vk_general_pipeline_layout) {
			vkDestroyPipelineLayout(r->vk_device, r->vk_general_pipeline_layout, r->vk_allocation_callbacks);
			r->vk_general_pipeline_layout = VK_NULL_HANDLE;
		}
	}
}

sf_public void sf_graphics_vulkan_pipeline_layouts_init(struct sf_graphics_renderer *r) {
	VkPipelineLayoutCreateInfo info = {0};
	VkDescriptorSetLayout layouts[4] = {0};

	layouts[0] = r->vk_dynamic_uniform_descriptor_set_layout;
	layouts[1] = r->vk_uniform_descriptor_set_layout;
	layouts[2] = r->vk_image_descriptor_set_layout;
	layouts[3] = r->vk_image_descriptor_set_layout;

	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.setLayoutCount = SF_SIZE(layouts);
	info.pSetLayouts = layouts;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = NULL;

	if (!SF_VULKAN_CHECK(vkCreatePipelineLayout(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_general_pipeline_layout)))
		r->vk_general_pipeline_layout = VK_NULL_HANDLE;
}

sf_private void sf_graphics_vulkan_global_descriptor_pool_init(struct sf_graphics_renderer *r) {
	VkDescriptorPoolSize sizes[8] = {0};
	VkDescriptorPoolCreateInfo info = {0};

	sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sizes[0].descriptorCount = 256;

	sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sizes[1].descriptorCount = 256;

	sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	sizes[2].descriptorCount = 256;

	sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	sizes[3].descriptorCount = 256;

	sizes[4].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	sizes[4].descriptorCount = 256;

	sizes[5].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	sizes[5].descriptorCount = 256;

	sizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	sizes[6].descriptorCount = 256;

	sizes[7].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	sizes[7].descriptorCount = 256;

	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.pNext = NULL;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	info.maxSets = 256 * SF_SIZE(sizes);
	info.poolSizeCount = SF_SIZE(sizes);
	info.pPoolSizes = sizes;

	if (!SF_VULKAN_CHECK(vkCreateDescriptorPool(r->vk_device, &info, r->vk_allocation_callbacks, &r->vk_global_descriptor_pool)))
		r->vk_global_descriptor_pool = VK_NULL_HANDLE;
}


sf_public void sf_graphics_renderer_deinit(struct sf_graphics_renderer *r) {
	if (!r)
		return;

	sf_graphics_vulkan_pipeline_layouts_deinit(r);
	sf_graphics_vulkan_descriptor_set_layouts_deinit(r);
	sf_graphics_vulkan_swapchain_resources_deinit(r);

	if (r->vk_device) {
		if (r->vk_global_descriptor_pool) {
			vkDestroyDescriptorPool(r->vk_device, r->vk_global_descriptor_pool, r->vk_allocation_callbacks);
			r->vk_global_descriptor_pool = VK_NULL_HANDLE;
		}

		vkDestroyDevice(r->vk_device, r->vk_allocation_callbacks);
		r->vk_device = VK_NULL_HANDLE;
	}

	if (r->vk_instance) {
		if (r->vk_surface) {
			vkDestroySurfaceKHR(r->vk_instance, r->vk_surface, r->vk_allocation_callbacks);
			r->vk_surface = VK_NULL_HANDLE;
		}

		if (r->vk_validation_messenger && r->vk_destroy_debug_utils_messenger_ext) {
			r->vk_destroy_debug_utils_messenger_ext(r->vk_instance, r->vk_validation_messenger, r->vk_allocation_callbacks);
			r->vk_validation_messenger = VK_NULL_HANDLE;
		}

		vkDestroyInstance(r->vk_instance, r->vk_allocation_callbacks);
		r->vk_instance = VK_NULL_HANDLE;
	}
}

sf_public struct sf_graphics_renderer *sf_graphics_renderer_init(struct sf_arena *arena, struct sf_graphics_renderer_info *info) {
	struct sf_graphics_renderer *r = sf_arena_allocate(arena, sizeof(struct sf_graphics_renderer));

	if (!r)
		return NULL;

	r->plataform_data = info->plataform_data;

	sf_arena_scratch(arena, SF_KB(20), &r->arena);
	if (!r->arena.data)
		goto error;

	sf_graphics_vulkan_instance_init(arena, r, info);
	if (!r->vk_instance)
		goto error;

	sf_graphics_vulkan_load_functions(r);
	if (!r->vk_write_sampler_descriptors_ext)
		goto error;

	if (info->vk_request_enable_validation_layers) {
		sf_graphics_vulkan_validation_messenger_init(r);
	}

	info->plataform_vulkan_surface_init(info->plataform_data, r);
	if (!r->vk_surface)
		goto error;

	sf_graphics_vulkan_find_suitable_physical_device(arena, r, info);
	if (!r->vk_physical_device)
		goto error;

	sf_graphics_vulkan_device_init(r, info);
	if (!r->vk_device)
		goto error;

	sf_graphics_vulkan_load_device_queues(r);
	if (!r->vk_graphics_queue || !r->vk_present_queue)
		goto error;

	sf_arena_scratch(&r->arena, SF_KB(2), &r->swapchain_arena);
	if (!r->swapchain_arena.data)
		goto error;

	sf_graphics_vulkan_swapchain_resources_init(&r->swapchain_arena, r);
	if (!r->vk_swapchain)
		goto error;

	sf_graphics_vulkan_descriptor_set_layouts_init(r);
	if (!r->vk_image_descriptor_set_layout || !r->vk_uniform_descriptor_set_layout || !r->vk_dynamic_uniform_descriptor_set_layout)
		goto error;

	sf_graphics_vulkan_pipeline_layouts_init(r);
	if (!r->vk_general_pipeline_layout)
		goto error;

	sf_graphics_vulkan_global_descriptor_pool_init(r);
	if (!r->vk_global_descriptor_pool)
		goto error;

	return r;

error:
	sf_graphics_renderer_deinit(r);
	return NULL;
}

// NOTE(samuel): taken from the vulkan header as reference.
#define SF_VULKAN_MAX_DESCRIPTOR_POOL_SIZE_COUNT 10

sf_private VkDescriptorType sf_graphics_vulkan_descriptor_type_from_type(enum sf_graphics_descriptor_type type) {
	switch (type) {
		case SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
			//		case SF_GRAPHICS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			//			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			//		case SF_GRAPHICS_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			//			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			//		case SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			//			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			//		case SF_GRAPHICS_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			//			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		default:
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}

sf_public void sf_graphics_begin_frame(struct sf_graphics_renderer *r) {
	// https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html
	VkResult result = 0;
	u32 current_frame_index = 0;
	u32 current_image_index = 0;
	VkFence current_in_flight_fence = VK_NULL_HANDLE;
	VkSemaphore current_image_acquired_semaphore = VK_NULL_HANDLE;
	struct sf_graphics_command_buffer *current_command_buffer = NULL;
	VkCommandBufferBeginInfo command_buffer_begin_info = {0};
	VkClearValue clear_values[2] = {0};
	struct sf_graphics_render_target *current_render_target = NULL;
	VkRenderPassBeginInfo render_pass_begin_info = {0};
	VkViewport viewport = {0};
	VkRect2D scissor = {0};

	current_frame_index = r->vk_swapchain_current_frame_index;
	current_in_flight_fence = r->vk_swapchain_in_flight_fences[current_frame_index];
	current_command_buffer = sf_graphics_command_buffer_from_handle(r, r->main_command_buffers[current_frame_index]);
	current_image_acquired_semaphore = r->vk_swapchain_image_acquired_semaphores[current_frame_index];

	vkWaitForFences(r->vk_device, 1, &current_in_flight_fence, VK_TRUE, (uint64_t)-1);
	vkResetFences(r->vk_device, 1, &current_in_flight_fence);
	vkResetCommandPool(r->vk_device, current_command_buffer->vk_command_pool, 0);

	result = vkAcquireNextImageKHR(r->vk_device, r->vk_swapchain, (u64)-1, current_image_acquired_semaphore, VK_NULL_HANDLE, &r->vk_swapchain_current_image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_ERROR_SURFACE_LOST_KHR) {
		SF_VULKAN_CHECK(result);
		vkDeviceWaitIdle(r->vk_device);
		sf_graphics_vulkan_swapchain_resources_deinit(r);
		sf_graphics_vulkan_swapchain_resources_init(&r->swapchain_arena, r);
		r->swapchain_skip_end_frame = SF_TRUE;
		return;
	}

	current_image_index = r->vk_swapchain_current_image_index;
	current_render_target = sf_graphics_render_target_from_handle(r, r->swapchain_render_targets[current_image_index]);

	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.pNext = NULL;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	command_buffer_begin_info.pInheritanceInfo = NULL;

	vkBeginCommandBuffer(current_command_buffer->vk_command_buffer, &command_buffer_begin_info);

	clear_values[0].color.float32[0] = current_render_target->color_clear_value.data.rgba.r;
	clear_values[0].color.float32[1] = current_render_target->color_clear_value.data.rgba.g;
	clear_values[0].color.float32[2] = current_render_target->color_clear_value.data.rgba.b;
	clear_values[0].color.float32[3] = current_render_target->color_clear_value.data.rgba.a;
	clear_values[1].depthStencil.depth = current_render_target->depth_stencil_clear_value.data.depth_stencil.depth;	  // 1.0F
	clear_values[1].depthStencil.stencil = current_render_target->depth_stencil_clear_value.data.depth_stencil.stencil; // 0.0F

	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = NULL;
	render_pass_begin_info.renderPass = current_render_target->vk_render_pass;
	render_pass_begin_info.framebuffer = current_render_target->vk_framebuffer;
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent.width = current_render_target->width;
	render_pass_begin_info.renderArea.extent.height = current_render_target->height;

	render_pass_begin_info.clearValueCount = SF_SIZE(clear_values);
	render_pass_begin_info.pClearValues = clear_values;

	vkCmdBeginRenderPass(current_command_buffer->vk_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	viewport.x = 0.0F;
	viewport.y = 0.0F;
	viewport.width = (float)current_render_target->width;
	viewport.height = (float)current_render_target->height;
	viewport.minDepth = 0.0F;
	viewport.maxDepth = 0.0F;

	vkCmdSetViewport(current_command_buffer->vk_command_buffer, 0, 1, &viewport);

	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = current_render_target->width;
	scissor.extent.height = current_render_target->height;

	vkCmdSetScissor(current_command_buffer->vk_command_buffer, 0, 1, &scissor);
}

sf_public void sf_graphics_end_frame(struct sf_graphics_renderer *r) {
	u32 current_frame_index = 0;
	struct sf_graphics_command_buffer *current_command_buffer = NULL;
	u32 current_image_index = 0;
	VkSemaphore current_draw_complete_semaphore = VK_NULL_HANDLE;
	VkSemaphore current_image_acquired_semaphore = VK_NULL_HANDLE;
	VkFence current_in_flight_fence = VK_NULL_HANDLE;
	VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = {0};
	VkPresentInfoKHR present_info = {0};
	VkResult result = VK_SUCCESS;

	if (r->swapchain_skip_end_frame) {
		r->swapchain_skip_end_frame = SF_FALSE;
		return;
	}
	current_frame_index = r->vk_swapchain_current_image_index;
	current_command_buffer = sf_graphics_command_buffer_from_handle(r, r->main_command_buffers[current_frame_index]);

	current_image_index = r->vk_swapchain_current_frame_index;
	current_draw_complete_semaphore = r->vk_swapchain_draw_complete_semaphores[current_image_index];
	current_image_acquired_semaphore = r->vk_swapchain_image_acquired_semaphores[current_frame_index];
	current_in_flight_fence = r->vk_swapchain_in_flight_fences[current_frame_index];
	stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	vkCmdEndRenderPass(current_command_buffer->vk_command_buffer);
	vkEndCommandBuffer(current_command_buffer->vk_command_buffer);

	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &current_image_acquired_semaphore;
	submit_info.pWaitDstStageMask = &stage;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &current_command_buffer->vk_command_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &current_draw_complete_semaphore;

	vkQueueSubmit(r->vk_graphics_queue, 1, &submit_info, current_in_flight_fence);

	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = NULL;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &current_draw_complete_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &r->vk_swapchain;
	present_info.pImageIndices = &current_image_index;
	present_info.pResults = NULL;

	result = vkQueuePresentKHR(r->vk_present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		SF_VULKAN_CHECK(result);
		vkDeviceWaitIdle(r->vk_device);
		sf_graphics_vulkan_swapchain_resources_deinit(r);
		sf_graphics_vulkan_swapchain_resources_init(&r->swapchain_arena, r);
	}
}

sf_private void sf_graphics_glfw_platform_framebuffer_resize_callback(GLFWwindow *window, i32 width, i32 height) {
	struct sf_graphics_glfw_platform *platform = (struct sf_graphics_glfw_platform *)glfwGetWindowUserPointer(window);

	if (!platform)
		return;

	platform->window_width = width;
	platform->window_height = height;
}

sf_public struct sf_graphics_glfw_platform *sf_graphics_glfw_platform_init(struct sf_arena *arena, i32 width, i32 height, struct sf_string const *title) {
	struct sf_string window_title = {0};

	struct sf_graphics_glfw_platform *platform = sf_arena_allocate(arena, sizeof(struct sf_graphics_glfw_platform));
	if (!platform)
		return NULL;

	if (!glfwInit())
		return NULL;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	sf_string_null_terminate(arena, title, &window_title);
	platform->window = glfwCreateWindow(width, height, window_title.data, NULL, NULL);

	if (!platform->window)
		goto error;

	glfwSetWindowUserPointer(platform->window, platform);
	glfwSetFramebufferSizeCallback(platform->window, sf_graphics_glfw_platform_framebuffer_resize_callback);
	glfwGetFramebufferSize(platform->window, &platform->window_width, &platform->window_height);

	return platform;

error:
	sf_graphics_glfw_platform_deinit(platform);
	return NULL;
}

sf_public void sf_graphics_glfw_platform_deinit(struct sf_graphics_glfw_platform *platform) {
	if (!platform)
		return;

	if (platform->window) {
		glfwDestroyWindow(platform->window);
		platform->window = NULL;
	}

	platform->window_width = 0;
	platform->window_height = 0;

	glfwTerminate();
}

sf_public void sf_graphics_glfw_platform_process_events(struct sf_graphics_glfw_platform *platform) {
	if (!platform)
		return;

	glfwPollEvents();
}

sf_public sf_bool sf_graphics_glfw_platform_should_close(struct sf_graphics_glfw_platform *platform) {
	if (!platform)
		return SF_FALSE;

	return glfwWindowShouldClose(platform->window);
}

sf_private void sf_graphics_glfw_platform_vulkan_surface_init(void *data, struct sf_graphics_renderer *r) {
	struct sf_graphics_glfw_platform *platform = (struct sf_graphics_glfw_platform *)data;

	if (!platform || !r || !r->vk_instance)
		return;

	if (!SF_VULKAN_CHECK(glfwCreateWindowSurface(r->vk_instance, platform->window, r->vk_allocation_callbacks, &r->vk_surface)))
		r->vk_surface = VK_NULL_HANDLE;
}

sf_private void sf_graphics_glfw_platform_request_swapchain_dimensions(void *data, struct sf_graphics_renderer *r) {
	struct sf_graphics_glfw_platform *platform = (struct sf_graphics_glfw_platform *)data;

	if (!platform || !r)
		return;

	r->vk_swapchain_width = platform->window_width;
	r->vk_swapchain_height = platform->window_height;
}

sf_public void sf_graphics_glfw_platform_fill_renderer_info(struct sf_arena *arena, struct sf_graphics_glfw_platform *platform, struct sf_graphics_renderer_info *info) {
	u32 base_instance_extension_count = 0;
	char const **base_instance_extensions = NULL;

	u32 required_instance_extension_count = 0;

	sf_local_persist char const *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
	sf_local_persist char const *device_extensions[] = {
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME,

#ifdef __APPLE__
	    , "VK_KHR_portability_subset"
#endif
	};

	if (!platform || !info)
		return;

	info->plataform_data = platform;
	info->request_enable_vsync = SF_TRUE;
	info->width = platform->window_width;
	info->height = platform->window_height;
	info->plataform_vulkan_surface_init = sf_graphics_glfw_platform_vulkan_surface_init;
	info->plataform_request_swapchain_dimensions = sf_graphics_glfw_platform_request_swapchain_dimensions;

	info->application_name.data = "SF";
	info->application_name.size = sizeof("SF");

	base_instance_extensions = glfwGetRequiredInstanceExtensions(&base_instance_extension_count);

#ifdef __APPLE__
	required_instance_extension_count = base_instance_extension_count + 4;
#else
	required_instance_extension_count = base_instance_extension_count + 3;
#endif
	info->vk_instance_extensions = sf_arena_allocate(arena, (required_instance_extension_count) * sizeof(char const *));
	if (info->vk_instance_extensions) {
		u32 i = 0;
		info->vk_instance_extension_count = required_instance_extension_count;

		for (i = 0; i < base_instance_extension_count; ++i)
			info->vk_instance_extensions[i] = base_instance_extensions[i];

		info->vk_instance_extensions[base_instance_extension_count + 0] = VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME;
		info->vk_instance_extensions[base_instance_extension_count + 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		info->vk_instance_extensions[base_instance_extension_count + 2] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#ifdef __APPLE__
		info->vk_instance_extensions[base_instance_extension_count + 3] = "VK_KHR_portability_enumeration";
#endif
	}

	info->vk_instance_layer_count = SF_SIZE(validation_layers);
	info->vk_instance_layers = validation_layers;

	info->vk_device_extension_count = SF_SIZE(device_extensions);
	info->vk_device_extensions = device_extensions;
}
