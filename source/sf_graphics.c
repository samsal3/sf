#include "sf_graphics.h"

#include <stdio.h>

#define SF_POW(b, p) pow(b, p)
#define SF_SNPRINTF snprintf

#define sf_log_error(...) fprintf(stderr, "\033[31m[ERROR]: \033[m" __VA_ARGS__)
#define sf_log_info(...) fprintf(stdout, "[INFO]:" __VA_ARGS__)
#define sf_log_warning(...) fprintf(stdout, "\033[31m[WARNING]: \033[m" __VA_ARGS__)
#define sf_log_verbose(...) fprintf(stdout, "[VERBOSE]:" __VA_ARGS__)

#define SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(name)                                               \
	SF_INTERNAL struct type *sf_graphics_get_or_allocate_##name##_from_resource_pool(                             \
	    struct sf_arena *arena, struct sf_graphics_resource_pool *pool) {                                         \
		struct sf_graphics_##name *result = NULL;                                                             \
		if (SF_QUEUE_IS_EMPTY(&pool->free_resource_queue)) {                                                  \
			result = sf_allocate(arena, sizeof(struct sf_graphics_##name));                               \
			if (!result)                                                                                  \
				return NULL;                                                                          \
		} else {                                                                                              \
			struct sf_queue *head = SF_QUEUE_HEAD(&pool->free_resource_queue);                            \
			SF_QUEUE_REMOVE(head);                                                                        \
			result = SF_QUEUE_DATA(head, struct sf_graphics_##name, queue);                               \
		}                                                                                                     \
		SF_MEMORY_SET(result, 0, sizeof(struct sf_graphics_##name));                                          \
		SF_QUEUE_INIT(&result->queue);                                                                        \
		SF_QUEUE_INSERT_HEAD(&pool->resource_queue, &result->queue);                                          \
		return result;                                                                                        \
	}

SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(texture)
SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(buffer)
SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(render_target)
SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(command_buffer)
SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION(pipeline)

#undef SF_GRAPHICS_DEFINE_RESOURCE_POOL_GET_OR_ALLOCATE_FUNCTION

SF_INTERNAL void sf_graphics_default_init_resource_pool(struct sf_graphics_resource_pool *pool) {
	SF_QUEUE_INIT(&pool->resource_queue);
	SF_QUEUE_INIT(&pool->free_resource_queue);
}

SF_INTERNAL char const *sf_graphics_vulkan_result_to_string(VkResult result) {
	switch (result) {
		case VK_SUCCESS:			return "VK_SUCCESS";
		case VK_NOT_READY:			return "VK_NOT_READY";
		case VK_TIMEOUT:			return "VK_TIMEOUT";
		case VK_EVENT_SET:			return "VK_EVENT_SET";
		case VK_EVENT_RESET:			return "VK_EVENT_RESET";
		case VK_INCOMPLETE:			return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:	return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:		return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:	return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:	return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:	return "VK_ERROR_EXTENSION_NO_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:	return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:	return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:		return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:	return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:		return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_OUT_OF_POOL_MEMORY:	return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:	return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_SURFACE_LOST_KHR:		return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:			return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:		return "VK_ERROR_OUT_OF_DATE_KHR";

		case VK_ERROR_UNKNOWN:
		default:	       return "VK_ERROR_UNKNOWN";
	}
}

static sf_bool sf_graphics_vulkan_check(VkResult vulkan_result, char const *what, int line, char const *file) {
	if (VK_SUCCESS != vulkan_result)
		sf_log_error(
		    "%s:%i - %s = %s\n", file, line, sf_graphics_vulkan_result_to_string(vulkan_result), what);
	else if (SF_FALSE)
		sf_log_info("%s:%i - %s = %s\n", file, line, sf_graphics_vulkan_result_to_string(vulkan_result), what);
	return VK_SUCCESS == vulkan_result;
}
#define SF_VULKAN_CHECK(e) sf_graphics_check((e), #e, __LINE__, __FILE__)

#define sf_log_error(...) fprintf(stderr, "\033[31m[ERROR]: \033[m" __VA_ARGS__)
#define sf_log_info(...) fprintf(stdout, "[INFO]:" __VA_ARGS__)
#define sf_log_warning(...) fprintf(stdout, "\033[31m[WARNING]: \033[m" __VA_ARGS__)
#define sf_log_verbose(...) fprintf(stdout, "[VERBOSE]:" __VA_ARGS__)

static void sf_graphics_vulkan_create_instance(struct sf_graphics_renderer	       *renderer,
					       struct sf_graphics_renderer_description *description) {
	VkApplicationInfo      application_info	    = {0};
	VkInstanceCreateInfo   info		    = {0};
	struct sf_arena	      *arena		    = &renderer->arena;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	application_info.sType		    = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pNext		    = NULL;
	application_info.pApplicationName   = description->application_name;
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.pEngineName	    = NULL;
	application_info.engineVersion	    = VK_MAKE_VERSION(1, 0, 0);
	application_info.apiVersion	    = VK_MAKE_VERSION(1, 0, 0);

	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
#ifdef __APPLE__
	info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
	info.pApplicationInfo	     = &application_info;
	info.enabledLayerCount	     = description->vk_instance_layer_count;
	info.ppEnabledLayerNames     = description->vk_instance_layers;
	info.enabledExtensionCount   = description->vk_instance_extension_count;
	info.ppEnabledExtensionNames = description->vk_instance_extensions;

	SF_VULKAN_CHECK(vkCreateInstance(&info, allocation_callbacks, &renderer->vk_instance));
}

#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)
static void sf_graphics_vulkan_proc_functions(struct sf_graphics_renderer *renderer) {
	VkInstance instance = renderer->vk_instance;

	if (!instance)
		return;

	renderer->vk_create_debug_utils_messenger_ext  = SF_VULKAN_PROC(vkCreateDebugUtilsMessengerEXT, instance);
	renderer->vk_destroy_debug_utils_messenger_ext = SF_VULKAN_PROC(vkDestroyDebugUtilsMessengerEXT, instance);
}

static VkBool32 VKAPI_CALL sf_graphics_vulkan_log(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
						  VkDebugUtilsMessageTypeFlagsEXT	      message_types,
						  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
						  void					     *user_data) {
	(void)message_types;
	(void)user_data;

	switch (message_severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			sf_log_verbose("%s\n", callback_data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: sf_log_info("%s\n", callback_data->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			sf_log_warning("%s\n", callback_data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			sf_log_error("%s\n", callback_data->pMessage);
			break;
		default: sf_log_info("%s\n", callback_data->pMessage); break;
	}

	return VK_TRUE;
}

static void sf_graphics_vulkan_create_validation_messenger(struct sf_graphics_renderer *renderer) {
	VkDebugUtilsMessengerCreateInfoEXT  info		 = {0};
	VkInstance			    instance		 = renderer->vk_instance;
	VkAllocationCallbacks		   *allocation_callbacks = renderer->vk_allocation_callbacks;
	PFN_vkCreateDebugUtilsMessengerEXT  create		 = renderer->vk_create_debug_utils_messenger_ext;
	PFN_vkDestroyDebugUtilsMessengerEXT destroy		 = renderer->vk_destroy_debug_utils_messenger_ext;

	if (!instance || !create || !destroy)
		return;

	info.sType	     = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.pNext	     = NULL;
	info.flags	     = 0;
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
	info.pUserData	     = NULL;

	SF_VULKAN_CHECK(create(instance, &info, allocation_callbacks, &renderer->vk_validation_messenger));
}

static void sf_graphics_vulkan_create_surface(struct sf_graphics_renderer	      *renderer,
					      struct sf_graphics_renderer_description *description) {
	description->create_vulkan_surface(renderer);
}

static sf_bool sf_graphics_vulkan_are_queue_family_indices_valid(struct sf_graphics_renderer const *renderer) {
	return (uint32_t)-1 != renderer->vk_graphics_queue_family_index &&
	       (uint32_t)-1 != renderer->vk_present_queue_family_index;
}

static void sf_graphics_vulkan_find_suitable_queue_family_indices(struct sf_arena	      *arena,
								  struct sf_graphics_renderer *renderer) {
	uint32_t		 i		= 0;
	uint32_t		 property_count = 0;
	VkQueueFamilyProperties *properties	= NULL;

	VkPhysicalDevice device = renderer->vk_physical_device;

	renderer->vk_graphics_queue_family_index = ((uint32_t)-1);
	renderer->vk_present_queue_family_index	 = ((uint32_t)-1);
	;

	if (!device)
		return;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &property_count, NULL);
	if (!property_count)
		return;

	if (!(properties = sf_allocate(arena, property_count * sizeof(*properties))))
		return;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &property_count, properties);
	for (i = 0; i < property_count && !sf_graphics_are_queue_family_indices_valid(renderer); ++i) {
		VkBool32 supports_surface = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, renderer->vk_surface, &supports_surface);
		if (supports_surface)
			renderer->vk_present_queue_family_index = i;

		if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			renderer->vk_graphics_queue_family_index = i;
	}
}

static sf_bool
sf_graphics_vulkan_check_physical_device_extension_support(struct sf_arena			   *arena,
							   struct sf_graphics_renderer const	   *renderer,
							   struct sf_graphics_renderer_description *description) {
	uint32_t	       i			 = 0;
	uint32_t	       available_extension_count = 0;
	VkExtensionProperties *available_extensions	 = NULL;
	VkPhysicalDevice       device			 = renderer->vk_physical_device;

	if (!device)
		return SF_FALSE;

	if (!SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, NULL)))
		return SF_FALSE;

	if (!(available_extensions = sf_allocate(arena, available_extension_count * sizeof(*available_extensions))))
		return SF_FALSE;

	if (!SF_VULKAN_CHECK(
		vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, available_extensions)))
		return SF_FALSE;

	for (i = 0; i < description->vk_device_extension_count; ++i) {
		uint32_t	 j		  = 0;
		sf_bool		 found_current	  = SF_FALSE;
		struct sf_string current_required = {0};
		sf_string_create_from_non_literal(description->vk_device_extensions[i], &current_required);

		for (j = 0, found_current = 0; j < available_extension_count && !found_current; ++j) {
			struct sf_string current_available = {0};
			sf_string_create_from_non_literal(available_extensions[j].extensionName, &current_available);

			found_current = sf_string_compare(
			    &current_required, &current_available, VK_MAX_EXTENSION_NAME_SIZE);
		}

		if (!found_current)
			return SF_FALSE;
	}

	return SF_TRUE;
}

static sf_bool
sf_graphics_vulkan_check_physical_device_swapchain_support(struct sf_graphics_renderer const *renderer) {
	uint32_t surface_format_count = 0, present_mode_count = 0;

	VkPhysicalDevice device	 = renderer->vk_physical_device;
	VkSurfaceKHR	 surface = renderer->vk_surface;

	if (!device || !surface)
		return SF_FALSE;

	SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surface_format_count, NULL));
	SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL));

	return surface_format_count && present_mode_count;
}

