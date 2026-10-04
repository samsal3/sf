#ifndef SF_OS_H
#define SF_OS_H

#include "sf_core.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

struct sf_graphics_init_context_info;

struct sf_os_window {
	u32 width;
	u32 height;
	struct  {
		GLFWwindow *window;
	} glfw;
};

sf_public struct sf_os_window *sf_os_init_window(struct sf_arena *arena, u32 width, u32 height, struct sf_string const *title);
sf_public void sf_os_deinit_window(struct sf_os_window *window);

sf_public void sf_os_process_events(struct sf_os_window *window);
sf_public sf_bool sf_os_window_should_close(struct sf_os_window *window);
sf_public void sf_os_fill_graphics_init_context_info(struct sf_arena *arena, struct sf_os_window *window, struct sf_graphics_init_context_info *info);


#endif
