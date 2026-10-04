#include "sf_graphics.h"

#include "sf_image.h"

#include <stdio.h>

#define SF_VULKAN_CHECK(e) sf_graphics_vulkan_check_result((e), #e, __LINE__, __FILE__)
#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)

sf_private char const *sf_graphics_get_string_from_vulkan_result(VkResult vk_result) {
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

sf_private sf_bool sf_graphics_vulkan_check_result(VkResult result, char const *what, int line, char const *file) {
	char const *result_string = sf_graphics_get_string_from_vulkan_result(result);
	if (result != VK_SUCCESS)
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

sf_private void sf_graphics_vulkan_load_available_instance_extensions(struct sf_arena *arena, struct sf_graphics_vulkan_extension_properties_array *out) {
	u32 count = 0;

	out->data = NULL;
	out->size = 0;

	if (!SF_VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, NULL)))
		return;

	out->data = sf_arena_allocate(arena, sizeof(*out->data) * count);
	if (!out->data)
		return;

	if (!SF_VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, out->data)))
		return;

	out->size = count;
}

sf_private void sf_graphics_vulkan_load_available_device_extensions(struct sf_arena *arena, VkPhysicalDevice device, struct sf_graphics_vulkan_extension_properties_array *out) {
	u32 count = 0;

	if (!SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL)))
		return;

	out->data = sf_arena_allocate(arena, sizeof(*out->data) * count);
	if (!out->data)
		return;

	if (!SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &count, out->data)))
		return;

	out->size = count;
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

struct sf_graphics_physical_device_array {
	u32 size;
	VkPhysicalDevice *data;
};

sf_private void sf_graphics_vulkan_load_physical_devices(struct sf_arena *arena, VkInstance instance, struct sf_graphics_physical_device_array *out) {
	u32 count = 0;

	if (!arena || !instance || !out)
		return;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &count, NULL)))
		return;

	out->data = sf_arena_allocate(arena, count * sizeof(*out->data));
	if (!out->data)
		return;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &count, out->data)))
		return;

	out->size = count;
}

struct sf_graphics_queue_family_properties_array {
	u32 size;
	VkQueueFamilyProperties *data;
};

sf_private void sf_graphics_load_queue_family_properties(struct sf_arena *arena, VkPhysicalDevice device, struct sf_graphics_queue_family_properties_array *out) {
	if (!arena || !device || !out)
		return;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &out->size, NULL);
	if (!out->size)
		return;

	out->data = sf_arena_allocate(arena, out->size * sizeof(*out->data));
	if (!out->data)
		return;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &out->size, out->data);
}

struct sf_graphics_vulkan_family_indices {
	u32 graphics_queue_family_index;
	u32 present_queue_family_index;
};

sf_private void sf_graphics_vulkan_find_suitable_queue_family_indices(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, struct sf_graphics_vulkan_family_indices *out) {
	u32 i = 0;
	struct sf_graphics_queue_family_properties_array properties = {0};

	if (!arena || !device || !surface || !out)
		return;

	out->graphics_queue_family_index = (u32)-1;
	out->present_queue_family_index = (u32)-1;

	sf_graphics_load_queue_family_properties(arena, device, &properties);

	if (!properties.size || !properties.data)
		return;

	for (i = 0; i < properties.size && (out->graphics_queue_family_index == (u32)-1 || out->present_queue_family_index == (u32)-1); ++i) {
		VkBool32 supports_surface = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_surface);

		if (supports_surface)
			out->present_queue_family_index = i;

		if (properties.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			out->graphics_queue_family_index = i;
	}
}

sf_private VkPhysicalDevice sf_graphics_vulkan_find_suitable_physical_device(struct sf_arena *arena, VkInstance vk_instance, VkSurfaceKHR vk_surface, struct sf_graphics_init_device_info const *init_info, struct sf_graphics_vulkan_family_indices *out_families) {
	u32 i = 0;
	VkPhysicalDevice candidate = VK_NULL_HANDLE;
	VkPhysicalDevice last_candidate = VK_NULL_HANDLE;
	struct sf_graphics_physical_device_array physical_devices = {0};

	if (!arena || !vk_instance || !vk_surface || !init_info || !out_families)
		return VK_NULL_HANDLE;

	out_families->graphics_queue_family_index = (u32)-1;
	out_families->present_queue_family_index = (u32)-1;

	sf_graphics_vulkan_load_physical_devices(arena, vk_instance, &physical_devices);
	if (!physical_devices.data || !physical_devices.size)
		return VK_NULL_HANDLE;

	for (i = 0; i < physical_devices.size; ++i) {
		VkPhysicalDeviceProperties properties = {0};
		VkPhysicalDeviceFeatures features = {0};
		VkPhysicalDevice device = VK_NULL_HANDLE;

		device = physical_devices.data[i];
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		sf_graphics_vulkan_find_suitable_queue_family_indices(arena, device, vk_surface, out_families);
		if (out_families->graphics_queue_family_index == (u32)-1 || out_families->present_queue_family_index == (u32)-1)
			continue;

		if (!sf_graphics_vulkan_are_device_extensions_available(arena, device, init_info->vk.device_extension_count, init_info->vk.device_extensions))
			continue;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			candidate = device;
			last_candidate = candidate;
			goto finish;
		} else {
			last_candidate = device; // NOTE(samuel): Found a candidate, but keep looking for a discrete GPU.
		}
	}

	candidate = last_candidate;

finish:
	if (candidate != last_candidate)
		sf_graphics_vulkan_find_suitable_queue_family_indices(arena, candidate, vk_surface, out_families);

	return candidate;
}

sf_private void sf_graphics_deinit_device(struct sf_graphics_device *device) {
	if (device) {
		if (device->vk.device) {
			if (device->vk.global_descriptor_pool) {
				vkDestroyDescriptorPool(device->vk.device, device->vk.global_descriptor_pool, device->vk.allocation_callbacks);
				device->vk.global_descriptor_pool = VK_NULL_HANDLE;
			}

			if (device->vk.general_pipeline_layout) {
				vkDestroyPipelineLayout(device->vk.device, device->vk.general_pipeline_layout, device->vk.allocation_callbacks);
				device->vk.general_pipeline_layout = VK_NULL_HANDLE;
			}

			if (device->vk.dynamic_uniform_descriptor_set_layout) {
				vkDestroyDescriptorSetLayout(device->vk.device, device->vk.dynamic_uniform_descriptor_set_layout, device->vk.allocation_callbacks);
				device->vk.dynamic_uniform_descriptor_set_layout = VK_NULL_HANDLE;
			}

			if (device->vk.uniform_descriptor_set_layout) {
				vkDestroyDescriptorSetLayout(device->vk.device, device->vk.uniform_descriptor_set_layout, device->vk.allocation_callbacks);
				device->vk.uniform_descriptor_set_layout = VK_NULL_HANDLE;
			}

			if (device->vk.image_descriptor_set_layout) {
				vkDestroyDescriptorSetLayout(device->vk.device, device->vk.image_descriptor_set_layout, device->vk.allocation_callbacks);
				device->vk.image_descriptor_set_layout = VK_NULL_HANDLE;
			}

			if (device->vk.linear_sampler) {
				vkDestroySampler(device->vk.device, device->vk.linear_sampler, device->vk.allocation_callbacks);	
				device->vk.linear_sampler = VK_NULL_HANDLE;
			}

			vkDestroyDevice(device->vk.device, device->vk.allocation_callbacks);
			device->vk.device = VK_NULL_HANDLE;
		}

		if (device->vk.surface) {
			vkDestroySurfaceKHR(device->vk.instance, device->vk.surface, device->vk.allocation_callbacks);
			device->vk.surface = VK_NULL_HANDLE;
		}

		if (device->vk.destroy_debug_utils_messenger_ext && device->vk.validation_messenger) {
			device->vk.destroy_debug_utils_messenger_ext(device->vk.instance, device->vk.validation_messenger, device->vk.allocation_callbacks);
			device->vk.validation_messenger = VK_NULL_HANDLE;
		}

		if (device->vk.instance) {
			vkDestroyInstance(device->vk.instance, device->vk.allocation_callbacks);
			device->vk.instance = VK_NULL_HANDLE;
		}
	}
}

sf_private struct sf_graphics_device *sf_graphics_init_device(struct sf_arena *arena, struct sf_graphics_init_device_info const *init_info) {
	u32 vk_api_version = 0;
	u32 vk_required_api_version = VK_MAKE_API_VERSION(0, 1, 0, 0);
	struct sf_string null_terminated_app_name = {0};
	struct sf_graphics_device *device = NULL;

	if (!arena || !init_info)
		return NULL;

	device = sf_arena_allocate(arena, sizeof(*device));
	if (!device)
		return NULL;
	
	device->info = *init_info;
	device->vk.allocation_callbacks = init_info->vk.allocation_callbacks;

	vkEnumerateInstanceVersion(&vk_api_version);
	if (vk_api_version < vk_required_api_version)
		goto error;

	sf_string_null_terminate(arena, &init_info->application_name, &null_terminated_app_name);
	if (!null_terminated_app_name.data || !null_terminated_app_name.size)
		goto error;

	if (!sf_graphics_vulkan_are_instance_extensions_available(arena, init_info->vk.instance_extension_count, init_info->vk.instance_extensions))
		goto error;

	{
		VkApplicationInfo app_info = {0};
		VkInstanceCreateInfo instance_info = {0};

		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = null_terminated_app_name.data;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "sf";
		app_info.engineVersion = 1;
		app_info.apiVersion = VK_API_VERSION_1_0;

		instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_info.pNext = NULL;
#ifdef __APPLE__
		instance_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
		instance_info.flags = 0;
#endif
		instance_info.pApplicationInfo = &app_info;
		instance_info.enabledLayerCount = init_info->vk.instance_layer_count;
		instance_info.ppEnabledLayerNames = init_info->vk.instance_layers;
		instance_info.enabledExtensionCount = init_info->vk.instance_extension_count;
		instance_info.ppEnabledExtensionNames = init_info->vk.instance_extensions;

		if (!SF_VULKAN_CHECK(vkCreateInstance(&instance_info, device->vk.allocation_callbacks, &device->vk.instance))) {
			device->vk.instance = VK_NULL_HANDLE;
			goto error;
		}
	}

	device->vk.create_debug_utils_messenger_ext = SF_VULKAN_PROC(vkCreateDebugUtilsMessengerEXT, device->vk.instance);
	device->vk.destroy_debug_utils_messenger_ext = SF_VULKAN_PROC(vkDestroyDebugUtilsMessengerEXT, device->vk.instance);
	if (!device->vk.create_debug_utils_messenger_ext || !device->vk.destroy_debug_utils_messenger_ext)
		goto error;


	{
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
		info.pfnUserCallback = sf_graphics_vulkan_log;
		info.pUserData = NULL;

		if (!SF_VULKAN_CHECK(device->vk.create_debug_utils_messenger_ext(device->vk.instance, &info, device->vk.allocation_callbacks, &device->vk.validation_messenger))) {
			device->vk.validation_messenger = VK_NULL_HANDLE;
			goto error;
		}
	}

	init_info->init_surface(init_info->plataform_data, device);
	if (!device->vk.surface)
		goto error;

	{
		struct sf_graphics_vulkan_family_indices families = {0};

		device->vk.physical_device = sf_graphics_vulkan_find_suitable_physical_device(arena, device->vk.instance, device->vk.surface, init_info, &families);
		if (!device->vk.physical_device)
			goto error;

		device->vk.graphics_queue_family_index = families.graphics_queue_family_index;
		device->vk.present_queue_family_index = families.present_queue_family_index;
	}

	vkGetPhysicalDeviceFeatures(device->vk.physical_device, &device->vk.physical_device_features);

	{
		float priority = 1.0F;
		VkDeviceQueueCreateInfo queue_infos[2] = {0};
		VkDeviceCreateInfo device_info = {0};

		queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_infos[0].pNext = NULL;
		queue_infos[0].flags = 0;
		queue_infos[0].queueFamilyIndex = device->vk.graphics_queue_family_index;
		queue_infos[0].queueCount = 1;
		queue_infos[0].pQueuePriorities = &priority;

		queue_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_infos[1].pNext = NULL;
		queue_infos[1].flags = 0;
		queue_infos[1].queueFamilyIndex = device->vk.present_queue_family_index;
		queue_infos[1].queueCount = 1;
		queue_infos[1].pQueuePriorities = &priority;

		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pNext = NULL;
		device_info.flags = 0;
		if (device->vk.graphics_queue_family_index == device->vk.present_queue_family_index)
			device_info.queueCreateInfoCount = 1;
		else
			device_info.queueCreateInfoCount = SF_SIZE(queue_infos);
		device_info.pQueueCreateInfos = queue_infos;
		device_info.enabledLayerCount = 0;	// NOTE(samuel): deprecated
		device_info.ppEnabledLayerNames = NULL; // NOTE(samuel): deprecated
		device_info.enabledExtensionCount = init_info->vk.device_extension_count;
		device_info.ppEnabledExtensionNames = init_info->vk.device_extensions;
		device_info.pEnabledFeatures = NULL; // NOTE(samuel): deprecated

		if (!SF_VULKAN_CHECK(vkCreateDevice(device->vk.physical_device, &device_info, device->vk.allocation_callbacks, &device->vk.device))) {
			device->vk.device = VK_NULL_HANDLE;
			goto error;
		}
	}

	vkGetDeviceQueue(device->vk.device, device->vk.graphics_queue_family_index, 0, &device->vk.graphics_queue);
	vkGetDeviceQueue(device->vk.device, device->vk.present_queue_family_index, 0, &device->vk.present_queue);

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->vk.physical_device, device->vk.surface, &device->vk.physical_device_surface_capabilities)))
		goto error;

	{
		VkSamplerCreateInfo sampler_info = {0};

		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.pNext = NULL;
		sampler_info.flags = 0;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.mipLodBias = 0.0F;
		sampler_info.anisotropyEnable = VK_FALSE;
		sampler_info.maxAnisotropy = 1.0F;
		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_info.minLod = 0.0F;
		sampler_info.maxLod = VK_LOD_CLAMP_NONE;
		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.unnormalizedCoordinates = VK_FALSE;

		if (!SF_VULKAN_CHECK(vkCreateSampler(device->vk.device, &sampler_info, device->vk.allocation_callbacks, &device->vk.linear_sampler))) {
			device->vk.linear_sampler = VK_NULL_HANDLE;
			goto error;
		}
	}

	{
		VkDescriptorSetLayoutCreateInfo info = {0};
		VkDescriptorSetLayoutBinding bindings[1] = {0};

		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;

		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[0].pImmutableSamplers = NULL;

		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.bindingCount = SF_SIZE(bindings);
		info.pBindings = bindings;

		if (!SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(device->vk.device, &info, device->vk.allocation_callbacks, &device->vk.image_descriptor_set_layout))) {
			device->vk.image_descriptor_set_layout = VK_NULL_HANDLE;
			goto error;
		}
	}

	{
		VkDescriptorSetLayoutCreateInfo info = {0};
		VkDescriptorSetLayoutBinding bindings[1] = {0};

		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].pImmutableSamplers = NULL;

		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.bindingCount = SF_SIZE(bindings);
		info.pBindings = bindings;

		if (!SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(device->vk.device, &info, device->vk.allocation_callbacks, &device->vk.uniform_descriptor_set_layout))) {
			device->vk.uniform_descriptor_set_layout = VK_NULL_HANDLE;
			goto error;
		}
	}

	{
		VkDescriptorSetLayoutCreateInfo info = {0};
		VkDescriptorSetLayoutBinding bindings[1] = {0};

		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].pImmutableSamplers = NULL;

		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.bindingCount = SF_SIZE(bindings);
		info.pBindings = bindings;

		if (!SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(device->vk.device, &info, device->vk.allocation_callbacks, &device->vk.dynamic_uniform_descriptor_set_layout))) {
			device->vk.dynamic_uniform_descriptor_set_layout = VK_NULL_HANDLE;
			goto error;
		}
	}
	
	{
		VkPipelineLayoutCreateInfo info = {0};
		VkDescriptorSetLayout layouts[4] = {0};

		layouts[0] = device->vk.dynamic_uniform_descriptor_set_layout;
		layouts[1] = device->vk.uniform_descriptor_set_layout;
		layouts[2] = device->vk.image_descriptor_set_layout;
		layouts[3] = device->vk.image_descriptor_set_layout;

		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.setLayoutCount = SF_SIZE(layouts);
		info.pSetLayouts = layouts;
		info.pushConstantRangeCount = 0;
		info.pPushConstantRanges = NULL;

		if (!SF_VULKAN_CHECK(vkCreatePipelineLayout(device->vk.device, &info, device->vk.allocation_callbacks, &device->vk.general_pipeline_layout))) {
			device->vk.general_pipeline_layout = VK_NULL_HANDLE;
			goto error;
		}
	}

	{
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

		if (!SF_VULKAN_CHECK(vkCreateDescriptorPool(device->vk.device, &info, device->vk.allocation_callbacks, &device->vk.global_descriptor_pool))) {
			device->vk.global_descriptor_pool = VK_NULL_HANDLE;
			goto error;
		}
	}

	return device;