static sf_bool sf_graphics_vulkan_check_physical_device_support(struct sf_arena				*arena,
								struct sf_graphics_renderer		*renderer,
								struct sf_graphics_renderer_description *description) {
	sf_graphics_vulkan_find_suitable_queue_family_indices(arena, renderer);
	if (!sf_graphics_vulkan_are_queue_family_indices_valid(renderer))
		return SF_FALSE;

	if (!sf_graphics_vulkan_check_physical_device_swapchain_support(renderer))
		return SF_FALSE;

	if (!sf_graphics_vulkan_check_physical_device_extension_support(arena, renderer, description))
		return SF_FALSE;

	return SF_TRUE;
}

static void sf_graphics_vulkan_pick_physical_device(struct sf_graphics_renderer *renderer) {
	uint32_t i = 0;

	uint32_t	  physical_device_count = 0;
	VkPhysicalDevice *devices		= NULL;

	struct sf_arena *arena	  = &renderer->arena;
	VkInstance	 instance = renderer->vk_instance;

	if (!instance)
		return;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL)))
		return;

	if (!(devices = sf_allocate(arena, physical_device_count * sizeof(*devices))))
		return;

	if (!SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, devices)))
		return;

	for (i = 0; i < physical_device_count; ++i) {
		renderer->vk_physical_device = devices[i];
		if (sf_graphics_check_physical_device_support(arena, renderer))
			return;
	}

	renderer->vk_physical_device = VK_NULL_HANDLE;
}

static void sf_graphics_vulkan_create_device(struct sf_graphics_renderer	     *renderer,
					     struct sf_graphics_renderer_description *description) {
	VkDeviceCreateInfo	 info	       = {0};
	VkDeviceQueueCreateInfo	 queue_info[2] = {0};
	VkPhysicalDeviceFeatures features      = {0};
	float			 priority      = 1.0F;

	VkInstance	       instance		    = renderer->vk_instance;
	VkPhysicalDevice       physical_device	    = renderer->vk_physical_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!instance || !physical_device)
		return;

	vkGetPhysicalDeviceFeatures(renderer->vk_physical_device, &features);

	queue_info[0].sType	       = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].pNext	       = NULL;
	queue_info[0].flags	       = 0;
	queue_info[0].queueFamilyIndex = renderer->vk_graphics_queue_family_index;
	queue_info[0].queueCount       = 1;
	queue_info[0].pQueuePriorities = &priority;

	queue_info[1].sType	       = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[1].pNext	       = NULL;
	queue_info[1].flags	       = 0;
	queue_info[1].queueFamilyIndex = renderer->vk_present_queue_family_index;
	queue_info[1].queueCount       = 1;
	queue_info[1].pQueuePriorities = &priority;

	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;
	if (renderer->vk_graphics_queue_family_index == renderer->vk_present_queue_family_index)
		info.queueCreateInfoCount = 1;
	else
		info.queueCreateInfoCount = SF_SIZE(queue_info);
	info.pQueueCreateInfos	     = queue_info;
	info.enabledLayerCount	     = 0;
	info.ppEnabledLayerNames     = NULL;
	info.enabledExtensionCount   = description->vk_device_extension_count;
	info.ppEnabledExtensionNames = description->vk_device_extensions;
	info.pEnabledFeatures	     = &features;

	if (!SF_VULKAN_CHECK(vkCreateDevice(physical_device, &info, allocation_callbacks, &renderer->vk_device)))
		return;

	vkGetDeviceQueue(
	    renderer->vk_device, renderer->vk_graphics_queue_family_index, 0, &renderer->vk_graphics_queue);
	vkGetDeviceQueue(renderer->vk_device, renderer->vk_present_queue_family_index, 0, &renderer->vk_present_queue);
}

static void sf_graphics_vulkan_pick_surface_format(struct sf_graphics_renderer *renderer) {
	uint32_t i = 0;

	uint32_t	    format_count = 0;
	VkSurfaceFormatKHR *formats	 = NULL;

	struct sf_arena *arena = &renderer->arena;

	VkPhysicalDevice device	 = renderer->vk_physical_device;
	VkSurfaceKHR	 surface = renderer->vk_surface;

	if (!device || !surface)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL)))
		return;

	if (!(formats = sf_allocate(arena, format_count * sizeof(*formats))))
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, formats)))
		return;

	renderer->vk_surface_format	 = formats[0].format;
	renderer->vk_surface_color_space = formats[0].colorSpace;

	for (i = 0; i < format_count; ++i) {
		VkSurfaceFormatKHR *format = &formats[i];

		if (VK_FORMAT_B8G8R8A8_SRGB == format->format ||
		    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == format->colorSpace) {
			renderer->vk_surface_format	 = VK_FORMAT_B8G8R8A8_SRGB;
			renderer->vk_surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			return;
		}
	}
}

static void sf_graphics_vulkan_pick_present_mode(struct sf_graphics_renderer		 *renderer,
						 struct sf_graphics_renderer_description *description) {
	uint32_t i = 0;

	uint32_t	  present_mode_count = 0;
	VkPresentModeKHR *present_modes	     = NULL;

	struct sf_arena *arena	 = &renderer->arena;
	VkPhysicalDevice device	 = renderer->vk_physical_device;
	VkSurfaceKHR	 surface = renderer->vk_surface;

	if (!device || !surface)
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL)))
		return;

	if (!(present_modes = sf_allocate(arena, present_mode_count * sizeof(*present_modes))))
		return;

	if (!SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL)))
		return;

	if (description->enable_vsync)
		renderer->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	else
		renderer->vk_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	for (i = 0; i < present_mode_count; ++i)
		if (renderer->vk_present_mode == present_modes[i])
			return;

	renderer->vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
}

static sf_bool sf_graphics_vulkan_test_format_features(struct sf_graphics_renderer const *renderer, VkFormat format,
						       VkImageTiling tiling, VkFormatFeatureFlags features) {
	VkFormatProperties properties = {0};
	VkPhysicalDevice   device     = renderer->vk_physical_device;
	if (!device)
		return SF_FALSE;

	vkGetPhysicalDeviceFormatProperties(device, format, &properties);

	if (VK_IMAGE_TILING_LINEAR == tiling)
		return !!(properties.linearTilingFeatures & features);
	else if (VK_IMAGE_TILING_OPTIMAL == tiling)
		return !!(properties.optimalTilingFeatures & features);

	return SF_FALSE;
}

