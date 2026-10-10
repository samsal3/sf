#include "sf_os.h"

#include "sf_graphics.h"

#include <stdio.h>


sf_private void sf_os_glfw_framebuffer_resize_callback(GLFWwindow *glfw_window, i32 width, i32 height) {
	struct sf_os_window *window = (struct sf_os_window *)glfwGetWindowUserPointer(glfw_window);

	if (!window)
		return;

	window->width = width;
	window->height = height;
}

sf_public struct sf_os_window *sf_os_init_window(struct sf_arena *arena, u32 width, u32 height, struct sf_string title) {
	struct sf_string null_terminated_title = {0};
	struct sf_os_window *window = NULL;

	window = sf_arena_allocate(arena, sizeof(*window));
	if (!window)
		return NULL;

	if (!glfwInit())
		return NULL;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	null_terminated_title = sf_string_null_terminate(arena, title);
	if (!null_terminated_title.data)
		goto error;

	window->glfw.window = glfwCreateWindow(width, height, null_terminated_title.data, NULL, NULL);
	if (!window->glfw.window)
		goto error;

	glfwSetWindowUserPointer(window->glfw.window, window);
	glfwSetFramebufferSizeCallback(window->glfw.window, sf_os_glfw_framebuffer_resize_callback);
	{
		i32 width_ = 0;
		i32 height_ = 0;
		glfwGetFramebufferSize(window->glfw.window, &width_, &height_);
		window->width = width_;
		window->height = height_;
	}

	return window;

error:
	sf_os_deinit_window(window);
	return NULL;
}

sf_public void sf_os_deinit_window(struct sf_os_window *window) {
	if (!window)
		return;

	if (window->glfw.window) {
		glfwDestroyWindow(window->glfw.window);
		window->glfw.window = NULL;
	}

	glfwTerminate();
}

sf_public void sf_os_process_events(struct sf_os_window *window) {
	if (!window)
		return;

	glfwPollEvents();
}

sf_public sf_bool sf_os_window_should_close(struct sf_os_window *window) {
	if (!window)
		return SF_FALSE;

	return glfwWindowShouldClose(window->glfw.window);
}


sf_private void sf_os_init_graphics_surface(void *data, struct sf_graphics_device *device) {
	struct sf_os_window *window = (struct sf_os_window *)data;

	if (!window || !device || !device->vk.instance)
		return;

	if (VK_SUCCESS != glfwCreateWindowSurface(device->vk.instance, window->glfw.window, device->vk.allocation_callbacks, &device->vk.surface))
		device->vk.surface = VK_NULL_HANDLE;
}

sf_private void sf_os_request_surface_dimension(void *data, u32 *width, u32 *height) {
	struct sf_os_window *window = (struct sf_os_window *)data;

	if (!window)
		return;


	*width = (u32)window->width;
	*height = (u32)window->height;
}

sf_public void sf_os_fill_graphics_init_context_info(struct sf_arena *arena, struct sf_os_window *window, struct sf_graphics_init_context_info *info) {
	u32 base_instance_extension_count = 0;
	char const **base_instance_extensions = NULL;

	u32 required_instance_extension_count = 0;

	sf_local_persist char const *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
	sf_local_persist char const *device_extensions[] = {
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME

#ifdef __APPLE__
	    , "VK_KHR_portability_subset"
#endif
	};

	if (!arena || !window || !info)
		return;

	info->buffering_count = 2;
	info->device_info.application_name = SF_STRING("test sf application!");
	info->device_info.plataform_data = window;
	info->device_info.init_surface = sf_os_init_graphics_surface;
	info->device_info.request_surface_dimensions = sf_os_request_surface_dimension;
	info->device_info.vk.request_enable_validation_layers = SF_TRUE;
	info->device_info.vk.allocation_callbacks = NULL;
	
	info->swapchain_info.requested_image_count = 3;
	info->swapchain_info.requested_enable_vsync = SF_TRUE;
	info->swapchain_info.sample_count = SF_GRAPHICS_SAMPLE_COUNT_1;
	info->swapchain_info.color_clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_COLOR;
	info->swapchain_info.color_clear_value.data.rgba.r = 1.0F;
	info->swapchain_info.color_clear_value.data.rgba.g = 0.0F;
	info->swapchain_info.color_clear_value.data.rgba.b = 0.0F;
	info->swapchain_info.color_clear_value.data.rgba.a = 1.0F;

	info->swapchain_info.depth_stencil_clear_value.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH_STENCIL;
	info->swapchain_info.depth_stencil_clear_value.data.depth_stencil.depth = 1.0F;
	info->swapchain_info.depth_stencil_clear_value.data.depth_stencil.stencil = 0.0F;

	base_instance_extensions = glfwGetRequiredInstanceExtensions(&base_instance_extension_count);

#ifdef __APPLE__
	required_instance_extension_count = base_instance_extension_count + 3;
#else
	required_instance_extension_count = base_instance_extension_count + 1;
#endif
	info->device_info.vk.instance_extensions = sf_arena_allocate(arena, (required_instance_extension_count) * sizeof(char const *));
	if (info->device_info.vk.instance_extensions) {
		u32 i = 0;
		info->device_info.vk.instance_extension_count = required_instance_extension_count;

		for (i = 0; i < base_instance_extension_count; ++i)
			info->device_info.vk.instance_extensions[i] = base_instance_extensions[i];

		info->device_info.vk.instance_extensions[base_instance_extension_count + 0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#ifdef __APPLE__
		info->device_info.vk.instance_extensions[base_instance_extension_count + 1] = "VK_KHR_portability_enumeration";
		info->device_info.vk.instance_extensions[base_instance_extension_count + 2] = "VK_KHR_get_physical_device_properties2";
#endif
	}

	info->device_info.vk.instance_layer_count = SF_SIZE(validation_layers);
	info->device_info.vk.instance_layers = validation_layers;

	info->device_info.vk.device_extension_count = SF_SIZE(device_extensions);
	info->device_info.vk.device_extensions = device_extensions;
}