error:
	sf_graphics_deinit_device(device);
	return NULL;
}

struct sf_graphics_vulkan_surface_format_array {
	u32 size;
	VkSurfaceFormatKHR *data;
};

sf_private void sf_graphics_vulkan_load_surface_formats(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, struct sf_graphics_vulkan_surface_format_array *out) {
	u32 count = 0;

	if (!arena || !device || !surface || !out)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL)))
		return;

	out->data = sf_arena_allocate(arena, count * sizeof(*out->data));
	if (!out->data)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, out->data)))
		return;

	out->size = count;
}

struct sf_graphics_vulkan_present_mode_array {
	u32 size;
	VkPresentModeKHR *data;
};

sf_private void sf_graphics_vulkan_load_present_modes(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, struct sf_graphics_vulkan_present_mode_array *out) {
	u32 count = 0;

	if (!arena || !device || !surface || !out)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL)))
		return;

	out->data = sf_arena_allocate(arena, count * sizeof(*out->data));
	if (!out->data)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, out->data)))
		return;

	out->size = count;
}

sf_private VkPresentModeKHR sf_graphics_vulkan_find_swapchain_present_mode(struct sf_arena *arena, VkSurfaceKHR vk_surface, VkPhysicalDevice vk_physical_device, struct sf_graphics_init_swapchain_info *init_info) {
	u32 i = 0;
	struct sf_graphics_vulkan_present_mode_array present_modes = {0};
	VkPresentModeKHR requested_present_mode = VK_PRESENT_MODE_FIFO_KHR;

	if (!arena || !vk_surface || !vk_physical_device || !init_info)
		return VK_PRESENT_MODE_FIFO_KHR;

	requested_present_mode = init_info->requested_enable_vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

	sf_graphics_vulkan_load_present_modes(arena, vk_physical_device, vk_surface, &present_modes);
	if (!present_modes.data || !present_modes.size)
		return VK_PRESENT_MODE_FIFO_KHR;

	for (i = 0; i < present_modes.size; ++i)
		if (requested_present_mode == present_modes.data[i])
			return requested_present_mode;

	return VK_PRESENT_MODE_FIFO_KHR; // NOTE(samuel): FIFO is guaranteed to be available on all platforms.
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

sf_private void sf_graphics_vulkan_find_supported_color_format(struct sf_arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR *out) {
	u32 i = 0;
	VkSurfaceFormatKHR requested_format = {0};
	VkSurfaceFormatKHR default_format = {0};
	struct sf_graphics_vulkan_surface_format_array candidates = {0};

	requested_format.format = VK_FORMAT_B8G8R8A8_SRGB;
	requested_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	default_format.format = VK_FORMAT_UNDEFINED;
	default_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	*out = default_format;

	if (!arena || !device || !surface || !out)
		return;

	sf_graphics_vulkan_load_surface_formats(arena, device, surface, &candidates);
	if (!candidates.size || !candidates.data)
		return;

	for (i = 0; i < candidates.size; ++i) {
		VkSurfaceFormatKHR *current = &candidates.data[i];

		if (current->format == requested_format.format && current->colorSpace == requested_format.colorSpace) {
			*out = *current;
			return;
		}
	}
}


sf_private VkFormat sf_graphics_vulkan_find_swapchain_depth_stencil_format(struct sf_graphics_device *device) {
	VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

	if (!device || !device->vk.physical_device)
		return VK_FORMAT_UNDEFINED;


	return sf_graphics_vulkan_find_format(device->vk.physical_device, SF_SIZE(candidates), candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}


sf_private enum sf_graphics_format sf_graphics_get_format_from_vulkan_format(VkFormat format) {
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

sf_private VkFormat sf_graphics_vulkan_get_format_from_format(enum sf_graphics_format format) {
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
		case SF_GRAPHICS_FORMAT_R8G8B8A8_SRGB:
			result = VK_FORMAT_R8G8B8A8_SRGB;
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

sf_private void sf_graphics_update_swapchain_dimensions(struct sf_graphics_device *device, struct sf_graphics_swapchain *swapchain) {
	if (!device || !device->vk.physical_device || !device->vk.surface || !swapchain || !device->info.request_surface_dimensions)
		return;

	swapchain->width = 0;
	swapchain->height = 0;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->vk.physical_device, device->vk.surface, &device->vk.physical_device_surface_capabilities)))
		return

	device->info.request_surface_dimensions(device->info.plataform_data,  &swapchain->width, &swapchain->height);

	swapchain->width = SF_CLAMP(swapchain->width, device->vk.physical_device_surface_capabilities.minImageExtent.width, device->vk.physical_device_surface_capabilities.maxImageExtent.width);
	swapchain->height = SF_CLAMP(swapchain->height, device->vk.physical_device_surface_capabilities.minImageExtent.height, device->vk.physical_device_surface_capabilities.maxImageExtent.height);
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

sf_private VkMemoryPropertyFlags sf_graphics_vulkan_get_memory_property_flags_from_memory_property_flags(sf_graphics_memory_property_flags flags) {
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

sf_private VkDeviceMemory sf_graphics_vulkan_allocate_memory(struct sf_graphics_device *device, sf_graphics_memory_property_flags memory_properties, u32 filter, u64 size) {
	u32 memory_type_index = (u32)-1;
	VkDeviceMemory memory = VK_NULL_HANDLE;

	if (!device || !device->vk.physical_device)
		return VK_NULL_HANDLE;

	memory_type_index = sf_graphics_vulkan_find_memory_type_index(device->vk.physical_device, sf_graphics_vulkan_get_memory_property_flags_from_memory_property_flags(memory_properties), filter);
	if (memory_type_index == (u32)-1)
		return VK_NULL_HANDLE;

	{
		VkMemoryAllocateInfo info = {0};

		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.pNext = NULL;
		info.allocationSize = size;
		info.memoryTypeIndex = memory_type_index;

		if (SF_VULKAN_CHECK(vkAllocateMemory(device->vk.device, &info, device->vk.allocation_callbacks, &memory)))
			return memory;
	}

	return VK_NULL_HANDLE;
}

sf_private VkDeviceMemory sf_graphics_vulkan_allocate_and_bind_memory_for_image(struct sf_graphics_device *device, VkImage image, sf_graphics_memory_property_flags memory_properties) {
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkMemoryRequirements requirements = {0};

	if (!device || !device->vk.physical_device || !device->vk.device || !image)
		return VK_NULL_HANDLE;

	vkGetImageMemoryRequirements(device->vk.device, image, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(device, memory_properties, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindImageMemory(device->vk.device, image, memory, 0))) {
		vkFreeMemory(device->vk.device, memory, device->vk.allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}

sf_private VkDeviceMemory sf_graphics_vulkan_allocate_memory_for_buffer(struct sf_graphics_device *device, VkBuffer buffer, sf_graphics_memory_property_flags memory_properties) {
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkMemoryRequirements requirements = {0};

	if (!device || !device->vk.physical_device || !device->vk.device || !buffer)
		return VK_NULL_HANDLE;

	vkGetBufferMemoryRequirements(device->vk.device, buffer, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(device, memory_properties, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindBufferMemory(device->vk.device, buffer, memory, 0))) {
		vkFreeMemory(device->vk.device, memory, device->vk.allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}


sf_public void sf_graphics_device_deinit_image(struct sf_graphics_device *device, struct sf_graphics_image *image) {
	if (device && image) {
		if (device->vk.device) {
			if (device->vk.global_descriptor_pool) {
				if (image->vk.descriptor_set) {
					vkFreeDescriptorSets(device->vk.device, device->vk.global_descriptor_pool, 1, &image->vk.descriptor_set);
					image->vk.descriptor_set = VK_NULL_HANDLE;
				}
			}

			if (image->vk.image_view) {
				vkDestroyImageView(device->vk.device, image->vk.image_view, device->vk.allocation_callbacks);
				image->vk.image_view = VK_NULL_HANDLE;
			}

			if (image->vk.image != image->info.vk.not_owned_image) {
				if (image->vk.memory) {
					vkFreeMemory(device->vk.device, image->vk.memory, device->vk.allocation_callbacks);
					image->vk.memory = VK_NULL_HANDLE;
				}

				if (image->vk.image) {
					vkDestroyImage(device->vk.device, image->vk.image, device->vk.allocation_callbacks);
					image->vk.image = VK_NULL_HANDLE;
				}
			}
			
		}
	}
}


sf_private u32 sf_graphics_calculate_mips(u32 width, u32 height) {
	u32 mips = 0;
	u32 max_dim = SF_MAX(width, height);

	while (max_dim > 0) {
		max_dim /= 2;
		mips++;
	}

	return mips;
}


sf_private VkImageType sf_graphics_vulkan_get_image_type_from_image_type(enum sf_graphics_image_type type) {
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


sf_private VkSampleCountFlags sf_graphics_vulkan_get_sample_count_from_sample_count(enum sf_graphics_sample_count samples) {
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


sf_private VkImageUsageFlags sf_graphics_vulkan_get_image_usage_from_image_usage(sf_graphics_image_usage_flags usage) {
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


sf_private VkImageViewType sf_graphics_vulkan_get_image_view_type_from_image_type(enum sf_graphics_image_type type) {
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


sf_private VkImageAspectFlags sf_graphics_vulkan_get_image_aspect_flags_from_format(enum sf_graphics_format format) {
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
			// FIXME(samuel): currently depth stencil aspectMasks are not handled correctly on image creation
			result = VK_IMAGE_ASPECT_DEPTH_BIT; // | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		default:
			result = 0;
			break;
	}

	return result;
}

sf_public struct sf_graphics_image *sf_graphics_device_init_image(struct sf_arena *arena, struct sf_graphics_device *device, struct sf_graphics_init_image_info const *info) {
	struct sf_graphics_image *image = NULL;

	if (!device || !device->vk.device || !info)
		return NULL;

	image = sf_arena_allocate(arena, sizeof(*image));
	if (!image)
		goto error;



	image->info = *info;

	if (image->info.mips == SF_GRAPHICS_CALCULATE_MIPS)
		image->info.mips = sf_graphics_calculate_mips(info->width, info->height);

	if (info->vk.not_owned_image) {
		image->vk.image = info->vk.not_owned_image;
	} else {
		VkImageCreateInfo image_info = {0};

		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = NULL;
		image_info.flags = 0;
		image_info.imageType = sf_graphics_vulkan_get_image_type_from_image_type(info->type);
		image_info.format = sf_graphics_vulkan_get_format_from_format(info->format);
		image_info.extent.width = info->width;
		image_info.extent.height = info->height;
		image_info.extent.depth = 1;
		image_info.mipLevels = image->info.mips;
		image_info.arrayLayers = 1;
		image_info.samples = sf_graphics_vulkan_get_sample_count_from_sample_count(info->samples);
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = sf_graphics_vulkan_get_image_usage_from_image_usage(info->usage);
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.queueFamilyIndexCount = 0;
		image_info.pQueueFamilyIndices = NULL;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (!SF_VULKAN_CHECK(vkCreateImage(device->vk.device, &image_info, device->vk.allocation_callbacks, &image->vk.image))) {
			image->vk.image = VK_NULL_HANDLE;
			goto error;
		}

		image->vk.memory = sf_graphics_vulkan_allocate_and_bind_memory_for_image(device, image->vk.image, info->memory_properties);
		if (!image->vk.memory)
			goto error;
	}

	{
		VkImageViewCreateInfo image_view_info = {0};

		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.pNext = NULL;
		image_view_info.flags = 0;
		image_view_info.image = image->vk.image;
		image_view_info.viewType = sf_graphics_vulkan_get_image_view_type_from_image_type(info->type);
		image_view_info.format = sf_graphics_vulkan_get_format_from_format(info->format);
		image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
		image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
		image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
		image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
		image_view_info.subresourceRange.aspectMask = sf_graphics_vulkan_get_image_aspect_flags_from_format(info->format); // FIXME(samuel): depth/stencil cases fail validation. Need to create a new image view for that
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = info->mips;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;

		if (!SF_VULKAN_CHECK(vkCreateImageView(device->vk.device, &image_view_info, device->vk.allocation_callbacks, &image->vk.image_view))) {
			image->vk.image_view = VK_NULL_HANDLE;
			goto error;
		}
	}

	if (info->usage & SF_GRAPHICS_IMAGE_USAGE_SAMPLED) {
		{
			VkDescriptorSetAllocateInfo descriptor_set_info = {0};

			descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptor_set_info.pNext = NULL;
			descriptor_set_info.descriptorPool = device->vk.global_descriptor_pool;
			descriptor_set_info.descriptorSetCount = 1;
			descriptor_set_info.pSetLayouts = &device->vk.image_descriptor_set_layout;

			if (!SF_VULKAN_CHECK(vkAllocateDescriptorSets(device->vk.device, &descriptor_set_info, &image->vk.descriptor_set))) {
				image->vk.descriptor_set = VK_NULL_HANDLE;
				goto error;
			}
		}

		{
			VkWriteDescriptorSet write = {0};
			VkDescriptorImageInfo image_info = {0};

			image_info.sampler = device->vk.linear_sampler; // TODO(samuel): For now default to this, later build a sampler cache
			image_info.imageView = image->vk.image_view;
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = NULL;
			write.dstSet = image->vk.descriptor_set;
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &image_info;
			write.pBufferInfo = NULL;
			write.pTexelBufferView = NULL;

			vkUpdateDescriptorSets(device->vk.device, 1, &write, 0, NULL);
		}
	}

	return image;
error:
	sf_graphics_device_deinit_image(device, image);
	return NULL;
}

sf_private void sf_graphics_device_deinit_semaphore(struct sf_graphics_device *device, struct sf_graphics_semaphore *semaphore) {
	if (device && semaphore) {
		if (device->vk.device) {
			if (semaphore->vk.semaphore) {
				vkDestroySemaphore(device->vk.device, semaphore->vk.semaphore, device->vk.allocation_callbacks);
				semaphore->vk.semaphore = VK_NULL_HANDLE;
			}
		}
	}
}


sf_private struct sf_graphics_semaphore *sf_graphics_device_init_semaphore(struct sf_arena *arena, struct sf_graphics_device *device) {
	struct sf_graphics_semaphore *semaphore = NULL;

	if (!device || !device->vk.device)
		return NULL;

	semaphore = sf_arena_allocate(arena, sizeof(*semaphore));
	if (!semaphore)
		return NULL;

	{
		VkSemaphoreCreateInfo semaphore_info = {0};

		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_info.pNext = NULL;
		semaphore_info.flags = 0;

		if (!SF_VULKAN_CHECK(vkCreateSemaphore(device->vk.device, &semaphore_info, device->vk.allocation_callbacks, &semaphore->vk.semaphore))) {
			semaphore->vk.semaphore = VK_NULL_HANDLE;
			goto error;
		}
	}
	
	return semaphore;


error:
	sf_graphics_device_deinit_semaphore(device, semaphore);
	return NULL;
}

sf_private void sf_graphics_device_deinit_fence(struct sf_graphics_device *device, struct sf_graphics_fence *fence) {
	if (device && fence) {
		if (device->vk.device) {
			if (fence->vk.fence) {
				vkDestroyFence(device->vk.device, fence->vk.fence, device->vk.allocation_callbacks);
				fence->vk.fence = VK_NULL_HANDLE;
			}
		}
	}
}

sf_private struct sf_graphics_fence *sf_graphics_device_init_fence(struct sf_arena *arena, struct sf_graphics_device *device, sf_bool signaled) {
	struct sf_graphics_fence *fence = NULL;

	if (!device || !device->vk.device)
		return NULL;

	fence = sf_arena_allocate(arena, sizeof(*fence));
	if (!fence)
		return NULL;

	{
		VkFenceCreateInfo fence_info = {0};

		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.pNext = NULL;
		fence_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		if (!SF_VULKAN_CHECK(vkCreateFence(device->vk.device, &fence_info, device->vk.allocation_callbacks, &fence->vk.fence))) {
			fence->vk.fence = VK_NULL_HANDLE;
			goto error;
		}
	}

	return fence;

error:
	sf_graphics_device_deinit_fence(device, fence);
	return NULL;
}

struct sf_graphics_vulkan_swapchain_image_array {
	u32 size;
	VkImage data[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
};

sf_private void sf_graphics_vulkan_load_swapchain_image_array(VkDevice vk_device, VkSwapchainKHR vk_swapchain, struct sf_graphics_vulkan_swapchain_image_array *out) {
	u32 count = 0;

	if (!vk_device || !vk_swapchain || !out)
		return;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &count, NULL)))
		return;
	if (count > SF_SIZE(out->data))
		return;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &count, out->data)))
		return;

	out->size = count;
}



sf_private VkBufferUsageFlags sf_graphics_vulkan_get_buffer_usage_flags_from_buffer_usage_flags(sf_graphics_buffer_usage_flags flags) {
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

sf_private void sf_graphics_device_deinit_buffer(struct sf_graphics_device *device, struct sf_graphics_buffer *buffer) {
	if (device && buffer) {
		buffer->cpu_mapped_data = NULL;

		if (device->vk.device) {
			if (device->vk.global_descriptor_pool) {
				if (buffer->vk.descriptor_set) {
					vkFreeDescriptorSets(device->vk.device, device->vk.global_descriptor_pool, 1, &buffer->vk.descriptor_set);
					buffer->vk.descriptor_set = VK_NULL_HANDLE;
				}
			}

			if (buffer->vk.memory) {
				vkFreeMemory(device->vk.device, buffer->vk.memory, device->vk.allocation_callbacks);
				buffer->vk.memory = VK_NULL_HANDLE;
			}

			if (buffer->vk.buffer) {
				vkDestroyBuffer(device->vk.device, buffer->vk.buffer, device->vk.allocation_callbacks);
				buffer->vk.buffer = VK_NULL_HANDLE;
			}
		}
	}
}

sf_private struct sf_graphics_buffer *sf_graphics_device_init_buffer(struct sf_arena *arena, struct sf_graphics_device *device, struct sf_graphics_init_buffer_info const *info) {
	struct sf_graphics_buffer *buffer = NULL;
	
	if (!device || !device->vk.device || !info)
		return NULL;

	buffer = sf_arena_allocate(arena, sizeof(*buffer));
	if (!buffer)
		return NULL;

	buffer->info = *info;
	buffer->is_dynamic = (info->memory_properties & SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE);

	{
		VkBufferCreateInfo buffer_info = {0};

		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.pNext = NULL;
		buffer_info.flags = 0;
		buffer_info.size = info->size;
		buffer_info.usage = sf_graphics_vulkan_get_buffer_usage_flags_from_buffer_usage_flags(info->usage);
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		buffer_info.queueFamilyIndexCount = 0;
		buffer_info.pQueueFamilyIndices = NULL;

		if (!SF_VULKAN_CHECK(vkCreateBuffer(device->vk.device, &buffer_info, device->vk.allocation_callbacks, &buffer->vk.buffer))) {
			buffer->vk.buffer = VK_NULL_HANDLE;
			goto error;
		}
	}

	buffer->vk.memory = sf_graphics_vulkan_allocate_memory_for_buffer(device, buffer->vk.buffer, info->memory_properties);
	if (!buffer->vk.memory)
		goto error;

	if ((info->memory_properties & SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE)) {
		if (!SF_VULKAN_CHECK(vkMapMemory(device->vk.device, buffer->vk.memory, 0, VK_WHOLE_SIZE, 0, &buffer->cpu_mapped_data)))
			goto error;
	}


	if (info->usage & SF_GRAPHICS_BUFFER_USAGE_UNIFORM_BUFFER) {
		sf_bool is_dynamic = !!(info->memory_properties & SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE);

		{
			VkDescriptorSetAllocateInfo descriptor_set_info = {0};

			descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptor_set_info.pNext = NULL;
			descriptor_set_info.descriptorPool = device->vk.global_descriptor_pool;
			descriptor_set_info.descriptorSetCount = 1;
			descriptor_set_info.pSetLayouts = is_dynamic ? &device->vk.dynamic_uniform_descriptor_set_layout : &device->vk.uniform_descriptor_set_layout;

			if (!SF_VULKAN_CHECK(vkAllocateDescriptorSets(device->vk.device, &descriptor_set_info, &buffer->vk.descriptor_set))) {
				buffer->vk.descriptor_set = VK_NULL_HANDLE;
				goto error;
			}
		}

		{
			VkWriteDescriptorSet write = {0};
			VkDescriptorBufferInfo buffer_info = {0};

			buffer_info.buffer = buffer->vk.buffer;
			buffer_info.offset = 0;
			buffer_info.range = VK_WHOLE_SIZE;

			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = NULL;
			write.dstSet = buffer->vk.descriptor_set;
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = is_dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.pImageInfo = NULL;
			write.pBufferInfo = &buffer_info;
			write.pTexelBufferView = NULL;

			vkUpdateDescriptorSets(device->vk.device, 1, &write, 0, NULL);
		}
	}

	return buffer;

error:
	sf_graphics_device_deinit_buffer(device, buffer);
	return NULL;
}

sf_public struct sf_graphics_buffer *sf_graphics_device_init_buffer_for_staging(struct sf_arena *arena, struct sf_graphics_device *device, u64 data_size_in_bytes, void const *data) {
	struct sf_graphics_init_buffer_info info = {0};
	struct sf_graphics_buffer *buffer = NULL;

	if (!arena || !device || !data_size_in_bytes || !data)
		return NULL;

	info.size = data_size_in_bytes;
	info.usage = 0;
	info.usage |= SF_GRAPHICS_BUFFER_USAGE_TRANSFER_SOURCE;
	info.usage |= SF_GRAPHICS_BUFFER_USAGE_TRANSFER_DESTINATION;
	info.memory_properties = SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE;

	buffer = sf_graphics_device_init_buffer(arena, device, &info);
	if (!buffer)
		return NULL;

	if (data) {
		void *mapped_data = buffer->cpu_mapped_data;

		if (!mapped_data)
			goto error;

		SF_MEMORY_COPY(mapped_data, data, data_size_in_bytes);
	}

	return buffer;

error:
	sf_graphics_device_deinit_buffer(device, buffer);
	return NULL;
}

sf_private VkCommandBufferUsageFlags sf_graphics_vulkan_get_command_buffer_usage_flags_from_command_buffer_flags(sf_graphics_command_buffer_usage_flags flags) {
	VkCommandBufferUsageFlags result = 0;

	if (flags & SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT)
		result = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	return result;
}

sf_private VkCommandPoolCreateFlags sf_graphics_vulkan_get_command_pool_create_flags_from_command_buffer_flags(sf_graphics_command_buffer_usage_flags flags) {
	VkCommandPoolCreateFlags result = 0;

	if (flags & SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT)
		result = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	// NOTE(samuel): all command buffers can be reset
	result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	return result;
}


sf_private void sf_graphics_device_deinit_command_buffer(struct sf_graphics_device *device, struct sf_graphics_command_buffer *command_buffer) {
	if (device && command_buffer) {
		if (device->vk.device) {
			if (command_buffer->vk.command_pool) {
				if (command_buffer->vk.command_buffer) {
					vkFreeCommandBuffers(device->vk.device, command_buffer->vk.command_pool, 1, &command_buffer->vk.command_buffer);
					command_buffer->vk.command_buffer = VK_NULL_HANDLE;
				}

				vkDestroyCommandPool(device->vk.device, command_buffer->vk.command_pool, device->vk.allocation_callbacks);
				command_buffer->vk.command_pool = VK_NULL_HANDLE;
			}
		}
	}
}

sf_private struct sf_graphics_command_buffer *sf_graphics_device_init_command_buffer(struct sf_arena *arena, struct sf_graphics_device *device, sf_graphics_command_buffer_usage_flags usage) {
	struct sf_graphics_command_buffer *command_buffer = NULL;

	if (!device || !device->vk.device)
		return NULL;

	command_buffer = sf_arena_allocate(arena, sizeof(*command_buffer));
	if (!command_buffer)
		return NULL;

	command_buffer->is_executable = SF_FALSE;
	command_buffer->is_recording = SF_FALSE;
	command_buffer->usage = usage;
	
	{
		VkCommandPoolCreateInfo info = {0};

		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = NULL;
		info.flags = sf_graphics_vulkan_get_command_pool_create_flags_from_command_buffer_flags(usage);
		info.queueFamilyIndex = device->vk.graphics_queue_family_index;

		if (!SF_VULKAN_CHECK(vkCreateCommandPool(device->vk.device, &info, device->vk.allocation_callbacks, &command_buffer->vk.command_pool))) {
			command_buffer->vk.command_pool = VK_NULL_HANDLE;
			goto error;
		}
	}
	{
		VkCommandBufferAllocateInfo info = {0};

		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = NULL;
		info.commandPool = command_buffer->vk.command_pool;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount = 1;

		if (!SF_VULKAN_CHECK(vkAllocateCommandBuffers(device->vk.device, &info, &command_buffer->vk.command_buffer))) {
			command_buffer->vk.command_buffer = VK_NULL_HANDLE;
			goto error;
		}
	}

	return command_buffer;

error:
	sf_graphics_device_deinit_command_buffer(device, command_buffer);
	return NULL;
}

sf_private void sf_graphics_reset_command_buffer(struct sf_graphics_device *device, struct sf_graphics_command_buffer *command_buffer) {
	if (!device || !device->vk.device || !command_buffer || !command_buffer->vk.command_buffer)
		return;

	if (SF_VULKAN_CHECK(vkResetCommandPool(device->vk.device, command_buffer->vk.command_pool, 0))) {
		command_buffer->is_recording = SF_FALSE;
		command_buffer->is_executable = SF_FALSE;
	}
}

sf_private void sf_graphics_begin_command_buffer(struct sf_graphics_command_buffer *command_buffer) {
	if (!command_buffer || !command_buffer->vk.command_buffer || command_buffer->is_executable || command_buffer->is_recording)
		return;

	{
		VkCommandBufferBeginInfo info = {0};

		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = NULL;
		if (command_buffer->usage & SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT)
			info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		else
			info.flags = 0;
		info.pInheritanceInfo = NULL;

		command_buffer->is_recording = SF_VULKAN_CHECK(vkBeginCommandBuffer(command_buffer->vk.command_buffer, &info));
	}
}

sf_private void sf_graphics_end_command_buffer(struct sf_graphics_command_buffer *command_buffer) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !command_buffer->is_recording)
		return;
	
	// https://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html#commandbuffers-lifecycle
	if (command_buffer->is_recording) {
		if (SF_VULKAN_CHECK(vkEndCommandBuffer(command_buffer->vk.command_buffer))) {
			command_buffer->is_recording = SF_FALSE;
			command_buffer->is_executable = SF_TRUE;
		}
	}
}

sf_private void sf_graphics_device_submit_and_block_command_buffer(struct sf_graphics_device *device, struct sf_graphics_command_buffer *command_buffer) {
	if (!device || !device->vk.device || !command_buffer || !command_buffer->is_executable)
		return;

	{
		VkSubmitInfo info = {0};

		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = NULL;
		info.waitSemaphoreCount = 0;
		info.pWaitSemaphores = NULL;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &command_buffer->vk.command_buffer;
		info.signalSemaphoreCount = 0;
		info.pSignalSemaphores = NULL;

		SF_VULKAN_CHECK(vkQueueSubmit(device->vk.graphics_queue, 1, &info, VK_NULL_HANDLE));
		SF_VULKAN_CHECK(vkDeviceWaitIdle(device->vk.device));

		
	}
}

sf_private void sf_graphics_vulkan_get_clear_value_from_clear_value(struct sf_graphics_clear_value *clear_value, VkClearValue *out) {
	if (clear_value->type == SF_GRAPHICS_CLEAR_VALUE_TYPE_COLOR) {
		out->color.float32[0] = clear_value->data.rgba.r;
		out->color.float32[1] = clear_value->data.rgba.g;
		out->color.float32[2] = clear_value->data.rgba.b;
		out->color.float32[3] = clear_value->data.rgba.a;
	} else if (clear_value->type == SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH_STENCIL) {
		out->depthStencil.depth = clear_value->data.depth_stencil.depth;
		out->depthStencil.stencil = clear_value->data.depth_stencil.stencil;
	}
}

sf_public void sf_graphics_command_begin_render_pass(struct sf_graphics_device *device, struct sf_graphics_command_buffer *command_buffer, struct sf_graphics_render_target *render_target) {
	VkClearValue clear_values[2] = {0};

	if (!device || !device->vk.device || !command_buffer || !command_buffer->vk.command_buffer || !render_target || !render_target->vk.framebuffer || !render_target->vk.render_pass)
		return;

	sf_graphics_vulkan_get_clear_value_from_clear_value(&render_target->info.color_clear_value, &clear_values[0]);
	sf_graphics_vulkan_get_clear_value_from_clear_value(&render_target->info.depth_stencil_clear_value, &clear_values[1]);

	{
		VkRenderPassBeginInfo info = {0};
#if 0
		clear_values[0].color.float32[0] = 0.0F;
		clear_values[0].color.float32[1] = 0.0F;
		clear_values[0].color.float32[2] = 0.0F;
		clear_values[0].color.float32[3] = 1.0F;
		clear_values[1].depthStencil.depth = 1.0F;
		clear_values[1].depthStencil.stencil = 0;
#endif

		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = NULL;
		info.renderPass = render_target->vk.render_pass;
		info.framebuffer = render_target->vk.framebuffer;
		info.renderArea.offset.x = 0;
		info.renderArea.offset.y = 0;
		info.renderArea.extent.width = render_target->info.width;
		info.renderArea.extent.height = render_target->info.height;
		info.clearValueCount = SF_SIZE(clear_values);
		info.pClearValues = clear_values;

		vkCmdBeginRenderPass(command_buffer->vk.command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	{
		VkViewport viewport = {0};

		viewport.x = 0.0F;
		viewport.y = 0.0F;
		viewport.width = (float)render_target->info.width;
		viewport.height = (float)render_target->info.height;
		viewport.minDepth = 0.0F;
		viewport.maxDepth = 1.0F;

		vkCmdSetViewport(command_buffer->vk.command_buffer, 0, 1, &viewport);
	}

	{
		VkRect2D scissor = {0};

		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = render_target->info.width;
		scissor.extent.height = render_target->info.height;

		vkCmdSetScissor(command_buffer->vk.command_buffer, 0, 1, &scissor);
	}
}

sf_public void sf_graphics_command_end_render_pass(struct sf_graphics_command_buffer *command_buffer) {
	if (!command_buffer || !command_buffer->vk.command_buffer)	
		return;

	vkCmdEndRenderPass(command_buffer->vk.command_buffer);
}



sf_public struct sf_graphics_command_buffer *sf_graphics_device_init_and_begin_single_use_command_buffer(struct sf_arena *arena, struct sf_graphics_device *device) {
	struct sf_graphics_command_buffer *command_buffer = NULL;

	if (!device)
		return NULL;

	command_buffer = sf_graphics_device_init_command_buffer(arena, device, SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT);
	if (!command_buffer)
		return NULL;

	sf_graphics_begin_command_buffer(command_buffer);
	if (!command_buffer->is_recording)
		goto error;

	return command_buffer;

error:
	sf_graphics_device_deinit_command_buffer(device, command_buffer);
	return NULL;
}

sf_public void sf_graphics_device_end_submit_and_deinit_command_buffer(struct sf_graphics_device *device, struct sf_graphics_command_buffer *command_buffer) {
	sf_graphics_end_command_buffer(command_buffer);
	sf_graphics_device_submit_and_block_command_buffer(device, command_buffer);
	sf_graphics_device_deinit_command_buffer(device, command_buffer);
}

sf_private void sf_graphics_command_copy_buffer_to_buffer(struct sf_graphics_command_buffer *command_buffer, u64 source_offset, u64 destination_offset, u64 size, struct sf_graphics_buffer *source_buffer, struct sf_graphics_buffer *destination_buffer) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !source_buffer || !source_buffer->vk.buffer || !destination_buffer || !destination_buffer->vk.buffer)
		return;

	{
		VkBufferCopy copy = {0};

		copy.srcOffset = source_offset;
		copy.dstOffset = destination_offset;
		copy.size = size;

		vkCmdCopyBuffer(command_buffer->vk.command_buffer, source_buffer->vk.buffer, destination_buffer->vk.buffer, 1, &copy);
	}
}

sf_private void sf_graphics_command_copy_buffer_to_image(struct sf_graphics_command_buffer *command_buffer, u64 buffer_offset, u32 width, u32 height, u32 depth, u32 mips, struct sf_graphics_buffer *source_buffer, struct sf_graphics_image *destination_image) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !source_buffer || !source_buffer->vk.buffer || !destination_image || !destination_image->vk.image)
		return;

	{
		VkBufferImageCopy copy = {0};

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

		vkCmdCopyBufferToImage(command_buffer->vk.command_buffer, source_buffer->vk.buffer, destination_image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
	}
}

sf_private void sf_graphics_device_wait_idle(struct sf_graphics_device *device) {
	if (!device || !device->vk.device)
		return;

	SF_VULKAN_CHECK(vkDeviceWaitIdle(device->vk.device));
}

sf_private u64 sf_graphics_get_stride_from_format(enum sf_graphics_format format) {
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

sf_private struct sf_graphics_buffer *sf_graphics_device_init_buffer_and_upload_data(struct sf_arena *arena, struct sf_graphics_device *device, sf_graphics_buffer_usage_flags usage, sf_graphics_memory_property_flags memory_properties, u64 data_size_in_bytes, void const *data) {
	struct sf_graphics_buffer *result_buffer = NULL;
	struct sf_graphics_buffer *staging_buffer = NULL;
	struct sf_graphics_command_buffer *command_buffer = NULL;

	if (!device || !data_size_in_bytes || !data)
		return NULL;

	staging_buffer = sf_graphics_device_init_buffer_for_staging(arena, device, data_size_in_bytes, data);
	if (!staging_buffer)
		goto error;

	{
		struct sf_graphics_init_buffer_info info = {0};

		info.size = data_size_in_bytes;
		info.usage = usage;
		info.usage |= SF_GRAPHICS_BUFFER_USAGE_TRANSFER_DESTINATION;
		info.memory_properties = memory_properties;


		result_buffer = sf_graphics_device_init_buffer(arena, device, &info);
		if (!result_buffer)
			goto error;
	}

	command_buffer = sf_graphics_device_init_and_begin_single_use_command_buffer(arena, device);
	if (!command_buffer)
		goto error;

	sf_graphics_command_copy_buffer_to_buffer(command_buffer, 0, 0, data_size_in_bytes, staging_buffer, result_buffer);

	goto cleanup;

error:
	sf_graphics_device_wait_idle(device);
	sf_graphics_device_deinit_buffer(device, result_buffer);
	result_buffer = NULL;

cleanup:
	sf_graphics_device_end_submit_and_deinit_command_buffer(device, command_buffer);
	sf_graphics_device_deinit_buffer(device, staging_buffer);

	return result_buffer;
}

#if 0 
sf_private enum sf_graphics_format sf_graphics_get_format_from_vulkan_format(VkFormat format) {
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
#endif

sf_private enum sf_graphics_sample_count sf_graphics_get_sample_count_from_vulkan_sample_count(VkSampleCountFlags samples) {
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

sf_private void sf_graphics_vulkan_command_transition_image_layout(struct sf_graphics_command_buffer *command_buffer, struct sf_graphics_image *image, u32 mip_levels, VkImageLayout old_layout, VkImageLayout new_layout) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !image || !image->vk.image)
		return;

	{
		VkImageMemoryBarrier barrier = {0};
		VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_NONE;
		VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_NONE;

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = NULL;
		barrier.srcAccessMask = VK_ACCESS_NONE;
		barrier.dstAccessMask = VK_ACCESS_NONE;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image->vk.image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mip_levels;
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

		vkCmdPipelineBarrier(command_buffer->vk.command_buffer, source_stage, destination_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
	}
}

sf_private sf_bool sf_graphics_check_mip_map_support(struct sf_graphics_device *device, enum sf_graphics_format format) {
	VkFormatProperties properties = {0};

	if (!device || !device->vk.physical_device)
		return SF_FALSE;

	vkGetPhysicalDeviceFormatProperties(device->vk.physical_device, sf_graphics_vulkan_get_format_from_format(format), &properties);

	return !(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
}

sf_private void sf_graphics_command_generate_image_mip_maps(struct sf_graphics_command_buffer *command_buffer, struct sf_graphics_image *image) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !image || !image->vk.image)
		return;

	{
		VkImageMemoryBarrier barrier = {0};
		i32 current_width = 0;
		i32 current_height = 0;
		u32 i = 0;

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = NULL;
		barrier.srcAccessMask = VK_ACCESS_NONE;
		barrier.dstAccessMask = VK_ACCESS_NONE;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image->vk.image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		current_width = image->info.width;
		current_height = image->info.height;

		for (i = 1; i < image->info.mips; i++) {
			VkImageBlit blit = {0};

			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(command_buffer->vk.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

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

			blit.dstOffsets[1].x = current_width > 1 ? current_width / 2 : 1;
			blit.dstOffsets[1].y = current_height > 1 ? current_height / 2 : 1;
			blit.dstOffsets[1].z = 1;

			vkCmdBlitImage(command_buffer->vk.command_buffer, image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(command_buffer->vk.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

			if (current_width > 1)
				current_width /= 2;

			if (current_height > 1)
				current_height /= 2;
		}

		barrier.subresourceRange.baseMipLevel = image->info.mips - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer->vk.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	}
}



sf_public struct sf_graphics_image *sf_graphics_device_init_image_and_upload_data(struct sf_arena *arena, struct sf_graphics_device *device, struct sf_graphics_init_image_info *info, void const *data) {
	u64 data_size_in_bytes = 0;
	struct sf_graphics_init_image_info real_info = {0};
	struct sf_graphics_image *result_image = NULL;
	struct sf_graphics_buffer *staging_buffer = NULL;
	struct sf_graphics_command_buffer *command_buffer = NULL;

	if (!arena || !device || !info || !data)
		return NULL;

	data_size_in_bytes = info->width * info->height * sf_graphics_get_stride_from_format(info->format);
	staging_buffer = sf_graphics_device_init_buffer_for_staging(arena, device, data_size_in_bytes, data);
	if (!staging_buffer)
		goto error;

	real_info = *info;

	real_info.usage |= SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION;
	result_image = sf_graphics_device_init_image(arena, device, &real_info);
	if (!result_image)
		goto error;

	command_buffer = sf_graphics_device_init_and_begin_single_use_command_buffer(arena, device);
	if (!command_buffer)
		goto error;


	sf_graphics_vulkan_command_transition_image_layout(command_buffer, result_image, result_image->info.mips, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	sf_graphics_command_copy_buffer_to_image(command_buffer, 0, result_image->info.width, result_image->info.height, 1, 0, staging_buffer, result_image);
	sf_graphics_vulkan_command_transition_image_layout(command_buffer, result_image, result_image->info.mips, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	if (sf_graphics_check_mip_map_support(device, result_image->info.format))
		sf_graphics_command_generate_image_mip_maps(command_buffer, result_image);


	goto cleanup;

error:
	sf_graphics_device_wait_idle(device);
	sf_graphics_device_deinit_image(device, result_image);
	result_image = NULL;

cleanup:
	sf_graphics_device_end_submit_and_deinit_command_buffer(device, command_buffer);
	sf_graphics_device_deinit_buffer(device, staging_buffer);

	return result_image;
}

sf_public struct sf_graphics_image *sf_graphics_device_init_image_from_file(struct sf_arena *arena, struct sf_graphics_device *device , struct sf_string const *path) {
	struct sf_graphics_init_image_info info = {0};
	u32 width = 0;
	u32 height = 0;
	enum sf_graphics_format format = SF_GRAPHICS_FORMAT_UNDEFINED;
	void *data = NULL;
	struct sf_graphics_image *image = NULL;

	if (!device || !path)
		return NULL;

	data = sf_image_load_from_file(arena, path, &width, &height, &format);
	if (!data)
		return NULL;


	info.type = SF_GRAPHICS_IMAGE_TYPE_2D;
	info.usage = SF_GRAPHICS_IMAGE_USAGE_SAMPLED;
	info.format = format;
	info.samples = SF_GRAPHICS_SAMPLE_COUNT_1;
	info.memory_properties = SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_LOCAL;
	info.width = width;
	info.height = height;
	info.mips = SF_GRAPHICS_CALCULATE_MIPS;

	image = sf_graphics_device_init_image_and_upload_data(arena, device, &info, data);
	sf_image_free(data);
	return image;
}


sf_private void sf_graphics_device_deinit_render_target(struct sf_graphics_device *device, struct sf_graphics_render_target *render_target) {
	if (device && render_target) {
		if (render_target->color_attachment && (render_target->color_attachment != render_target->info.color_attachment)) {
			sf_graphics_device_deinit_image(device, render_target->color_attachment);
			render_target->color_attachment = NULL;
		}

		if (render_target->resolve_attachment) {
			sf_graphics_device_deinit_image(device, render_target->resolve_attachment);
			render_target->resolve_attachment = NULL;
		}

		if (render_target->depth_stencil_attachment) {
			sf_graphics_device_deinit_image(device, render_target->depth_stencil_attachment);
			render_target->resolve_attachment = NULL;
		}

		if (render_target->vk.framebuffer) {
			vkDestroyFramebuffer(device->vk.device, render_target->vk.framebuffer, device->vk.allocation_callbacks);
			render_target->vk.framebuffer = VK_NULL_HANDLE;
		}

		if (render_target->vk.render_pass) {
			vkDestroyRenderPass(device->vk.device, render_target->vk.render_pass, device->vk.allocation_callbacks);
			render_target->vk.render_pass = VK_NULL_HANDLE;
		}
	}
}

sf_private u64 sf_u64_align_up(u64 value, u64 alignment) {
	if (!alignment)
		return value;

	return ((value + alignment - 1) / alignment) * alignment;
}

sf_private u32 sf_graphics_value_or_else(u32 value, u32 default_value) {
	return value ? value : default_value;
}

#define SF_GRAPHICS_MAX_RENDER_TARGET_TOTAL_ATTACHMENT_COUNT 3

struct sf_graphics_vulkan_render_target_attachment_array {
	u32 size;
	VkImageView data[SF_GRAPHICS_MAX_RENDER_TARGET_TOTAL_ATTACHMENT_COUNT];
};

sf_private void sf_graphics_vulkan_load_render_target_attachment_array(struct sf_graphics_render_target *render_target, struct sf_graphics_vulkan_render_target_attachment_array *out) {
	u32 attachment_count = 0;

	SF_ARRAY_INIT(out->data, VK_NULL_HANDLE);

	if (render_target->info.sample_count != SF_GRAPHICS_SAMPLE_COUNT_1 && render_target->resolve_attachment) {
		out->data[attachment_count++] = render_target->resolve_attachment->vk.image_view;
	}
	
	if (render_target->color_attachment) {
		out->data[attachment_count++] = render_target->color_attachment->vk.image_view;
	}

	if (render_target->info.depth_stencil_attachment_format != SF_GRAPHICS_FORMAT_UNDEFINED && render_target->depth_stencil_attachment) {
		out->data[attachment_count++] = render_target->depth_stencil_attachment->vk.image_view;
	}

	out->size = attachment_count;
}

struct sf_graphics_vulkan_render_target_attachment_reference_array {
	u32 has_color_reference;
	VkAttachmentReference color_reference;

	u32 has_resolve_reference;
	VkAttachmentReference resolve_reference;

	sf_bool has_depth_stencil_reference;
	VkAttachmentReference depth_stencil_reference;

	u32 total_attachment_count;
	VkAttachmentDescription descriptions[SF_GRAPHICS_MAX_RENDER_TARGET_TOTAL_ATTACHMENT_COUNT];
};

sf_private void sf_graphics_vulkan_load_render_target_attachment_reference_array(struct sf_graphics_render_target *render_target, struct sf_graphics_vulkan_render_target_attachment_reference_array *out) {
	u32 attachment_count = 0;

	if (!render_target || !out)
		return;

	out->has_resolve_reference = render_target->info.sample_count != SF_GRAPHICS_SAMPLE_COUNT_1 && !!render_target->resolve_attachment;
	if (out->has_resolve_reference) {
		VkAttachmentDescription *attachment = &out->descriptions[attachment_count];

		attachment->flags = 0;
		attachment->format = sf_graphics_vulkan_get_format_from_format(render_target->info.color_attachment_format);
		attachment->samples = sf_graphics_vulkan_get_sample_count_from_sample_count(render_target->info.sample_count);
		attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment->finalLayout = render_target->info.will_be_presented ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		out->resolve_reference.attachment = attachment_count++;
		out->resolve_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // FIXME(samuel): is this layout right?
	}

	out->has_color_reference = !!render_target->color_attachment;
	if (out->has_color_reference) {
		VkAttachmentDescription *attachment = &out->descriptions[attachment_count];

		attachment->flags = 0;
		attachment->format = sf_graphics_vulkan_get_format_from_format(render_target->info.color_attachment_format);
		attachment->samples = sf_graphics_vulkan_get_sample_count_from_sample_count(SF_GRAPHICS_SAMPLE_COUNT_1);
		attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (render_target->info.sample_count == SF_GRAPHICS_SAMPLE_COUNT_1)
			attachment->finalLayout = render_target->info.will_be_presented ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else
			attachment->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		out->color_reference.attachment = attachment_count++;
		out->color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // FIXME(samuel): is this layout right?
	}

	out->has_depth_stencil_reference = render_target->info.depth_stencil_attachment_format != SF_GRAPHICS_FORMAT_UNDEFINED && !!render_target->depth_stencil_attachment;
	if (out->has_depth_stencil_reference) {
		VkAttachmentDescription *attachment = &out->descriptions[attachment_count];

		attachment->flags = 0;
		attachment->format = sf_graphics_vulkan_get_format_from_format(render_target->info.depth_stencil_attachment_format);
		attachment->samples = sf_graphics_vulkan_get_sample_count_from_sample_count(render_target->info.sample_count);
		attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		out->depth_stencil_reference.attachment = attachment_count++;
		out->depth_stencil_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	out->total_attachment_count = attachment_count;
}

sf_private struct sf_graphics_render_target *sf_graphics_device_init_render_target(struct sf_arena *arena, struct sf_graphics_device *device, struct sf_graphics_init_render_target_info const *info) {
	struct sf_graphics_render_target *render_target = NULL;

	if (!device || !device->vk.device || !info || !info->width || !info->height)
		return NULL;

	render_target = sf_arena_allocate(arena, sizeof(*render_target));
	if (!render_target)
		return NULL;

	render_target->info = *info;
	
	if (info->color_attachment) {
		render_target->color_attachment = info->color_attachment;
	} else {
		struct sf_graphics_init_image_info image_info = {0};
		
		image_info.type = SF_GRAPHICS_IMAGE_TYPE_2D;
		image_info.usage = 0;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_TRANSFER_SOURCE;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_SAMPLED;
		image_info.format = info->color_attachment_format;
		image_info.samples = info->sample_count;
		image_info.memory_properties = SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_LOCAL;
		image_info.width = info->width;
		image_info.height = info->height;
		image_info.mips = 1;

		render_target->color_attachment = sf_graphics_device_init_image(arena, device, &image_info);
		if (!render_target->color_attachment)
			goto error;

		// FIXME(SamueL):  there needs to be a second view forced to have a 1 in the alpha channel??
	}

	if (info->sample_count != SF_GRAPHICS_SAMPLE_COUNT_1) {
		struct sf_graphics_init_image_info image_info = {0};

		image_info.type = SF_GRAPHICS_IMAGE_TYPE_2D;
		image_info.usage = 0;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_TRANSFER_SOURCE;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_SAMPLED;
		image_info.format = info->color_attachment_format;
		image_info.samples = SF_GRAPHICS_SAMPLE_COUNT_1;
		image_info.memory_properties = SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_LOCAL;
		image_info.width = info->width;
		image_info.height = info->height;
		image_info.mips = 1;

		render_target->resolve_attachment = sf_graphics_device_init_image(arena, device, &image_info);
		if (!render_target->resolve_attachment)
			goto error;
	}

	if (info->depth_stencil_attachment_format != SF_GRAPHICS_FORMAT_UNDEFINED) {
		struct sf_graphics_init_image_info image_info = {0};

		image_info.type = SF_GRAPHICS_IMAGE_TYPE_2D;
		image_info.usage = 0;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT;
		image_info.usage |= SF_GRAPHICS_IMAGE_USAGE_SAMPLED;
		image_info.format = info->depth_stencil_attachment_format;
		image_info.samples = info->sample_count;
		image_info.memory_properties = SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_LOCAL;
		image_info.width = info->width;
		image_info.height = info->height;
		image_info.mips = 1;

		render_target->depth_stencil_attachment = sf_graphics_device_init_image(arena, device, &image_info);
		if (!render_target->depth_stencil_attachment)
			goto error;
	}

	{
		VkSubpassDescription subpass = {0};
		VkSubpassDependency subpass_dependency = {0};
		VkRenderPassCreateInfo render_pass_info = {0};
		struct sf_graphics_vulkan_render_target_attachment_reference_array attachment_references = {0};

		sf_graphics_vulkan_load_render_target_attachment_reference_array(render_target, &attachment_references);

		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = NULL;
		subpass.colorAttachmentCount = attachment_references.has_color_reference;
		subpass.pColorAttachments = &attachment_references.color_reference;

		if (attachment_references.has_resolve_reference)
			subpass.pResolveAttachments = &attachment_references.resolve_reference;
		else
			subpass.pResolveAttachments = NULL;

		if (attachment_references.has_depth_stencil_reference)
			subpass.pDepthStencilAttachment = &attachment_references.depth_stencil_reference;
		else
			subpass.pDepthStencilAttachment = NULL;

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
		render_pass_info.attachmentCount = attachment_references.total_attachment_count;
		render_pass_info.pAttachments = attachment_references.descriptions;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &subpass_dependency;

		if (!SF_VULKAN_CHECK(vkCreateRenderPass(device->vk.device, &render_pass_info, device->vk.allocation_callbacks, &render_target->vk.render_pass))) {
			render_target->vk.render_pass = VK_NULL_HANDLE;
			goto error;
		}
	}

	{
		VkFramebufferCreateInfo framebuffer_info = {0};
		struct sf_graphics_vulkan_render_target_attachment_array attachments = {0};

		sf_graphics_vulkan_load_render_target_attachment_array(render_target, &attachments);

		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.pNext = NULL;
		framebuffer_info.flags = 0;
		framebuffer_info.renderPass = render_target->vk.render_pass;
		framebuffer_info.attachmentCount = attachments.size;
		framebuffer_info.pAttachments = attachments.data;
		framebuffer_info.width = render_target->info.width;
		framebuffer_info.height = render_target->info.height;
		framebuffer_info.layers = 1;

		if (!SF_VULKAN_CHECK(vkCreateFramebuffer(device->vk.device, &framebuffer_info, device->vk.allocation_callbacks, &render_target->vk.framebuffer))) {
			render_target->vk.framebuffer = VK_NULL_HANDLE;
			goto error;
		}
	}

	return render_target;

error:
	sf_graphics_device_deinit_render_target(device, render_target);
	return NULL;
}


sf_private void sf_graphics_device_deinit_swapchain(struct sf_graphics_device *device, struct sf_graphics_swapchain *swapchain) {
	u32 i = 0;

	if (device && swapchain) {
		for (i = 0; i < SF_SIZE(swapchain->render_targets); ++i) {
			sf_graphics_device_deinit_render_target(device, swapchain->render_targets[i]);
			swapchain->render_targets[i] = NULL;
		}
		swapchain->render_target_count = 0;

		for (i = 0; i < SF_SIZE(swapchain->draw_complete_semaphores); ++i) {
			sf_graphics_device_deinit_semaphore(device, swapchain->draw_complete_semaphores[i]);
			swapchain->draw_complete_semaphores[i] = NULL;
		}
		swapchain->draw_complete_semaphore_count = 0;

		for (i = 0; i < SF_SIZE(swapchain->images); ++i) {
			sf_graphics_device_deinit_image(device, swapchain->images[i]);
			swapchain->images[i] = NULL;
		}
		swapchain->image_count = 0;

		if (device->vk.device) {
			if (swapchain->vk.swapchain) {
				vkDestroySwapchainKHR(device->vk.device, swapchain->vk.swapchain, device->vk.allocation_callbacks);
				swapchain->vk.swapchain = VK_NULL_HANDLE;
			}
		}
	}
}

sf_private struct sf_graphics_swapchain *sf_graphics_device_init_swapchain(struct sf_arena *arena, struct sf_graphics_device *device, struct sf_graphics_swapchain *old_swapchain, struct sf_graphics_init_swapchain_info *init_info) {
	u32 i = 0;
	VkPresentModeKHR vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	VkSurfaceFormatKHR vk_surface_format = {0};
	VkFormat vk_depth_stencil_format = VK_FORMAT_UNDEFINED;
	struct sf_graphics_vulkan_swapchain_image_array swapchain_images = {0};
	struct sf_graphics_swapchain *swapchain = NULL;

	if (!arena || !device || !device->vk.device || !init_info)
		return NULL;

	swapchain = sf_arena_allocate(arena, sizeof(*swapchain));
	if (!swapchain)
		return NULL;

	swapchain->info = *init_info;
	swapchain->current_image_index = 0;

	vk_present_mode = sf_graphics_vulkan_find_swapchain_present_mode(arena, device->vk.surface, device->vk.physical_device, init_info);
	if (vk_present_mode == VK_PRESENT_MODE_FIFO_KHR)
		swapchain->is_vsync_enabled = SF_TRUE;

	swapchain->vk.present_mode = vk_present_mode;

	vk_depth_stencil_format = sf_graphics_vulkan_find_swapchain_depth_stencil_format(device);
	if (vk_depth_stencil_format == VK_FORMAT_UNDEFINED)
		goto error;

	swapchain->depth_stencil_format = sf_graphics_get_format_from_vulkan_format(vk_depth_stencil_format);


	// FIXME(samuel): device and surface order...
	sf_graphics_vulkan_find_supported_color_format(arena, device->vk.physical_device, device->vk.surface, &vk_surface_format);
	if (vk_surface_format.format == VK_FORMAT_UNDEFINED)
		goto error;


	swapchain->vk.color_space = vk_surface_format.colorSpace;
	swapchain->color_format = sf_graphics_get_format_from_vulkan_format(vk_surface_format.format);

	sf_graphics_update_swapchain_dimensions(device, swapchain);
	if (!swapchain->width || !swapchain->height)
		goto error;

	{
		VkSwapchainCreateInfoKHR info = {0};
		u32 queue_family_indices[2] = {0};

		queue_family_indices[0] = device->vk.graphics_queue_family_index;
		queue_family_indices[1] = device->vk.present_queue_family_index;

		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.pNext = NULL;
		info.flags = 0;
		info.surface = device->vk.surface;
		info.minImageCount = init_info->requested_image_count;
		info.imageFormat = sf_graphics_vulkan_get_format_from_format(swapchain->color_format);
		info.imageColorSpace = swapchain->vk.color_space;
		info.imageExtent.width = swapchain->width;
		info.imageExtent.height = swapchain->height;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (device->vk.graphics_queue_family_index == device->vk.present_queue_family_index) {
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 1;
			info.pQueueFamilyIndices = queue_family_indices;
		} else {
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = SF_SIZE(queue_family_indices);
			info.pQueueFamilyIndices = queue_family_indices;
		}
		info.preTransform = device->vk.physical_device_surface_capabilities.currentTransform;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = swapchain->vk.present_mode;
		info.clipped = VK_TRUE;
		info.oldSwapchain = old_swapchain ? old_swapchain->vk.swapchain : VK_NULL_HANDLE;

		if (!SF_VULKAN_CHECK(vkCreateSwapchainKHR(device->vk.device, &info, device->vk.allocation_callbacks, &swapchain->vk.swapchain))) {
			swapchain->vk.swapchain = VK_NULL_HANDLE;
			goto error;
		}
	}

	sf_graphics_vulkan_load_swapchain_image_array(device->vk.device, swapchain->vk.swapchain, &swapchain_images);
	if (!swapchain_images.size)
		goto error;

	swapchain->image_count = swapchain_images.size;
	for (i = 0; i < swapchain->image_count; ++i) {
		struct sf_graphics_init_image_info image_info = {0};

		image_info.type = SF_GRAPHICS_IMAGE_TYPE_2D;
		image_info.usage = SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT;
		image_info.format = swapchain->color_format;
		image_info.samples = SF_GRAPHICS_SAMPLE_COUNT_1;
		image_info.width = swapchain->width;
		image_info.height = swapchain->height;
		image_info.mips = 1;
		image_info.vk.not_owned_image = swapchain_images.data[i];

		swapchain->images[i] = sf_graphics_device_init_image(arena, device, &image_info);
		if (!swapchain->images[i])
			goto error;
	}

	swapchain->draw_complete_semaphore_count = swapchain->image_count;
	for (i = 0; i < swapchain->draw_complete_semaphore_count; ++i) {
		swapchain->draw_complete_semaphores[i] = sf_graphics_device_init_semaphore(arena, device);
		if (!swapchain->draw_complete_semaphores[i])
			goto error;
	}

	swapchain->render_target_count = swapchain->image_count;
	for (i = 0; i < swapchain->render_target_count; ++i) {
		struct sf_graphics_init_render_target_info render_target_info = {0};

		render_target_info.width = swapchain->width;
		render_target_info.height = swapchain->height;
		render_target_info.color_attachment_format = swapchain->color_format;
		render_target_info.color_attachment = swapchain->images[i];
		render_target_info.depth_stencil_attachment_format = swapchain->depth_stencil_format;
		render_target_info.sample_count = init_info->sample_count; // FIXME(samuel): check if sample count is supported
		render_target_info.color_clear_value = init_info->color_clear_value;
		render_target_info.depth_stencil_clear_value = init_info->depth_stencil_clear_value;
		render_target_info.will_be_presented = SF_TRUE;

		swapchain->render_targets[i] = sf_graphics_device_init_render_target(arena, device, &render_target_info);
		if (!swapchain->render_targets[i])
			goto error;

	}

	return swapchain;
error:
	sf_graphics_device_deinit_swapchain(device, swapchain);
	return NULL;
}


sf_private void sf_graphics_device_deinit_shader(struct sf_graphics_device *device, struct sf_graphics_shader *shader) {
	if (device && shader) {
		if (device->vk.device) {
			if (shader->vk.shader) {
				vkDestroyShaderModule(device->vk.device, shader->vk.shader, device->vk.allocation_callbacks);
				shader->vk.shader = VK_NULL_HANDLE;
			}
		}
	}
}

sf_private struct sf_graphics_shader *sf_graphics_device_init_shader(struct sf_arena *arena, struct sf_graphics_device *device, u64 spirv_code_size_in_bytes, void const *spirv_code) {
	struct sf_graphics_shader *shader = NULL;

	if (!device || !device->vk.device || !spirv_code_size_in_bytes || !spirv_code)
		return NULL;

	shader = sf_arena_allocate(arena, sizeof(*shader));
	if (!shader)
		return NULL;

	{
		VkShaderModuleCreateInfo info = {0};

		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.codeSize = spirv_code_size_in_bytes;
		info.pCode = (u32 const *)spirv_code;

		if (!SF_VULKAN_CHECK(vkCreateShaderModule(device->vk.device, &info, device->vk.allocation_callbacks, &shader->vk.shader))) {
			shader->vk.shader = VK_NULL_HANDLE;
			goto error;
		}
	}

	return shader;

error:
	sf_graphics_device_deinit_shader(device, shader);
	return NULL;
}

sf_private void sf_graphics_device_deinit_pipeline(struct sf_graphics_device *device, struct sf_graphics_pipeline *pipeline) {
	if (device && pipeline) {
		if (device->vk.device) {
			if (pipeline->vk.pipeline) {
				vkDestroyPipeline(device->vk.device, pipeline->vk.pipeline, device->vk.allocation_callbacks);
				pipeline->vk.pipeline = VK_NULL_HANDLE;
			}
		}
	}
}

struct sf_graphics_vulkan_vertex_layout_description {
	VkVertexInputBindingDescription binding_description;
	u32 attribute_count;
	VkVertexInputAttributeDescription attribute_descriptions[SF_GRAPHICS_MAX_VERTEX_LAYOUT_ATTRIBUTE_COUNT];
};

sf_private void sf_graphics_vulkan_fill_vulkan_vertex_layout_description(struct sf_graphics_vertex_layout const *layout, struct sf_graphics_vulkan_vertex_layout_description *out_layout) {
	SF_STATIC_ASSERT(SF_SIZE(layout->attributes) == SF_SIZE(out_layout->attribute_descriptions), layout_attribute_description_size_does_not_match);

	u32 i = 0;

	if (!layout || !out_layout)
		return;

	out_layout->binding_description.binding = 0;
	out_layout->binding_description.stride = layout->stride;
	out_layout->binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	for (i = 0; i < layout->attribute_count; ++i) {
		VkVertexInputAttributeDescription *attribute_description = &out_layout->attribute_descriptions[i];

		attribute_description->location = i;
		attribute_description->binding = 0;
		attribute_description->format = sf_graphics_vulkan_get_format_from_format(layout->attributes[i].format);
		attribute_description->offset = layout->attributes[i].offset;
	}

	out_layout->attribute_count = layout->attribute_count;
}

struct sf_graphics_vulkan_color_blend_attachment_state_array {
	u32 size;
	VkPipelineColorBlendAttachmentState data[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT];
};

sf_private void sf_graphics_vulkan_load_color_blend_attachment_state_array(struct sf_graphics_render_target const *render_target, struct sf_graphics_vulkan_color_blend_attachment_state_array *out) {
	u32 i = 0;

	if (!render_target || !out)
		return;

	if (render_target->color_attachment) {
		VkPipelineColorBlendAttachmentState *state = &out->data[i++];

		state->blendEnable = VK_FALSE;
		state->srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		state->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		state->colorBlendOp = VK_BLEND_OP_ADD;
		state->srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		state->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		state->alphaBlendOp = VK_BLEND_OP_ADD;
		state->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	}

	out->size = i;
}

sf_private struct sf_graphics_pipeline *sf_graphics_device_init_pipeline(struct sf_arena *arena, struct sf_graphics_device *device, struct sf_graphics_init_pipeline_info const *info) {
	VkPipelineShaderStageCreateInfo shader_stages[2] = {0};
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {0};
	// VkPipelineTessellationStateCreateInfo tessellation_info = {0};
	VkPipelineViewportStateCreateInfo viewport_info = {0};
	VkPipelineRasterizationStateCreateInfo rasterization_info = {0};
	VkPipelineMultisampleStateCreateInfo multisample_info = {0};
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {0};
	VkPipelineColorBlendStateCreateInfo color_blend_info = {0};
	VkPipelineDynamicStateCreateInfo dynamic_info = {0};
	VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkGraphicsPipelineCreateInfo pipeline_info = {0};

	struct sf_graphics_pipeline *pipeline = NULL;
	struct sf_graphics_vulkan_vertex_layout_description vertex_layout_description = {0};
	struct sf_graphics_vulkan_color_blend_attachment_state_array color_blend_attachment_states = {0};
	

	if (!arena || !device || !info || !info->reference_render_target || !info->vertex_shader || !info->fragment_shader)
		return NULL;

	pipeline = sf_arena_allocate(arena, sizeof(*pipeline));
	if (!pipeline)
		return NULL;

	pipeline->info = *info;

	sf_graphics_vulkan_fill_vulkan_vertex_layout_description(&info->vertex_layout, &vertex_layout_description);
	sf_graphics_vulkan_load_color_blend_attachment_state_array(info->reference_render_target, &color_blend_attachment_states);

	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].pNext = NULL;
	shader_stages[0].flags = 0;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = info->vertex_shader->vk.shader;
	shader_stages[0].pName = "main";
	shader_stages[0].pSpecializationInfo = NULL;

	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].pNext = NULL;
	shader_stages[1].flags = 0;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = info->fragment_shader->vk.shader;
	shader_stages[1].pName = "main";
	shader_stages[1].pSpecializationInfo = NULL;

	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.pNext = NULL;
	vertex_input_info.flags = 0;
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &vertex_layout_description.binding_description;
	vertex_input_info.vertexAttributeDescriptionCount = vertex_layout_description.attribute_count;
	vertex_input_info.pVertexAttributeDescriptions = vertex_layout_description.attribute_descriptions;

	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.pNext = NULL;
	input_assembly_info.flags = 0;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.pNext = NULL;
	viewport_info.flags = 0;
	viewport_info.viewportCount = !!info->reference_render_target->color_attachment;
	viewport_info.pViewports = NULL;
	viewport_info.scissorCount = !!info->reference_render_target->color_attachment;
	viewport_info.pScissors = NULL;

	rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_info.pNext = NULL;
	rasterization_info.flags = 0;
	rasterization_info.depthClampEnable = VK_FALSE;
	rasterization_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_info.depthBiasEnable = VK_FALSE;
	rasterization_info.depthBiasConstantFactor = 0.0F;
	rasterization_info.depthBiasClamp = 0.0f;
	rasterization_info.depthBiasSlopeFactor = 0.0F;
	rasterization_info.lineWidth = 1.0F;

	multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_info.pNext = NULL;
	multisample_info.flags = 0;
	multisample_info.rasterizationSamples = sf_graphics_vulkan_get_sample_count_from_sample_count(info->reference_render_target->info.sample_count);
	multisample_info.sampleShadingEnable = VK_FALSE;
	multisample_info.minSampleShading = 0.0F;
	multisample_info.pSampleMask = NULL;
	multisample_info.alphaToCoverageEnable = VK_FALSE;
	multisample_info.alphaToOneEnable = VK_FALSE;

	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.pNext = NULL;
	depth_stencil_info.flags = 0;
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_info.stencilTestEnable = VK_FALSE;
	depth_stencil_info.front.failOp = VK_STENCIL_OP_KEEP;
	depth_stencil_info.front.passOp = VK_STENCIL_OP_KEEP;
	depth_stencil_info.front.depthFailOp = VK_STENCIL_OP_KEEP;
	depth_stencil_info.front.compareOp = VK_COMPARE_OP_NEVER;
	depth_stencil_info.front.compareMask = 0;
	depth_stencil_info.front.writeMask = 0;
	depth_stencil_info.front.reference = 0;
	depth_stencil_info.back.failOp = VK_STENCIL_OP_KEEP;
	depth_stencil_info.back.passOp = VK_STENCIL_OP_KEEP;
	depth_stencil_info.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depth_stencil_info.back.compareOp = VK_COMPARE_OP_NEVER;
	depth_stencil_info.back.compareMask = 0;
	depth_stencil_info.back.writeMask = 0;
	depth_stencil_info.back.reference = 0;
	depth_stencil_info.minDepthBounds = 0.0F;
	depth_stencil_info.maxDepthBounds = 1.0F;

	color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_info.pNext = NULL;
	color_blend_info.flags = 0;
	color_blend_info.logicOpEnable = VK_FALSE;
	color_blend_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_info.attachmentCount = color_blend_attachment_states.size;
	color_blend_info.pAttachments = color_blend_attachment_states.data;
	color_blend_info.blendConstants[0] = 0.0F;
	color_blend_info.blendConstants[1] = 0.0F;
	color_blend_info.blendConstants[2] = 0.0F;
	color_blend_info.blendConstants[3] = 0.0F;

	dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_info.pNext = NULL;
	dynamic_info.flags = 0;
	dynamic_info.dynamicStateCount = SF_SIZE(dynamic_states);
	dynamic_info.pDynamicStates = dynamic_states;

	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = NULL;
	pipeline_info.flags = 0;
	pipeline_info.stageCount = SF_SIZE(shader_stages);
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pTessellationState = NULL;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &rasterization_info;
	pipeline_info.pMultisampleState = &multisample_info;
	pipeline_info.pDepthStencilState = &depth_stencil_info;
	pipeline_info.pColorBlendState = &color_blend_info;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.layout = device->vk.general_pipeline_layout;
	pipeline_info.renderPass = info->reference_render_target->vk.render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	if (SF_VULKAN_CHECK(vkCreateGraphicsPipelines(device->vk.device, VK_NULL_HANDLE, 1, &pipeline_info, device->vk.allocation_callbacks, &pipeline->vk.pipeline))) {
		pipeline->vk.pipeline = VK_NULL_HANDLE;
		goto error;
	}

	return pipeline;
error:
	sf_graphics_device_deinit_pipeline(device, pipeline);
	return NULL;
}

sf_private void sf_graphics_dynamic_buffer_clear_garbage(struct sf_graphics_device *device, struct sf_graphics_dynamic_buffer *dynamic_buffer) {
	u32 i = 0;
	
	if (!device || !dynamic_buffer)
		return;

	for (i = 0; i < SF_SIZE(dynamic_buffer->garbage_buffers); ++i) {
		sf_graphics_device_deinit_buffer(device, dynamic_buffer->garbage_buffers[i]);
		dynamic_buffer->garbage_buffers[i] = NULL;
	}
}

sf_private sf_bool sf_graphics_dynamic_buffer_move_buffer_to_garbage(struct sf_graphics_dynamic_buffer *dynamic_buffer) {
	if (dynamic_buffer->garbage_buffer_count >= SF_SIZE(dynamic_buffer->garbage_buffers))
		return SF_FALSE;

	dynamic_buffer->garbage_buffers[dynamic_buffer->garbage_buffer_count++] = dynamic_buffer->buffer;
	dynamic_buffer->buffer = NULL;

	return SF_TRUE;
}

sf_private void sf_graphics_device_deinit_dynamic_buffer(struct sf_graphics_device *device, struct sf_graphics_dynamic_buffer *dynamic_buffer) {
	if (device && dynamic_buffer) {
		sf_graphics_device_deinit_buffer(device, dynamic_buffer->buffer);
		sf_graphics_dynamic_buffer_clear_garbage(device, dynamic_buffer);
	}
}

sf_private sf_bool sf_graphics_dynamic_buffer_init_buffer_and_setup_arena(struct sf_arena *arena, struct sf_graphics_device *device, sf_graphics_buffer_usage_flags usage, u64 size, struct sf_graphics_dynamic_buffer *dynamic_buffer) {
	struct sf_graphics_init_buffer_info info = {0};

	if (!arena || !device || !size || !dynamic_buffer)
		return SF_FALSE;

	info.size = size;
	info.usage = usage;
	info.memory_properties = 0;
	info.memory_properties |= SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE;
	info.memory_properties |= SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_LOCAL;

	dynamic_buffer->buffer = sf_graphics_device_init_buffer(arena, device, &info);
	if (!dynamic_buffer->buffer)
		return SF_FALSE;

	dynamic_buffer->mapped_buffer_arena.data = dynamic_buffer->buffer->cpu_mapped_data;
	dynamic_buffer->mapped_buffer_arena.position = 0;
	dynamic_buffer->mapped_buffer_arena.alignment = 256; // FIXME(samuel): Actually query this and use the correct value
	dynamic_buffer->mapped_buffer_arena.capacity = size; // FIXME(samuel): Actually query this and use the correct value

	return SF_TRUE;
}


sf_private struct sf_graphics_dynamic_buffer *sf_graphics_device_init_dynamic_buffer(struct sf_arena *arena, struct sf_graphics_device *device, sf_graphics_buffer_usage_flags usage, u64 size) {
	struct sf_graphics_dynamic_buffer *dynamic_buffer = NULL;

	if (!arena || !device || !size)
		return NULL;

	dynamic_buffer = sf_arena_allocate(arena, sizeof(*dynamic_buffer));
	if (!dynamic_buffer)
		return NULL;

	if (!sf_graphics_dynamic_buffer_init_buffer_and_setup_arena(arena, device, usage, size, dynamic_buffer))
		goto error;

	return dynamic_buffer;

error:
	sf_graphics_device_deinit_dynamic_buffer(device, dynamic_buffer);
	return NULL;
}

sf_private void sf_graphics_dynamic_buffer_allocate(struct sf_arena *arena, struct sf_graphics_device *device, struct sf_graphics_dynamic_buffer *dynamic_buffer, u64 size, struct sf_graphics_dynamic_buffer_allocation *allocation) {
	if (!device || !dynamic_buffer || !dynamic_buffer->buffer || !size || !allocation)
		return;

	allocation->buffer = NULL;
	allocation->offset32 = 0;
	allocation->offset64 = 0;
	allocation->memory = sf_arena_allocate(&dynamic_buffer->mapped_buffer_arena, size);
	if (!allocation->memory) {
		u64 new_buffer_size = 0;
		u64 previous_buffer_size = 0;
		sf_graphics_buffer_usage_flags previous_buffer_usage_flags = 0;

		previous_buffer_size = dynamic_buffer->buffer->info.size;
		previous_buffer_usage_flags = dynamic_buffer->buffer->info.usage;
		new_buffer_size = SF_MAX(previous_buffer_size, size) * 2;

		if (!sf_graphics_dynamic_buffer_move_buffer_to_garbage(dynamic_buffer))
			return;
		
		if (!sf_graphics_dynamic_buffer_init_buffer_and_setup_arena(arena, device, previous_buffer_usage_flags, new_buffer_size, dynamic_buffer))
			return;

		allocation->memory = sf_arena_allocate(&dynamic_buffer->mapped_buffer_arena, size);
		if (!allocation->memory)
			return;
	}

	allocation->buffer = dynamic_buffer->buffer;
	allocation->offset32 = (u32)dynamic_buffer->mapped_buffer_arena.position;
	allocation->offset64 = dynamic_buffer->mapped_buffer_arena.position;
} 


sf_private void sf_graphics_device_deinit_frame(struct sf_graphics_device *device, struct sf_graphics_frame *frame) {
	if (device && frame) {
		if (frame->vertex_buffer) {
			sf_graphics_device_deinit_dynamic_buffer(device, frame->vertex_buffer);
			frame->vertex_buffer = NULL;
		}

		if (frame->index_buffer) {
			sf_graphics_device_deinit_dynamic_buffer(device, frame->index_buffer);
			frame->index_buffer = NULL;
		}

		if (frame->uniform_buffer) {
			sf_graphics_device_deinit_dynamic_buffer(device, frame->uniform_buffer);
			frame->uniform_buffer = NULL;
		}

		if (frame->in_flight_fence) {
			sf_graphics_device_deinit_fence(device, frame->in_flight_fence);
			frame->in_flight_fence = NULL;
		}

		if (frame->image_acquired_semaphore) {
			sf_graphics_device_deinit_semaphore(device, frame->image_acquired_semaphore);
			frame->image_acquired_semaphore = NULL;
		}

		if (frame->command_buffer) {
			sf_graphics_device_deinit_command_buffer(device, frame->command_buffer);
			frame->command_buffer = NULL;
		}
	}
}

sf_private struct sf_graphics_frame *sf_graphics_device_init_frame(struct sf_arena *arena, struct sf_graphics_device *device, u64 initial_uniform_buffer_size, u64 initial_index_buffer_size, u64 initial_vertex_buffer_size) {
	struct sf_graphics_frame *frame = NULL;

	if (!arena || !device)
		return NULL;

	frame = sf_arena_allocate(arena, sizeof(*frame));
	if (!frame)
		goto error;

	frame->command_buffer = sf_graphics_device_init_command_buffer(arena, device, 0);
	if (!frame->command_buffer)
		goto error;

	frame->image_acquired_semaphore = sf_graphics_device_init_semaphore(arena, device);
	if (!frame->image_acquired_semaphore)
		goto error;

	frame->in_flight_fence = sf_graphics_device_init_fence(arena, device, SF_TRUE);
	if (!frame->in_flight_fence)
		goto error;

	frame->uniform_buffer = sf_graphics_device_init_dynamic_buffer(arena, device, SF_GRAPHICS_BUFFER_USAGE_UNIFORM_BUFFER, initial_uniform_buffer_size);
	if (!frame->uniform_buffer)
		goto error;

	frame->index_buffer = sf_graphics_device_init_dynamic_buffer(arena, device, SF_GRAPHICS_BUFFER_USAGE_INDEX_BUFFER, initial_index_buffer_size);
	if (!frame->index_buffer)
		goto error;

	frame->vertex_buffer = sf_graphics_device_init_dynamic_buffer(arena, device, SF_GRAPHICS_BUFFER_USAGE_VERTEX_BUFFER, initial_vertex_buffer_size);
	if (!frame->vertex_buffer)
		goto error;
	
	return frame;

error:
	sf_graphics_device_deinit_frame(device, frame);
	return NULL;
}

sf_private void sf_graphics_context_deinit_frames(struct sf_graphics_context *context) {
	u32 i = 0;
	if (context && context->device) {
		for (i = 0; i < SF_SIZE(context->frames); ++i) {
			if (context->frames[i]) {
				sf_graphics_device_deinit_frame(context->device, context->frames[i]);
				context->frames[i] = NULL;
			}
		}
		context->frame_count = 0;
	}
}

sf_private u64 sf_graphics_get_max_uniform_buffer_range(struct sf_graphics_context *context) {
	return context->device->vk.physical_device_properties.limits.maxUniformBufferRange;
}

sf_private void sf_graphics_context_init_frames(struct sf_arena *arena, struct sf_graphics_context *context, u32 buffering_count) {
	u32 i = 0;

	if (!arena || !context || buffering_count > SF_SIZE(context->frames) || !context->device)
		return;

	context->current_frame_index = 0;
	SF_ARRAY_INIT(context->frames, NULL);


	for (i = 0; i < buffering_count; ++i) {
		context->frames[i] = sf_graphics_device_init_frame(arena, context->device, SF_KB(2), SF_MB(64), SF_MB(64));
		if (!context->frames[i])
			goto error;
	}

	context->frame_count = buffering_count;

	return;

error:
	sf_graphics_context_deinit_frames(context);
}

sf_public void sf_graphics_deinit_context(struct sf_graphics_context *context) {
	if (context) {
		sf_graphics_device_wait_idle(context->device);

		sf_graphics_deinit_image(context, context->default_bound_image);
		context->default_bound_image = NULL;

		sf_graphics_context_deinit_frames(context);

		sf_graphics_device_deinit_swapchain(context->device, context->swapchain);
		context->swapchain = NULL;

		sf_graphics_deinit_device(context->device);
		context->device = NULL;
	}
}

sf_public struct sf_graphics_context *sf_graphics_init_context(struct sf_arena *arena, struct sf_graphics_init_context_info *init_info) {
	struct sf_graphics_context *context = NULL;
	struct sf_string default_bound_image_path = {0};

	if (!arena || !init_info)
		return NULL;

	context = sf_arena_allocate(arena, sizeof(*context));
	if (!context)
		return NULL;

	context->info = *init_info; 

	sf_arena_scratch(arena, SF_MB(2), &context->arena);
	if (!context->arena.data)
		goto error;

	context->device = sf_graphics_init_device(&context->arena, &init_info->device_info);
	if (!context->device)
		goto error;


	context->current_swapchain_arena_index = 0;

	sf_arena_scratch(&context->arena, SF_KB(64), &context->swapchain_arenas[0]);
	if (!context->swapchain_arenas[0].data)
		goto error;

	sf_arena_scratch(&context->arena, SF_KB(64), &context->swapchain_arenas[1]);
	if (!context->swapchain_arenas[1].data)
		goto error;

	context->swapchain = sf_graphics_device_init_swapchain(&context->swapchain_arenas[0], context->device, NULL, &init_info->swapchain_info);
	if (!context->swapchain)
		goto error;

	sf_graphics_context_init_frames(&context->arena, context, init_info->buffering_count);
	if (!context->frame_count)
		goto error;

	context->current_swapchain_arena_index = 0;

	default_bound_image_path = SF_STRING("resources\\test.jpg");
	context->default_bound_image = sf_graphics_device_init_image_from_file(&context->arena, context->device, &default_bound_image_path);
	if (!context->default_bound_image)
		goto error;


	return context;

error:
	sf_graphics_deinit_context(context);
	return NULL;
}

sf_private struct sf_graphics_render_target *sf_graphics_get_current_swapchain_render_target(struct sf_graphics_swapchain *swapchain) {
	return swapchain->render_targets[swapchain->current_image_index];
}

sf_private struct sf_graphics_semaphore *sf_graphics_get_current_swapchain_semaphore(struct sf_graphics_swapchain *swapchain) {
	return swapchain->draw_complete_semaphores[swapchain->current_image_index];
}

sf_private struct sf_graphics_frame *sf_graphics_get_current_frame(struct sf_graphics_context *context) {
	return context->frames[context->current_frame_index];
}
 
sf_private struct sf_graphics_render_target *sf_graphics_acquire_next_swapchain_render_target(struct sf_graphics_context *context) {
	VkResult vk_result = 0;
	struct sf_graphics_frame *frame = NULL;
	struct sf_graphics_device *device = NULL;
	struct sf_graphics_swapchain *swapchain = NULL;

	if (!context || !context->device || !context->device->vk.device || !context->swapchain || !context->swapchain->vk.swapchain)
		return NULL;

	device = context->device;
	swapchain = context->swapchain;
	frame = sf_graphics_get_current_frame(context);

	vk_result = vkAcquireNextImageKHR(device->vk.device, swapchain->vk.swapchain, (u64)-1, frame->image_acquired_semaphore->vk.semaphore, VK_NULL_HANDLE, &swapchain->current_image_index);
	if (vk_result == VK_ERROR_OUT_OF_DATE_KHR) {
		SF_VULKAN_CHECK(vk_result);
		return NULL;
	} else if (vk_result == VK_SUBOPTIMAL_KHR) {
		return sf_graphics_get_current_swapchain_render_target(swapchain);
	} else if (!SF_VULKAN_CHECK(vk_result)) {
		return NULL;
	} else {
		return sf_graphics_get_current_swapchain_render_target(swapchain);
	}
}

sf_private void sf_graphics_prepare_next_frame(struct sf_graphics_context *context) {
	context->current_frame_index = (context->current_frame_index + 1) % context->info.buffering_count;
}

sf_private void sf_graphics_wait_for_fence(struct sf_graphics_device *device, struct sf_graphics_fence *fence) {
	if (!device || !device->vk.device || !fence || !fence->vk.fence)
		return;

	SF_UNUSED(vkWaitForFences(device->vk.device, 1, &fence->vk.fence, VK_TRUE, (u64)-1));
}

sf_private void sf_graphics_reset_fence(struct sf_graphics_device *device, struct sf_graphics_fence *fence) {
	if (!device || !device->vk.device || !fence || !fence->vk.fence)
		return;

	SF_UNUSED(vkResetFences(device->vk.device, 1, &fence->vk.fence));
}

sf_private sf_bool sf_graphics_try_to_rebuild_swapchain(struct sf_graphics_context *context) {
	u32 new_arena_index = 0;
	struct sf_graphics_swapchain *new_swapchain = NULL;

	if (!context || !context->device)
		return SF_FALSE;

	sf_graphics_device_wait_idle(context->device);

	new_arena_index = (context->current_swapchain_arena_index + 1) % SF_SIZE(context->swapchain_arenas);
	new_swapchain = sf_graphics_device_init_swapchain(&context->swapchain_arenas[new_arena_index], context->device, context->swapchain, &context->info.swapchain_info);
	if (!new_swapchain)
		return SF_FALSE;

	sf_graphics_device_deinit_swapchain(context->device, context->swapchain);
	sf_arena_clear(&context->swapchain_arenas[context->current_swapchain_arena_index]);

	context->swapchain = new_swapchain;
	context->current_swapchain_arena_index = new_arena_index;
		
	return SF_TRUE;
}

sf_public void sf_graphics_begin_frame(struct sf_graphics_context *context) {
	// https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html
	struct sf_graphics_render_target *render_target = NULL;
	struct sf_graphics_frame *frame = NULL;

	if (!context->device || !context->swapchain)
		return;

	frame = sf_graphics_get_current_frame(context);
	sf_graphics_wait_for_fence(context->device, frame->in_flight_fence);

	render_target = sf_graphics_acquire_next_swapchain_render_target(context);
	if (!render_target) {
		sf_graphics_try_to_rebuild_swapchain(context);
		context->skip_end_frame = SF_TRUE;
		return;
	}

	sf_graphics_reset_fence(context->device, frame->in_flight_fence);
	sf_graphics_reset_command_buffer(context->device, frame->command_buffer);
	sf_graphics_begin_command_buffer(frame->command_buffer);
	sf_graphics_command_begin_render_pass(context->device, frame->command_buffer, render_target);
}

sf_public void sf_graphics_end_frame(struct sf_graphics_context *context) {
	struct sf_graphics_frame *frame = NULL;
	struct sf_graphics_semaphore *draw_complete_semaphore = NULL;

	if (!context || !context->device || !context->swapchain)
		return;

	if (context->skip_end_frame) {
		context->skip_end_frame = SF_FALSE;
		return;
	}

	frame = sf_graphics_get_current_frame(context);
	draw_complete_semaphore = sf_graphics_get_current_swapchain_semaphore(context->swapchain);

	sf_graphics_command_end_render_pass(frame->command_buffer);
	sf_graphics_end_command_buffer(frame->command_buffer);

	{
		VkSubmitInfo submit_info = {0};
		VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &frame->image_acquired_semaphore->vk.semaphore;
		submit_info.pWaitDstStageMask = &stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &frame->command_buffer->vk.command_buffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &draw_complete_semaphore->vk.semaphore;

		SF_VULKAN_CHECK(vkQueueSubmit(context->device->vk.graphics_queue, 1, &submit_info, frame->in_flight_fence->vk.fence));
	}

	{
		VkResult result = VK_SUCCESS;
		VkPresentInfoKHR present_info = {0};

		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = NULL;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &draw_complete_semaphore->vk.semaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &context->swapchain->vk.swapchain;
		present_info.pImageIndices = &context->swapchain->current_image_index;
		present_info.pResults = NULL;

		result = vkQueuePresentKHR(context->device->vk.present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			sf_graphics_try_to_rebuild_swapchain(context);
		}
	}

	sf_graphics_prepare_next_frame(context);
}

sf_public void sf_graphics_command_bind_pipeline(struct sf_graphics_command_buffer *command_buffer, struct sf_graphics_pipeline *pipeline) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !pipeline || !pipeline->vk.pipeline)
		return;

	vkCmdBindPipeline(command_buffer->vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk.pipeline);
}

sf_public void sf_graphics_command_bind_vertex_buffer(struct sf_graphics_command_buffer *command_buffer, struct sf_graphics_buffer *vertex_buffer, u64 offset) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !vertex_buffer || !vertex_buffer->vk.buffer)
		return;

	vkCmdBindVertexBuffers(command_buffer->vk.command_buffer, 0, 1, &vertex_buffer->vk.buffer, &offset);
}

sf_public void sf_graphics_command_bind_index_buffer(struct sf_graphics_command_buffer *command_buffer, struct sf_graphics_buffer *index_buffer, u64 offset) {
	if (!command_buffer || !command_buffer->vk.command_buffer || !index_buffer || !index_buffer->vk.buffer)
		return;

	vkCmdBindIndexBuffer(command_buffer->vk.command_buffer, index_buffer->vk.buffer, offset, VK_INDEX_TYPE_UINT32);
}

sf_public void *sf_graphics_command_allocate_and_bind_immediate_vertex_memory(struct sf_graphics_context *context, u64 size) {
	struct sf_graphics_dynamic_buffer_allocation allocation = {0};
	struct sf_graphics_frame *frame = NULL;

	if (!context || !size)
		return NULL;

	frame = sf_graphics_get_current_frame(context);

	sf_graphics_dynamic_buffer_allocate(&context->arena, context->device, frame->vertex_buffer, size, &allocation);
	if (!allocation.memory)
		return NULL;

	sf_graphics_command_bind_vertex_buffer(frame->command_buffer, allocation.buffer, allocation.offset64);
	return allocation.memory;
}


sf_public void *sf_graphics_command_allocate_and_bind_immediate_index_memory(struct sf_graphics_context *context, u64 size) {
	struct sf_graphics_dynamic_buffer_allocation allocation = {0};
	struct sf_graphics_frame *frame = NULL;

	if (!context || !size)
		return NULL;

	frame = sf_graphics_get_current_frame(context);

	sf_graphics_dynamic_buffer_allocate(&context->arena, context->device, frame->index_buffer, size, &allocation);
	if (!allocation.memory)
		return NULL;

	sf_graphics_command_bind_index_buffer(frame->command_buffer, allocation.buffer, allocation.offset64);
	return allocation.memory;
}

sf_public void *sf_graphics_command_allocate_and_bind_immediate_uniform_memory(struct sf_graphics_context *context, u64 size) {
	struct sf_graphics_dynamic_buffer_allocation allocation = {0};
	struct sf_graphics_frame *frame = NULL;

	if (!context || !size || !context->device || !context->device->vk.dynamic_uniform_descriptor_set_layout)
		return NULL;

	frame = sf_graphics_get_current_frame(context);
	if (!frame->command_buffer || !frame->command_buffer->vk.command_buffer)
		return NULL;


	sf_graphics_dynamic_buffer_allocate(&context->arena, context->device, frame->uniform_buffer, size, &allocation);
	if (!allocation.memory)
		return NULL;

	vkCmdBindDescriptorSets(frame->command_buffer->vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->device->vk.general_pipeline_layout, 0, 1, &allocation.buffer->vk.descriptor_set, 1, &allocation.offset32);
	return allocation.memory;
}

sf_public void sf_graphics_command_bind_image_to_slot0(struct sf_graphics_context *context, struct sf_graphics_image *image) {
	struct sf_graphics_frame *frame = NULL;

	if (!context || !image || !image->vk.descriptor_set)
		return;

	frame = sf_graphics_get_current_frame(context);
	if (!frame->command_buffer || !frame->command_buffer->vk.command_buffer)
		return;

	vkCmdBindDescriptorSets(frame->command_buffer->vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->device->vk.general_pipeline_layout, 2, 1, &image->vk.descriptor_set, 0, NULL);
}

sf_public void sf_graphics_command_bind_image_to_slot1(struct sf_graphics_context *context, struct sf_graphics_image *image) {
	struct sf_graphics_frame *frame = NULL;

	if (!context || !image || !image->vk.descriptor_set)
		return;

	frame = sf_graphics_get_current_frame(context);
	if (!frame->command_buffer || !frame->command_buffer->vk.command_buffer)
		return;

	vkCmdBindDescriptorSets(frame->command_buffer->vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->device->vk.general_pipeline_layout, 3, 1, &image->vk.descriptor_set, 0, NULL);
}



sf_public void sf_graphics_command_draw_indexed(struct sf_graphics_context *context, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) {
	struct sf_graphics_frame *frame = NULL;

	if (!context)
		return;

	frame = sf_graphics_get_current_frame(context);
	if (!frame->command_buffer || !frame->command_buffer->vk.command_buffer)
		return;

	vkCmdDrawIndexed(frame->command_buffer->vk.command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}
sf_public struct sf_graphics_pipeline *sf_graphics_init_pipeline(struct sf_graphics_context *context, struct sf_graphics_init_pipeline_info const *info) {
	if (!context || !info)
		return NULL;

	return sf_graphics_device_init_pipeline(&context->arena, context->device, info);
}

sf_public void sf_graphics_deinit_pipeline(struct sf_graphics_context *context, struct sf_graphics_pipeline *pipeline) {
	if (!context || !pipeline)
		return;

	sf_graphics_device_deinit_pipeline(context->device, pipeline);
}

sf_public struct sf_graphics_image *sf_graphics_init_image(struct sf_graphics_context *context, struct sf_graphics_init_image_info const *info) {
	if (!context || !info)
		return NULL;

	return sf_graphics_device_init_image(&context->arena, context->device, info);
}

sf_public void sf_graphics_deinit_image(struct sf_graphics_context *context, struct sf_graphics_image *image) {
	if (!context || !image)
		return;

	sf_graphics_device_deinit_image(context->device, image);
}

sf_public struct sf_graphics_buffer *sf_graphics_init_buffer(struct sf_graphics_context *context, struct sf_graphics_init_buffer_info const *info) {
	if (!context || !info)
		return NULL;

	return sf_graphics_device_init_buffer(&context->arena, context->device, info);
}

sf_public void sf_graphics_deinit_buffer(struct sf_graphics_context *context, struct sf_graphics_buffer *buffer) {
	if (!context || !buffer)
		return;

	sf_graphics_device_deinit_buffer(context->device, buffer);
}