static void sf_graphics_vulkan_pick_depth_format(struct sf_graphics_renderer *renderer) {
	if (sf_graphics_vulkan_test_format_features(renderer, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
						    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		renderer->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT;
	else if (sf_graphics_vulkan_test_format_features(renderer, VK_FORMAT_D32_SFLOAT_S8_UINT,
							 VK_IMAGE_TILING_OPTIMAL,
							 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		renderer->vk_depth_stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	else if (sf_graphics_vulkan_test_format_features(renderer, VK_FORMAT_D24_UNORM_S8_UINT,
							 VK_IMAGE_TILING_OPTIMAL,
							 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		renderer->vk_depth_stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;
}

static void sf_graphics_vulkan_pick_sample_count(struct sf_graphics_renderer *renderer) {
	VkPhysicalDeviceProperties properties	 = {0};
	VkSampleCountFlags	   sample_counts = {0};

	VkPhysicalDevice device = renderer->vk_physical_device;
	if (!device)
		return;

	vkGetPhysicalDeviceProperties(device, &properties);
	sample_counts = properties.limits.framebufferColorSampleCounts &
			properties.limits.framebufferDepthSampleCounts;

	if (0)
		(void)0;
	// else if (sample_counts & VK_SAMPLE_COUNT_64_BIT) renderer->sample_count =  VK_SAMPLE_COUNT_64_BIT;
	// else if (sample_counts & VK_SAMPLE_COUNT_32_BIT) renderer->sample_count =  VK_SAMPLE_COUNT_32_BIT;
	// else if (sample_counts & VK_SAMPLE_COUNT_16_BIT) renderer->sample_count =  VK_SAMPLE_COUNT_16_BIT;
	// else if (sample_counts & VK_SAMPLE_COUNT_8_BIT) renderer->sample_count =  VK_SAMPLE_COUNT_8_BIT;
	// else if (sample_counts & VK_SAMPLE_COUNT_4_BIT) renderer->sample_count =  VK_SAMPLE_COUNT_4_BIT;
	else if (sample_counts & VK_SAMPLE_COUNT_2_BIT)
		renderer->vk_samples = VK_SAMPLE_COUNT_2_BIT;
	// else if (sample_counts & VK_SAMPLE_COUNT_1_BIT) renderer->sample_count =  VK_SAMPLE_COUNT_1_BIT;
}

static uint32_t sf_graphics_vulkan_find_device_memory_type_index(VkPhysicalDevice      device,
								 VkMemoryPropertyFlags memory_properties,
								 uint32_t	       filter) {
	uint32_t			 i	   = 0;
	VkPhysicalDeviceMemoryProperties available = {0};

	if (!device)
		return (uint32_t)-1;

	vkGetPhysicalDeviceMemoryProperties(device, &available);

	for (i = 0; i < available.memoryTypeCount; ++i)
		if ((filter & (1 << i)) &&
		    (available.memoryTypes[i].propertyFlags & memory_properties) == memory_properties)
			return i;

	return (uint32_t)-1;
}

static VkDeviceMemory sf_graphics_vulkan_allocate_memory(struct sf_graphics_renderer *renderer,
							 VkMemoryPropertyFlags memory_properties, uint32_t filter,
							 uint64_t size) {
	VkMemoryAllocateInfo info = {0};

	VkDeviceMemory memory		 = VK_NULL_HANDLE;
	uint32_t       memory_type_index = (uint32_t)-1;

	VkPhysicalDevice       physical_device	    = renderer->vk_physical_device;
	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!physical_device || !device)
		return VK_NULL_HANDLE;

	memory_type_index = sf_graphics_vulkan_find_device_memory_type_index(
	    physical_device, memory_properties, filter);
	if ((uint32_t)-1 == memory_type_index)
		return VK_NULL_HANDLE;

	info.sType	     = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.pNext	     = NULL;
	info.allocationSize  = size;
	info.memoryTypeIndex = memory_type_index;

	if (!SF_VULKAN_CHECK(vkAllocateMemory(device, &info, allocation_callbacks, &memory)))
		return VK_NULL_HANDLE;

	return memory;
}

static VkDeviceMemory sf_graphics_vulkan_allocate_and_bind_memory_for_image(struct sf_graphics_renderer *renderer,
									    VkImage			 image,
									    VkMemoryPropertyFlags	 properties) {
	VkMemoryRequirements requirements = {0};
	VkDeviceMemory	     memory	  = VK_NULL_HANDLE;

	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device)
		return VK_NULL_HANDLE;

	vkGetImageMemoryRequirements(device, image, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(
	    renderer, properties, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindImageMemory(device, image, memory, 0))) {
		vkFreeMemory(device, memory, allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}

static VkDeviceMemory sf_graphics_vulkan_allocate_and_bind_memory_for_buffer(struct sf_graphics_renderer *renderer,
									     VkBuffer			  buffer,
									     VkMemoryPropertyFlags	  properties) {
	VkMemoryRequirements requirements = {0};
	VkDeviceMemory	     memory	  = VK_NULL_HANDLE;

	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device)
		return VK_NULL_HANDLE;

	vkGetBufferMemoryRequirements(device, buffer, &requirements);
	memory = sf_graphics_vulkan_allocate_memory(
	    renderer, properties, requirements.memoryTypeBits, requirements.size);
	if (!memory)
		return VK_NULL_HANDLE;

	if (!SF_VULKAN_CHECK(vkBindBufferMemory(device, buffer, memory, 0))) {
		vkFreeMemory(device, memory, allocation_callbacks);
		return VK_NULL_HANDLE;
	}

	return memory;
}

SF_INTERNAL VkImageViewType sf_graphics_vulkan_get_image_view_type(enum sf_graphics_texture_type type) {
	switch (type) {
		case SF_GRAPHICS_TEXTURE_TYPE_1D:   return VK_IMAGE_VIEW_TYPE_1D;
		case SF_GRAPHICS_TEXTURE_TYPE_2D:   return VK_IMAGE_VIEW_TYPE_2D;
		case SF_GRAPHICS_TEXTURE_TYPE_3D:   return VK_IMAGE_VIEW_TYPE_3D;
		case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
	}
}

SF_INTERNAL VkImageType sf_graphics_vulkan_get_image_type(enum sf_graphics_texture_type type) {
	switch (type) {
		case SF_GRAPHICS_TEXTURE_TYPE_1D:   return VK_IMAGE_TYPE_1D;
		case SF_GRAPHICS_TEXTURE_TYPE_2D:   return VK_IMAGE_TYPE_2D;
		case SF_GRAPHICS_TEXTURE_TYPE_3D:   return VK_IMAGE_TYPE_3D;
		case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_TYPE_2D;
	}
}

SF_INTERNAL VkFormat sf_graphics_vulkan_get_format(enum sf_graphics_texture_type type) {
	switch (type) {
		// 1 channel
		case SF_GRAPHICS_FORMAT_R8_UNORM:	     return VK_FORMAT_R8_UNORM;
		case SF_GRAPHICS_FORMAT_R16_UNORM:	     return VK_FORMAT_R16_UNORM;
		case SF_GRAPHICS_FORMAT_R16_UINT:	     return VK_FORMAT_R16_UINT;
		case SF_GRAPHICS_FORMAT_R16_SFLOAT:	     return VK_FORMAT_R16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32_UINT:	     return VK_FORMAT_R32_UINT;
		case SF_GRAPHICS_FORMAT_R32_SFLOAT:	     return VK_FORMAT_R32_SFLOAT;
		// 2 channel
		case SF_GRAPHICS_FORMAT_R8G8_UNORM:	     return VK_FORMAT_R8G8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16_UNORM:	     return VK_FORMAT_R16G16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:	     return VK_FORMAT_R16G16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32_UINT:	     return VK_FORMAT_R32G32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:	     return VK_FORMAT_R32G32_SFLOAT;
		// 3 channel
		case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:	     return VK_FORMAT_R8G8B8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:     return VK_FORMAT_R16G16B16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:    return VK_FORMAT_R16G16B16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32B32_UINT:	     return VK_FORMAT_R32G32B32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
		// 4 channel
		case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:	     return VK_FORMAT_B8G8R8A8_UNORM;
		case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:	     return VK_FORMAT_R8G8B8A8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:  return VK_FORMAT_R16G16B16A16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:   return VK_FORMAT_R32G32B32A32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		// Depth/stencil
		case SF_GRAPHICS_FORMAT_D16_UNORM:	     return VK_FORMAT_D16_UNORM;
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: return VK_FORMAT_X8_D24_UNORM_PACK32;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT:	     return VK_FORMAT_D32_SFLOAT;
		case SF_GRAPHICS_FORMAT_S8_UINT:	     return VK_FORMAT_S8_UINT;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:   return VK_FORMAT_D16_UNORM_S8_UINT;
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:   return VK_FORMAT_D24_UNORM_S8_UINT;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
}

SF_INTERNAL VkImageAspectFlags sf_graphics_vulkan_get_image_aspect_flags(enum sf_graphics_format format) {
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
		case SF_GRAPHICS_FORMAT_D32_SFLOAT:	     return VK_IMAGE_ASPECT_DEPTH_BIT;
		case SF_GRAPHICS_FORMAT_S8_UINT:	     return VK_IMAGE_ASPECT_STENCIL_BIT;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default: return 0;
	}
}

SF_INTERNAL VkSampleCountFlags sf_graphics_vulkan_get_sample_count(enum sf_graphics_sample_count samples) {
	switch (samples) {
		case SF_GRAPHICS_SAMPLE_COUNT_1:  return VK_SAMPLE_COUNT_1_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_2:  return VK_SAMPLE_COUNT_2_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_4:  return VK_SAMPLE_COUNT_4_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_8:  return VK_SAMPLE_COUNT_8_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_16: return VK_SAMPLE_COUNT_16_BIT;
	}
}

SF_INTERNAL VkImageUsageFlags sf_graphics_vulkan_get_image_usage_flags(sf_graphics_texture_usage_flags usage) {
	VkImageUsageFlags result = 0;
	if (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC) {
		result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST) {
		result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (usage & SF_GRAPHICS_TEXTURE_USAGE_SAMPLED) {
		result |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			  VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
SF_INTERNAL void sf_graphics_vulkan_create_texture_image_view(struct sf_graphics_renderer *renderer,
							      struct sf_graphics_texture  *texture) {
	VkImageViewCreateInfo info = {0};

	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device)
		return;

	info.sType			     = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext			     = NULL;
	info.flags			     = 0;
	info.image			     = texture->vk_image;
	info.viewType			     = sf_graphics_vulkan_get_image_view_type(texture->type);
	info.format			     = sf_graphics_vulkan_get_format(texture->format);
	info.components.r		     = VK_COMPONENT_SWIZZLE_R;
	info.components.g		     = VK_COMPONENT_SWIZZLE_G;
	info.components.b		     = VK_COMPONENT_SWIZZLE_B;
	info.components.a		     = VK_COMPONENT_SWIZZLE_A;
	info.subresourceRange.aspectMask     = sf_graphics_vulkan_get_image_aspect_flags(texture->format);
	info.subresourceRange.baseMipLevel   = 0;
	info.subresourceRange.levelCount     = texture->mips;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount     = 1;

	SF_VULKAN_CHECK(vkCreateImageView(device, &info, allocation_callbacks, &texture->vk_image_view));
}

SF_INTERNAL void sf_graphics_vulkan_create_texture_image(struct sf_graphics_renderer *renderer,
							 struct sf_graphics_texture  *texture) {
	VkImageCreateInfo info = {0};

	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device)
		return;

	info.sType		   = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext		   = NULL;
	info.flags		   = 0;
	info.imageType		   = sf_graphics_vulkan_get_image_type(texture->type);
	info.format		   = sf_graphics_vulkan_get_format(texture->format);
	info.extent.width	   = texture->width;
	info.extent.height	   = texture->height;
	info.extent.depth	   = texture->depth;
	info.mipLevels		   = texture->mips;
	info.arrayLayers	   = 1;
	info.samples		   = sf_graphics_vulkan_get_sample_count(texture->samples);
	info.tiling		   = texture->mapped ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
	info.usage		   = sf_graphics_vulkan_get_image_usage_flags(texture->usage);
	info.sharingMode	   = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices   = NULL;
	info.initialLayout	   = VK_IMAGE_LAYOUT_UNDEFINED;

	SF_VULKAN_CHECK(vkCreateImage(device, &info, allocation_callbacks, &texture->vk_image));
}

SF_INTERNAL void sf_graphics_vulkan_allocate_texture_memory(struct sf_graphics_renderer *renderer,
							    struct sf_graphics_texture	*texture) {
	VkMemoryRequirements  requirements = {0};
	VkMemoryPropertyFlags properties   = 0;

	VkImage		       image		    = texture->vk_image;
	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device || !image)
		return VK_NULL_HANDLE;

	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	if (texture->mapped)
		properties |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	vkGetImageMemoryRequirements(device, image, &requirements);
	texture->vk_memory = sf_graphics_vulkan_allocate_memory(
	    renderer, properties, requirements.memoryTypeBits, requirements.size);
	if (!texture->vk_memory)
		goto error;

	if (!SF_VULKAN_CHECK(vkBindImageMemory(device, image, texture->vk_memory, 0)))
		goto error;

	if (texture->mapped &&
	    !SF_VULKAN_CHECK(vkMapMemory(device, texture->vk_memory, image, VK_WHOLE_SIZE, 0, &texture->mapped_data)))
		goto error;

	return;

error:
	if (texture->vk_memory) {
		vkFreeMemory(device, texture->vk_memory, allocation_callbacks);
		texture->vk_memory = VK_NULL_HANDLE;
	}
}

SF_EXTERNAL void sf_graphics_destroy_texture(struct sf_graphics_renderer *renderer, sf_handle texture_handle) {
	struct sf_graphics_texture *texture = (struct sf_graphics_texture *)texture_handle;

	if (!renderer || !texture)
		return;

	if (renderer->vk_device) {
		VkDevice	       device		    = renderer->vk_device;
		VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

		SF_QUEUE_REMOVE(&texture->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->texture_pool.free_resource_queue, &texture->queue);

		if (texture->vk_image_view) {
			vkDestroyImageView(device, texture->vk_image_view, allocation_callbacks);
			texture->vk_image_view = VK_NULL_HANDLE;
		}

		if (texture->vk_owns_image_and_memory) {
			if (texture->vk_memory) {
				vkFreeMemory(device, texture->vk_memory, allocation_callbacks);
				texture->vk_memory = VK_NULL_HANDLE;
			}
			if (texture->vk_image) {
				vkDestroyImage(device, texture->vk_image, allocation_callbacks);
				texture->vk_image = VK_NULL_HANDLE;
			}
		}
	}
}

SF_INTERNAL sf_handle sf_graphics_vulkan_create_texture(
    struct sf_graphics_renderer *renderer, enum sf_graphics_texture_type type, enum sf_graphics_format format,
    enum sf_graphics_sample_count samples, enum sf_graphics_texture_usage usage, uint32_t width, uint32_t height,
    uint32_t depth, uint32_t mips, sf_bool mapped, struct sf_graphics_clear_value *clear_value,
    VkImage vk_not_owned_image) {
	struct sf_graphics_texture *texture = NULL;

	if (!renderer)
		return SF_NULL_HANDLE;

	texture = sf_graphics_get_or_allocate_texture_from_resource_pool(&renderer->arena, &renderer->texture_pool);
	if (!texture)
		return SF_NULL_HANDLE;

	texture->type	 = type;
	texture->format	 = format;
	texture->samples = samples;
	texture->usage	 = usage;
	texture->width	 = width;
	texture->height	 = height;
	texture->depth	 = depth;
	texture->mips	 = mips;
	texture->mapped	 = mapped;
	if (clear_value)
		texture->clear_value = *clear_value;
	else
		texture->clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;

	if (!vk_not_owned_image) {
		texture->vk_owns_image_and_memory = SF_TRUE;
		sf_graphics_vulkan_create_texture_image(renderer, texture);
		if (!texture->vk_image)
			goto error;

		sf_graphics_vulkan_allocate_texture_memory(renderer, texture);
		if (!texture->vk_memory)
			goto error;
	} else {
		texture->vk_owns_image_and_memory = SF_FALSE;
		texture->vk_image		  = vk_not_owned_image;
		texture->mapped			  = SF_FALSE;
	}

	sf_graphics_vulkan_create_texture_image_view(renderer, texture);
	if (!texture->vk_image_view)
		goto error;

	return SF_AS_HANDLE(texture);

error:
	sf_graphics_destroy_texture(renderer, SF_AS_HANDLE(texture));
	return SF_NULL_HANDLE;
}

SF_EXTERNAL sf_handle sf_graphics_create_texture(struct sf_graphics_renderer  *renderer,
						 enum sf_graphics_texture_type type, enum sf_graphics_format format,
						 enum sf_graphics_sample_count	samples,
						 enum sf_graphics_texture_usage usage, uint32_t width, uint32_t height,
						 uint32_t depth, uint32_t mips, sf_bool mapped,
						 struct sf_graphics_clear_value *clear_value) {
	return sf_graphics_vulkan_create_texture(
	    renderer, type, format, samples, usage, width, height, depth, mips, mapped, clear_value, VK_NULL_HANDLE);
}

SF_INTERNAL void sf_graphics_vulkan_create_render_target_render_pass(struct sf_graphics_renderer      *renderer,
								     struct sf_graphics_render_target *render_target) {
	uint32_t		 i		      = 0;
	uint32_t		 description_count    = 0;
	VkAttachmentDescription *descriptions	      = NULL;
	VkAttachmentReference	*color_refs	      = NULL;
	VkAttachmentReference	*resolve_refs	      = NULL;
	VkAttachmentReference	 depth_stencil_ref    = {0};
	VkSubpassDescription	 subpass	      = {0};
	VkSubpassDependency	 dependency	      = {0};
	VkRenderPassCreateInfo	 info		      = {0};
	struct sf_arena		*arena		      = &renderer->render_target_arena;
	VkDevice		 device		      = renderer->vk_device;
	VkAllocationCallbacks	*allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device)
		return;

	description_count = render_target->color_attachment_count + render_target->resolve_attachment_count +
			    !!render_target->depth_stencil_attachment;

	descriptions = sf_allocate(arena, description_count * sizeof(*descriptions));
	if (!descriptions)
		goto error;

	color_refs = sf_allocate(arena, render_target->color_attachment_count * sizeof(*color_refs));
	if (!color_refs)
		goto error;

	resolve_refs = sf_allocate(arena, render_target->resolve_attachment_count * sizeof(*resolve_refs));
	if (render_target->resolve_attachment_count && !resolve_refs)
		goto error;

	for (i = 0; i < render_target->color_attachment_count; ++i) {
		struct sf_graphics_texture *attachment = NULL;

		uint32_t		 description_index = i * (!!render_target->resolve_attachment_count) * 2;
		VkAttachmentDescription *description	   = &descriptions[description_index];
		VkAttachmentReference	*ref		   = &color_refs[i];

		attachment = (struct sf_graphics_texture *)render_target->color_attachments[i];

		description->flags	    = 0;
		description->format	    = sf_graphics_vulkan_get_format(attachment->format);
		description->samples	    = VK_SAMPLE_COUNT_1_BIT;
		description->loadOp	    = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description->storeOp	    = VK_ATTACHMENT_STORE_OP_STORE;
		description->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		description->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		if (attachment->vk_owns_image_and_memory)
			description->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else
			description->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		ref->attachment = description_index;
		ref->layout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	for (i = 0; i < render_target->resolve_attachment_count; ++i) {
		struct sf_graphics_texture *attachment = NULL;

		uint32_t		 description_index = i * 2 + 1;
		VkAttachmentDescription *description	   = &descriptions[description_index];
		VkAttachmentReference	*ref		   = &resolve_refs[i];

		attachment = (struct sf_graphics_texture *)render_target->resolve_attachments[i];

		description->flags	    = 0;
		description->format	    = sf_graphics_vulkan_get_format(attachment->format);
		description->samples	    = sf_graphics_vulkan_get_sample_count(attachment->samples);
		description->loadOp	    = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description->storeOp	    = VK_ATTACHMENT_STORE_OP_STORE;
		description->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		description->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		description->finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		ref->attachment = description_index;
		ref->layout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (render_target->depth_stencil_attachment) {
		struct sf_graphics_texture *attachment = NULL;

		uint32_t		 description_index = description_count - 1;
		VkAttachmentDescription *description	   = &descriptions[description_index];
		VkAttachmentReference	*ref		   = &depth_stencil_ref;

		attachment = (struct sf_graphics_texture *)render_target->depth_stencil_attachment;

		description->flags	    = 0;
		description->format	    = sf_graphics_vulkan_get_format(attachment->format);
		description->samples	    = sf_graphics_vulkan_get_sample_count(attachment->samples);
		description->loadOp	    = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description->storeOp	    = VK_ATTACHMENT_STORE_OP_STORE;
		description->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		description->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		description->finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		ref->attachment = g;
		ref->layout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	subpass.flags		     = 0;
	subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments    = NULL;
	subpass.colorAttachmentCount = render_target->color_attachment_count;
	subpass.pColorAttachments    = color_refs;
	subpass.pResolveAttachments  = resolve_refs;
	if (render_target->depth_stencil_attachment)
		subpass.pDepthStencilAttachment = &depth_stencil_ref;
	else
		subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments	= NULL;

	dependency.srcSubpass	   = 0;
	dependency.dstSubpass	   = 0;
	dependency.srcStageMask	   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask	   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	info.sType	     = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.pNext	     = NULL;
	info.flags	     = 0;
	info.attachmentCount = description_count;
	info.pAttachments    = descriptions;
	info.subpassCount    = 1;
	info.pSubpasses	     = &subpass;
	info.dependencyCount = 1;
	info.pDependencies   = &dependency;

	SF_VULKAN_CHECK(vkCreateRenderPass(device, &info, allocation_callbacks, &render_target->vk_render_pass));

error:
	sf_clear_arena(arena);
}

SF_INTERNAL void sf_graphics_vulkan_create_render_target_framebuffer(struct sf_graphics_renderer      *renderer,
								     struct sf_graphics_render_target *render_target) {
	uint32_t		i							    = 0;
	VkFramebufferCreateInfo info							    = {0};
	VkImageView		attachments[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT] = {0};

	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device)
		return;

	for (i = 0; i < render_target->color_attachment_count; ++i) {
		uint32_t attachment_index = i * (!!render_target->resolve_attachment_count) * 2;

		struct sf_graphics_texture *attachment = (struct sf_graphics_texture *)
							     render_target->color_attachments[i];

		attachments[attachment_index] = attachment->vk_image_view;
	}

	for (i = 0; i < render_target->resolve_attachment_count; ++i) {
		uint32_t attachment_index = i * 2 + 1;

		struct sf_graphics_texture *attachment = (struct sf_graphics_texture *)
							     render_target->resolve_attachments[i];

		attachments[attachment_index] = attachment->vk_image_view;
	}

	if (render_target->depth_stencil_attachment) {
		uint32_t attachment_index = render_target->total_attachment_count - 1;

		struct sf_graphics_texture *attachment = (struct sf_graphics_texture *)
							     render_target->depth_stencil_attachment;
		attachments[attachment_index] = attachment->vk_image_view;
	}

	info.sType	     = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext	     = NULL;
	info.flags	     = 0;
	info.renderPass	     = render_target->vk_render_pass;
	info.attachmentCount = render_target->total_attachment_count;
	info.pAttachments    = attachments;
	info.width	     = render_target->width;
	info.height	     = render_target->height;
	info.layers	     = 1;

	SF_VULKAN_CHECK(vkCreateFramebuffer(device, &info, allocation_callbacks, &render_target->vk_framebuffer));
}

SF_EXTERNAL void sf_graphics_destroy_render_target(struct sf_graphics_renderer *renderer,
						   sf_handle			render_target_handle) {
	uint32_t			  i		= 0;
	struct sf_graphics_render_target *render_target = (struct sf_graphics_render_target *)render_target_handle;

	if (!renderer || !render_target)
		return;

	SF_QUEUE_REMOVE(&render_target->queue);
	SF_QUEUE_INSERT_HEAD(&renderer->render_target_pool.free_resource_queue, &render_target->queue);

	if (renderer->vk_device) {
		VkDevice	       device		    = renderer->vk_device;
		VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

		if (render_target->vk_framebuffer) {
			vkDestroyFramebuffer(device, render_target->vk_framebuffer, allocation_callbacks);
			render_target->vk_framebuffer = VK_NULL_HANDLE;
		}

		if (render_target->vk_render_pass) {
			vkDestroyRenderPass(device, render_target->vk_render_pass, allocation_callbacks);
			render_target->vk_render_pass = VK_NULL_HANDLE;
		}
	}

	for (i = 0; i < render_target->color_attachment_count; ++i) {
		sf_graphics_destroy_texture(renderer, render_target->color_attachments[i]);
		render_target->color_attachments[i] = SF_NULL_HANDLE;
	}
	render_target->color_attachment_count = 0;

	for (i = 0; i < render_target->resolve_attachment_count; ++i) {
		sf_graphics_destroy_texture(renderer, render_target->resolve_attachments[i]);
		render_target->resolve_attachments[i] = SF_NULL_HANDLE;
	}
	render_target->resolve_attachment_count = 0;

	sf_graphics_destroy_texture(renderer, render_target->depth_stencil_attachment);
	render_target->depth_stencil_attachment = SF_NULL_HANDLE;
}

SF_INTERNAL sf_handle sf_graphics_vulkan_create_render_target(
    struct sf_graphics_renderer *renderer, uint32_t width, uint32_t height, enum sf_graphics_sample_count samples,
    enum sf_graphics_format color_format, enum sf_graphics_format depth_stencil_format,
    uint32_t color_attachment_count, struct sf_graphics_clear_value *color_clear_values,
    struct sf_graphics_clear_value *depth_stencil_clear_value, VkImage vk_swapchain_image) {
	uint32_t			  i		= 0;
	struct sf_graphics_render_target *render_target = NULL;

	if (!renderer)
		return SF_NULL_HANDLE;

	render_target = sf_graphics_get_or_allocate_render_target_from_resource_pool(
	    &renderer->arena, &renderer->render_target_pool);

	if (!render_target)
		return SF_NULL_HANDLE;

	render_target->samples			= samples;
	render_target->width			= width;
	render_target->height			= height;
	render_target->resolve_attachment_count = SF_GRAPHICS_SAMPLE_COUNT_1 != samples ? color_attachment_count : 0;
	render_target->color_attachment_count	= color_attachment_count;

	render_target->total_attachment_count = render_target->color_attachment_count +
						render_target->resolve_attachment_count +
						(SF_GRAPHICS_FORMAT_UNDEFINED != depth_stencil_format);

	for (i = 0; i < render_target->color_attachment_count; ++i) {
		struct sf_graphics_clear_value *clear_value = color_clear_values ? &color_clear_values[i] : NULL;

		render_target->color_attachments[i] = sf_graphics_vulkan_create_texture(
		    renderer, SF_GRAPHICS_TEXTURE_TYPE_2D, color_format, SF_GRAPHICS_SAMPLE_COUNT_1,
		    SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT, width, height, 1, 1, SF_FALSE, clear_value,
		    vk_swapchain_image);
		if (!render_target->color_attachments[i])
			goto error;
	}

	for (i = 0; i < render_target->resolve_attachment_count; ++i) {
		struct sf_graphics_clear_value *clear_value = color_clear_values ? &color_clear_values[i] : NULL;

		render_target->resolve_attachments[i] = sf_graphics_create_texture(
		    renderer, SF_GRAPHICS_TEXTURE_TYPE_2D, color_format, samples,
		    SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT, width, height, 1, 1, SF_FALSE, clear_value);
		if (!render_target->resolve_attachments[i])
			goto error;
	}

	if (SF_GRAPHICS_FORMAT_UNDEFINED != depth_stencil_format) {
		render_target->depth_stencil_attachment = sf_graphics_create_texture(
		    renderer, SF_GRAPHICS_TEXTURE_TYPE_2D, depth_stencil_format, samples,
		    SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT, width, height, 1, 1, SF_FALSE,
		    depth_stencil_clear_value);
		if (!render_target->depth_stencil_attachment)
			goto error;
	}

	sf_graphics_vulkan_create_render_target_render_pass(renderer, render_target);
	if (!render_target->vk_render_pass)
		goto error;

	sf_graphics_vulkan_create_render_target_framebuffer(renderer, render_target);
	if (!render_target->vk_framebuffer)
		goto error;

	return SF_AS_HANDLE(render_target);

error:
	sf_graphics_destroy_render_target(renderer, SF_AS_HANDLE(render_target));
	return SF_NULL_HANDLE;
}

SF_EXTERNAL sf_handle sf_graphics_create_render_target(struct sf_graphics_renderer *renderer, uint32_t width,
						       uint32_t height, enum sf_graphics_sample_count samples,
						       enum sf_graphics_format	       color_format,
						       enum sf_graphics_format	       depth_stencil_format,
						       uint32_t			       color_attachment_count,
						       struct sf_graphics_clear_value *color_clear_values,
						       struct sf_graphics_clear_value *depth_stencil_clear_value) {
	return sf_graphics_vulkan_create_render_target(renderer, width, height, samples, color_format,
						       depth_stencil_format, color_attachment_count,
						       color_clear_values, depth_stencil_clear_value, VK_NULL_HANDLE);
}

static void sf_graphics_vulkan_destroy_swapchain_resources(struct sf_graphics_renderer *renderer) {
	uint32_t	       i		    = 0;
	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	if (!device)
		return;

	for (i = 0; i < renderer->swapchain_render_target_count; ++i) {
		sf_graphics_destroy_render_target(renderer, renderer->swapchain_render_targets[i]);
		renderer->swapchain_render_targets[i] = SF_NULL_HANDLE;
	}
	renderer->swapchain_render_target_count = 0;

	for (i = 0; i < SF_SIZE(renderer->vk_swapchain_images); ++i)
		renderer->vk_swapchain_images[i] = VK_NULL_HANDLE;

	renderer->vk_swapchain_image_count = 0;

	if (renderer->vk_swapchain) {
		vkDestroySwapchainKHR(device, renderer->vk_swapchain, allocation_callbacks);
		renderer->vk_swapchain = VK_NULL_HANDLE;
	}
}

static void sf_graphics_vulkan_create_swapchain(struct sf_graphics_renderer *renderer) {
	VkSurfaceCapabilitiesKHR capabilities = {0};
	VkSwapchainCreateInfoKHR info	      = {0};

	VkPhysicalDevice       physical_device	    = renderer->vk_physical_device;
	VkSurfaceKHR	       surface		    = renderer->vk_surface;
	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	uint32_t queue_family_indices[] = {
	    renderer->vk_graphics_queue_family_index, renderer->vk_present_queue_family_index};

	if (!physical_device || !surface || !device)
		return;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

	info.sType		= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.pNext		= NULL;
	info.flags		= 0;
	info.surface		= renderer->vk_surface;
	info.minImageCount	= renderer->swapchain_requested_image_count;
	info.imageFormat	= renderer->vk_surface_format;
	info.imageColorSpace	= renderer->vk_surface_color_space;
	info.imageExtent.width	= renderer->swapchain_width;
	info.imageExtent.height = renderer->swapchain_height;
	info.imageArrayLayers	= 1;
	info.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (renderer->vk_graphics_queue_family_index == renderer->vk_present_queue_family_index) {
		info.imageSharingMode	   = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 1;
		info.pQueueFamilyIndices   = queue_family_indices;
	} else {
		info.imageSharingMode	   = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = SF_SIZE(queue_family_indices);
		info.pQueueFamilyIndices   = queue_family_indices;
	}
	info.preTransform   = capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode    = renderer->vk_present_mode;
	info.clipped	    = VK_TRUE;
	info.oldSwapchain   = VK_NULL_HANDLE;

	SF_VULKAN_CHECK(vkCreateSwapchainKHR(device, &info, allocation_callbacks, &renderer->vk_swapchain));
}

SF_INTERNAL void sf_graphics_vulkan_load_swapchain_images(struct sf_graphics_renderer *renderer) {
	uint32_t swapchain_image_count = 0;
	VkDevice device		       = renderer->vk_device;

	if (!device)
		return;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(device, renderer->vk_swapchain, &swapchain_image_count, NULL)))
		return;

	if (SF_SIZE(renderer->vk_swapchain_images) < swapchain_image_count)
		return;

	if (!SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(
		device, renderer->vk_swapchain, &swapchain_image_count, renderer->vk_swapchain_images)))
		return;

	renderer->vk_swapchain_image_count = swapchain_image_count;
}

SF_INTERNAL void sf_graphics_vulkan_create_swapchain_render_targets(struct sf_graphics_renderer *renderer) {
	uint32_t i = 0;

	for (i = 0; i < renderer->vk_swapchain_image_count; ++i) {
		renderer->swapchain_render_targets[i] = sf_graphics_vulkan_create_render_target(
		    renderer, renderer->swapchain_width, renderer->swapchain_height, renderer->swapchain_sample_count,
		    renderer->swapchain_color_format, renderer->swapchain_depth_stencil_format, 1,
		    &renderer->swapchain_color_clear_value, &renderer->swapchain_depth_stencil_clear_value,
		    &renderer->vk_swapchain_images[i]);
	}

	renderer->swapchain_render_target_count = renderer->vk_swapchain_image_count;
}

static void sf_graphics_vulkan_create_swapchain_resources(struct sf_graphics_renderer *renderer) {
	sf_graphics_vulkan_create_swapchain(renderer);
	if (!renderer->vk_swapchain)
		goto error;

	sf_graphics_vulkan_load_swapchain_images(renderer);
	if (!renderer->vk_swapchain_image_count)
		goto error;

	sf_graphics_vulkan_create_swapchain_render_targets(renderer);
	if (!renderer->swapchain_render_target_count)
		goto error;

	return;

error:
	sf_graphics_vulkan_destroy_swapchain_resources(renderer);
}

SF_EXTERNAL void sf_graphics_destroy_command_buffer(struct sf_graphics_renderer *renderer,
						    sf_handle			 command_buffer_handle) {
	struct sf_graphics_command_buffer *command_buffer = (struct sf_graphics_command_buffer *)command_buffer_handle;

	if (!renderer || !command_buffer)
		return;

	SF_QUEUE_REMOVE(&command_buffer->queue);
	SF_QUEUE_INSERT_HEAD(&renderer->command_buffer_pool.free_resource_queue, &command_buffer->queue);

	if (renderer->vk_device) {
		VkDevice	       device		    = renderer->vk_device;
		VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

		if (command_buffer->vk_command_pool) {
			vkFreeCommandBuffers(
			    device, command_buffer->vk_command_pool, 1, &command_buffer->vk_command_buffer);
			command_buffer->vk_command_buffer = VK_NULL_HANDLE;

			vkDestroyCommandPool(device, command_buffer->vk_command_pool, allocation_callbacks);
			command_buffer->vk_command_pool = VK_NULL_HANDLE;
		}
	}
}

SF_EXTERNAL sf_handle sf_graphics_create_command_buffer(struct sf_graphics_renderer *renderer, sf_bool transient) {
	VkCommandPoolCreateInfo	    command_pool_info	= {0};
	VkCommandBufferAllocateInfo command_buffer_info = {0};

	struct sf_graphics_command_buffer *command_buffer = NULL;

	if (!renderer || !renderer->vk_device)
		return SF_NULL_HANDLE;

	command_buffer = sf_graphics_get_or_allocate_command_buffer_from_resource_pool(
	    renderer, &renderer->command_buffer_pool);
	if (!command_buffer)
		return SF_NULL_HANDLE;

	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = NULL;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (transient)
		command_pool_info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	// FIXME(samuel): for now manually set the graphics queue as the family index. Later we should pass the
	// requried queue handle
	command_pool_info.queueFamilyIndex = renderer->vk_graphics_queue_family_index;

	if (!SF_VULKAN_CHECK(vkCreateCommandPool(renderer->vk_device, &command_pool_info,
						 renderer->vk_allocation_callbacks, &command_buffer->vk_command_pool)))
		goto error;

	command_buffer_info.sType	       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_info.pNext	       = NULL;
	command_buffer_info.commandPool	       = command_buffer->vk_command_pool;
	command_buffer_info.level	       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_info.commandBufferCount = 1;

	if (!SF_VULKAN_CHECK(vkAllocateCommandBuffers(
		renderer->vk_device, &command_buffer_info, &command_buffer->vk_command_buffer)))
		goto error;

	return SF_AS_HANDLE(command_buffer);

error:
	sf_graphics_destroy_command_buffer(renderer, SF_AS_HANDLE(command_buffer));
	return SF_NULL_HANDLE;
}

SF_INTERNAL VkShaderModule sf_graphics_vulkan_create_shader(struct sf_graphics_renderer *renderer, uint32_t code_size,
							    void const *code) {
	VkShaderModuleCreateInfo info	= {0};
	VkShaderModule		 shader = VK_NULL_HANDLE;

	if (!renderer || !renderer->vk_device)
		return VK_NULL_HANDLE;

	info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.pNext    = NULL;
	info.flags    = 0;
	info.codeSize = code_size;
	info.pCode    = (uint32_t const *)code;

	if (!SF_VULKAN_CHECK(
		vkCreateShaderModule(renderer->vk_device, &info, renderer->vk_allocation_callbacks, &shader)))
		return VK_NULL_HANDLE;

	return shader;
}

SF_EXTERNAL void sf_graphics_destroy_pipeline(struct sf_graphics_renderer *renderer, sf_handle pipeline_handle) {
	struct sf_graphics_pipeline *pipeline = (struct sf_graphics_pipeline *)pipeline_handle;
	if (!renderer || !pipeline)
		return;

	SF_QUEUE_REMOVE(&pipeline->queue);
	SF_QUEUE_INSERT_HEAD(&renderer->pipeline_pool.free_resource_queue, &pipeline->queue);

	if (renderer->vk_device) {
		VkDevice	       device		    = renderer->vk_device;
		VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;
		if (pipeline->vk_pipeline) {
			vkDestroyPipeline(device, pipeline->vk_pipeline, allocation_callbacks);
			pipeline->vk_pipeline = VK_NULL_HANDLE;
		}

		if (pipeline->vk_pipeline_layout) {
			vkDestroyPipelineLayout(device, pipeline->vk_pipeline_layout, allocation_callbacks);
			pipeline->vk_pipeline_layout = VK_NULL_HANDLE;
		}

		if (pipeline->vk_vertex_shader) {
			vkDestroyShaderModule(device, pipeline->vk_vertex_shader, allocation_callbacks);
			pipeline->vk_vertex_shader = VK_NULL_HANDLE;
		}
		if (pipeline->vk_fragment_shader) {
			vkDestroyShaderModule(device, pipeline->vk_fragment_shader, allocation_callbacks);
			pipeline->vk_fragment_shader = VK_NULL_HANDLE;
		}
	}
}

SF_INTERNAL uint64_t sf_graphics_get_format_stride(enum sf_graphics_format format) {
	switch (format) {
		// 1 channel
		case SF_GRAPHICS_FORMAT_R8_UNORM:	     return 1;
		case SF_GRAPHICS_FORMAT_R16_UNORM:	     return 2;
		case SF_GRAPHICS_FORMAT_R16_UINT:	     return 2;
		case SF_GRAPHICS_FORMAT_R16_SFLOAT:	     return 2;
		case SF_GRAPHICS_FORMAT_R32_UINT:	     return 4;
		case SF_GRAPHICS_FORMAT_R32_SFLOAT:	     return 4;
		// 2 CHANNEL
		case SF_GRAPHICS_FORMAT_R8G8_UNORM:	     return 2;
		case SF_GRAPHICS_FORMAT_R16G16_UNORM:	     return 4;
		case SF_GRAPHICS_FORMAT_R16G16_SFLOAT:	     return 4;
		case SF_GRAPHICS_FORMAT_R32G32_UINT:	     return 8;
		case SF_GRAPHICS_FORMAT_R32G32_SFLOAT:	     return 8;
		// 3 CHANNEL
		case SF_GRAPHICS_FORMAT_R8G8B8_UNORM:	     return 3;
		case SF_GRAPHICS_FORMAT_R16G16B16_UNORM:     return 6;
		case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT:    return 6;
		case SF_GRAPHICS_FORMAT_R32G32B32_UINT:	     return 12;
		case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT:    return 12;
		// 4 CHANNEL
		case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM:	     return 4;
		case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM:	     return 4;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM:  return 8;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: return 8;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT:   return 16;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return 16;
		// DEPTH/STENCIL
		case SF_GRAPHICS_FORMAT_D16_UNORM:	     return 0;
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: return 0;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT:	     return 0;
		case SF_GRAPHICS_FORMAT_S8_UINT:	     return 0;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT:   return 0;
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT:   return 0;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT:  return 0;
		default:				     return 0;
	}
}

SF_EXTERNAL sf_handle sf_graphics_create_pipeline(struct sf_graphics_renderer		   *renderer,
						  struct sf_graphics_vertex_layout	   *vertex_layout,
						  struct sf_graphics_descriptor_set_layout *descriptor_set_layout,
						  uint32_t vertex_code_size, void const *vertex_code,
						  uint32_t fragment_code_size, void const *fragment_code) {
	struct sf_graphics_pipeline *pipeline = NULL;
	if (!renderer)
		return SF_NULL_HANDLE;

	pipeline = sf_graphics_get_or_allocate_pipeline_from_resource_pool(&renderer->arena, &renderer->pipeline_pool);
	if (!pipeline)
		return SF_NULL_HANDLE;

	pipeline->vk_vertex_shader = sf_graphics_vulkan_create_shader(renderer, vertex_code_size, vertex_code);
	if (!pipeline->vk_vertex_shader)
		goto error;

	pipeline->vk_fragment_shader = sf_graphics_vulkan_create_shader(renderer, fragment_code_size, fragment_code);
	if (!pipeline->vk_fragment_shader)
		goto error;

	return SF_AS_HANDLE(pipeline);

error:
	sf_graphics_destroy_pipeline(renderer, SF_AS_HANDLE(pipeline));
	return SF_NULL_HANDLE;
}

#define SF_ARRAY_INIT(array, value)                                                                                   \
	do {                                                                                                          \
		uint32_t i_;                                                                                          \
		for (i_ = 0; i_ < SF_SIZE(array); ++i_)                                                               \
			array[i_] = value;                                                                            \
	} while (0)

static void sf_graphics_default_init_renderer(struct sf_graphics_renderer *renderer) {
	renderer->arena.data			       = NULL;
	renderer->vk_instance			       = NULL;
	renderer->vk_allocation_callbacks	       = NULL;
	renderer->vk_create_debug_utils_messenger_ext  = NULL;
	renderer->vk_destroy_debug_utils_messenger_ext = NULL;
	renderer->vk_validation_messenger	       = NULL;
	renderer->vk_surface_format		       = VK_FORMAT_UNDEFINED;
	renderer->vk_depth_stencil_format	       = VK_FORMAT_UNDEFINED;
	renderer->vk_present_mode		       = VK_PRESENT_MODE_FIFO_KHR;
	renderer->vk_surface_color_space	       = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	renderer->vk_surface			       = VK_NULL_HANDLE;
	renderer->vk_samples			       = VK_SAMPLE_COUNT_1_BIT;
	renderer->vk_physical_device		       = VK_NULL_HANDLE;
	renderer->vk_device			       = VK_NULL_HANDLE;
	renderer->vk_graphics_queue		       = VK_NULL_HANDLE;
	renderer->vk_present_queue		       = VK_NULL_HANDLE;
	renderer->vk_present_queue_family_index	       = (uint32_t)-1;
	renderer->vk_graphics_queue_family_index       = (uint32_t)-1;
	renderer->swapchain_skip_end_frame	       = SF_FALSE;
	renderer->swapchain_width		       = 0;
	renderer->swapchain_height		       = 0;
	renderer->vk_swapchain			       = VK_NULL_HANDLE;
	renderer->vk_swapchain_image_count	       = 0;
	SF_ARRAY_INIT(renderer->vk_swapchain_images, VK_NULL_HANDLE);
	renderer->current_swapchain_image_index = 0;
	renderer->current_frame_index		= 0;

	SF_ASSERT(0);
}

static void sf_graphics_create_image_acquired_semaphores(struct sf_graphics_renderer *renderer, uint32_t count) {
	uint32_t	       i;
	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	renderer->vk_image_acquired_semaphore_count = 0;

	for (i = 0; i < count; ++i) {
		VkSemaphoreCreateInfo info;

		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;

		if (!SF_VULKAN_CHECK(vkCreateSemaphore(
			device, &info, allocation_callbacks, &renderer->vk_image_acquired_semaphores[i])))
			return;
	}
	renderer->vk_image_acquired_semaphore_count = count;
}

static void sf_graphics_create_draw_complete_semaphores(struct sf_graphics_renderer *renderer, uint32_t count) {
	uint32_t	       i;
	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	renderer->vk_draw_complete_semaphore_count = 0;

	for (i = 0; i < count; ++i) {
		VkSemaphoreCreateInfo info;

		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;

		if (!SF_VULKAN_CHECK(vkCreateSemaphore(
			device, &info, allocation_callbacks, &renderer->vk_draw_complete_semaphores[i])))
			return;
	}

	renderer->vk_draw_complete_semaphore_count = count;
}

static void sf_graphics_create_in_flight_fences(struct sf_graphics_renderer *renderer, uint32_t count) {
	uint32_t	       i;
	VkDevice	       device		    = renderer->vk_device;
	VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

	renderer->vk_in_flight_fence_count = 0;

	for (i = 0; i < count; ++i) {
		VkFenceCreateInfo info;

		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (!SF_VULKAN_CHECK(
			vkCreateFence(device, &info, allocation_callbacks, &renderer->vk_in_flight_fences[i])))
			return;
	}

	renderer->vk_in_flight_fence_count = count;
}

struct sf_graphics_renderer *sf_graphics_create_renderer(struct sf_arena			 *arena,
							 struct sf_graphics_renderer_description *description) {
	struct sf_graphics_renderer *renderer = sf_allocate(arena, sizeof(struct sf_graphics_renderer));
	if (!renderer)
		return NULL;

	sf_graphics_default_init_renderer(renderer);

	renderer->swapchain_width  = description->width;
	renderer->swapchain_height = description->height;

	sf_scratch(arena, 1024 * 128, &renderer->arena);
	if (!renderer->arena.data)
		return NULL;

	renderer->vk_allocation_callbacks = NULL;

	sf_graphics_vulkan_create_instance(renderer, description);
	if (!renderer->vk_instance)
		goto error;

	sf_graphics_vulkan_proc_functions(renderer);
	if (!renderer->vk_create_debug_utils_messenger_ext || !renderer->vk_destroy_debug_utils_messenger_ext)
		goto error;

	sf_graphics_vulkan_create_validation_messenger(renderer);
	if (!renderer->vk_validation_messenger)
		goto error;

	sf_graphics_vulkan_create_surface(renderer, description);
	if (!renderer->vk_surface)
		goto error;

	sf_graphics_vulkan_pick_physical_device(renderer, description);
	if (!renderer->vk_physical_device)
		goto error;

	sf_graphics_vulkan_create_device(renderer, description);
	if (!renderer->vk_device)
		goto error;

	sf_graphics_vulkan_pick_surface_format(renderer);
	sf_graphics_vulkan_pick_present_mode(renderer, description);
	sf_graphics_vulkan_pick_depth_format(renderer);
	sf_graphics_vulkan_pick_sample_count(renderer);

	sf_graphics_vulkan_create_swapchain_resources(renderer);
	if (!renderer->vk_swapchain)
		goto error;

	sf_graphics_create_image_acquired_semaphores(renderer, renderer->vk_main_command_buffer_count);
	if (!renderer->vk_image_acquired_semaphore_count)
		goto error;

	sf_graphics_create_in_flight_fences(renderer, renderer->vk_main_command_buffer_count);
	if (!renderer->vk_in_flight_fence_count)
		goto error;

	sf_graphics_create_draw_complete_semaphores(renderer, renderer->vk_swapchain_image_count);
	if (!renderer->vk_draw_complete_semaphore_count)
		goto error;

	return renderer;

error:
	sf_graphics_destroy_renderer(renderer);
	return NULL;
}

void sf_graphics_destroy_renderer(struct sf_graphics_renderer *renderer) {
	if (!renderer)
		return;

	if (renderer->vk_device) {
		uint32_t	       i		       = 0;
		VkDevice	       device		       = renderer->vk_device;
		VkAllocationCallbacks *vk_allocation_callbacks = renderer->vk_allocation_callbacks;

		vkDeviceWaitIdle(device);

		for (i = 0; i < SF_SIZE(renderer->vk_draw_complete_semaphores); ++i) {
			VkSemaphore semaphore = renderer->vk_draw_complete_semaphores[i];

			if (!semaphore)
				continue;

			vkDestroySemaphore(device, semaphore, vk_allocation_callbacks);
			renderer->vk_draw_complete_semaphores[i] = VK_NULL_HANDLE;
		}
		renderer->vk_draw_complete_semaphore_count = 0;

		for (i = 0; i < SF_SIZE(renderer->vk_in_flight_fences); ++i) {
			VkFence fence = renderer->vk_in_flight_fences[i];

			if (!fence)
				continue;

			vkDestroyFence(device, fence, vk_allocation_callbacks);
			renderer->vk_in_flight_fences[i] = VK_NULL_HANDLE;
		}
		renderer->vk_in_flight_fence_count = 0;

		for (i = 0; i < SF_SIZE(renderer->vk_image_acquired_semaphores); ++i) {
			VkSemaphore semaphore = renderer->vk_image_acquired_semaphores[i];

			if (!semaphore)
				continue;

			vkDestroySemaphore(device, semaphore, vk_allocation_callbacks);
			renderer->vk_image_acquired_semaphores[i] = VK_NULL_HANDLE;
		}
		renderer->vk_image_acquired_semaphore_count = 0;

		for (i = 0; i < SF_SIZE(renderer->vk_main_command_buffers); ++i) {
			VkCommandPool	command_pool   = renderer->vk_main_command_pools[i];
			VkCommandBuffer command_buffer = renderer->vk_main_command_buffers[i];

			if (!command_buffer || !command_pool)
				continue;

			vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
			renderer->vk_main_command_buffers[i] = VK_NULL_HANDLE;
		}
		renderer->vk_main_command_buffer_count = 0;

		for (i = 0; i < SF_SIZE(renderer->vk_main_command_pools); ++i) {
			VkCommandPool command_pool = renderer->vk_main_command_pools[i];
			if (!command_pool)
				continue;

			vkDestroyCommandPool(device, command_pool, vk_allocation_callbacks);
			renderer->vk_main_command_pools[i] = VK_NULL_HANDLE;
		}
		renderer->main_command_pool_count = 0;

		sf_graphics_vulkan_destroy_swapchain_resources(renderer);

		vkDestroyDevice(device, vk_allocation_callbacks);
		renderer->vk_device = VK_NULL_HANDLE;
	}

	if (renderer->vk_instance) {
		VkInstance	       instance		    = renderer->vk_instance;
		VkAllocationCallbacks *allocation_callbacks = renderer->vk_allocation_callbacks;

		if (renderer->vk_surface) {
			vkDestroySurfaceKHR(instance, renderer->vk_surface, allocation_callbacks);
			renderer->vk_surface = VK_NULL_HANDLE;
		}

		if (renderer->vk_destroy_debug_utils_messenger_ext && renderer->vk_validation_messenger) {
			renderer->vk_destroy_debug_utils_messenger_ext(
			    instance, renderer->vk_validation_messenger, allocation_callbacks);
			renderer->vk_validation_messenger = VK_NULL_HANDLE;
		}

		vkDestroyInstance(instance, allocation_callbacks);
		renderer->vk_instance = VK_NULL_HANDLE;
	}

	sf_arena_clear(&renderer->arena);
}

void sf_graphics_renderer_begin_frame(struct sf_graphics_renderer *renderer) {
	VkResult result;

	VkClearValue		 clear_values[2];
	VkCommandBufferBeginInfo command_buffer_begin_info;
	VkRenderPassBeginInfo	 render_pass_begin_info;
	VkViewport		 viewport;
	VkRect2D		 scissor;

	// https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html
	uint32_t	current_image_index		 = 0;
	uint32_t	current_frame_index		 = renderer->current_frame_index;
	VkCommandPool	current_command_pool		 = renderer->main_command_pools[current_frame_index];
	VkCommandBuffer current_command_buffer		 = renderer->main_command_buffers[current_frame_index];
	VkFence		current_in_flight_fence		 = renderer->in_flight_fences[current_frame_index];
	VkSemaphore	current_image_acquired_semaphore = renderer->image_acquired_semaphores[current_frame_index];

	VkDevice device = renderer->vk_device;

	vkWaitForFences(device, 1, &current_in_flight_fence, VK_TRUE, (uint64_t)-1);
	vkResetFences(device, 1, &current_in_flight_fence);
	vkResetCommandPool(device, current_command_pool, 0);

	result = vkAcquireNextImageKHR(device, renderer->vk_swapchain, (uint64_t)-1, current_image_acquired_semaphore,
				       VK_NULL_HANDLE, &renderer->current_swapchain_image_index);
	if (VK_ERROR_OUT_OF_DATE_KHR != result && VK_ERROR_SURFACE_LOST_KHR != result) {
		current_image_index = renderer->current_swapchain_image_index;
	} else {
		SF_VULKAN_CHECK(result);
		vkDeviceWaitIdle(device);
		sf_graphics_vulkan_destroy_swapchain_resources(renderer);
		sf_graphics_vulkan_create_swapchain_resources(renderer);
		renderer->swapchain_skip_end_frame = SF_TRUE;
		return;
	}

	command_buffer_begin_info.sType		   = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.pNext		   = NULL;
	command_buffer_begin_info.flags		   = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	command_buffer_begin_info.pInheritanceInfo = NULL;

	vkBeginCommandBuffer(current_command_buffer, &command_buffer_begin_info);

	clear_values[0].color.float32[0]     = 0.0F;
	clear_values[0].color.float32[1]     = 0.0F;
	clear_values[0].color.float32[2]     = 0.0F;
	clear_values[0].color.float32[3]     = .0F;
	clear_values[1].depthStencil.depth   = 1.0F;
	clear_values[1].depthStencil.stencil = 0.0F;

	render_pass_begin_info.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext			= NULL;
	render_pass_begin_info.renderPass		= renderer->main_render_pass;
	render_pass_begin_info.framebuffer		= renderer->main_framebuffers[current_image_index];
	render_pass_begin_info.renderArea.offset.x	= 0;
	render_pass_begin_info.renderArea.offset.y	= 0;
	render_pass_begin_info.renderArea.extent.width	= renderer->swapchain_width;
	render_pass_begin_info.renderArea.extent.height = renderer->swapchain_height;

	render_pass_begin_info.clearValueCount = SF_SIZE(clear_values);
	render_pass_begin_info.pClearValues    = clear_values;

	vkCmdBeginRenderPass(current_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	viewport.x	  = 0;
	viewport.y	  = 0;
	viewport.width	  = renderer->swapchain_width;
	viewport.height	  = renderer->swapchain_height;
	viewport.minDepth = 0.0F;
	viewport.maxDepth = .0F;

	vkCmdSetViewport(current_command_buffer, 0, 1, &viewport);

	scissor.offset.x      = 0;
	scissor.offset.y      = 0;
	scissor.extent.width  = renderer->swapchain_width;
	scissor.extent.height = renderer->swapchain_height;

	vkCmdSetScissor(current_command_buffer, 0, 1, &scissor);
}

void sf_graphics_renderer_end_frame(struct sf_graphics_renderer *renderer) {
	VkResult		result = {0};
	VkSubmitInfo		submit_info = {0};
	VkPresentInfoKHR	present_info = {0};
	VkPipelineStageFlagBits stage = {0};

	VkQueue	 graphics_queue = renderer->vk_graphics_queue;
	VkQueue	 present_queue	= renderer->vk_present_queue;
	VkDevice device		= renderer->vk_device;

	uint32_t	current_frame_index		 = renderer->current_frame_index;
	uint32_t	current_image_index		 = renderer->current_swapchain_image_index;
	VkCommandBuffer current_command_buffer		 = renderer->main_command_buffers[current_frame_index];
	VkSemaphore	current_image_acquired_semaphore = renderer->image_acquired_semaphores[current_frame_index];
	VkFence		current_in_flight_fence		 = renderer->in_flight_fences[current_frame_index];
	VkSemaphore	current_draw_complete_semaphore	 = renderer->vk_draw_complete_semaphores[current_image_index];

	if (renderer->swapchain_skip_end_frame) {
		renderer->swapchain_skip_end_frame = SF_FALSE;
		return;
	}

	vkCmdEndRenderPass(current_command_buffer);
	vkEndCommandBuffer(current_command_buffer);

	stage				 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit_info.sType		 = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext		 = NULL;
	submit_info.waitSemaphoreCount	 = 1;
	submit_info.pWaitSemaphores	 = &current_image_acquired_semaphore;
	submit_info.pWaitDstStageMask	 = &stage;
	submit_info.commandBufferCount	 = 1;
	submit_info.pCommandBuffers	 = &current_command_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores	 = &current_draw_complete_semaphore;

	vkQueueSubmit(graphics_queue, 1, &submit_info, current_in_flight_fence);

	present_info.sType		= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext		= NULL;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores	= &current_draw_complete_semaphore;
	present_info.swapchainCount	= 1;
	present_info.pSwapchains	= &renderer->vk_swapchain;
	present_info.pImageIndices	= &current_image_index;
	present_info.pResults		= NULL;

	result = vkQueuePresentKHR(present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		SF_VULKAN_CHECK(result);
		vkDeviceWaitIdle(device);
		sf_graphics_vulkan_destroy_swapchain_resources(renderer);
		sf_graphics_vulkan_create_swapchain_resources(renderer);
	}
}
