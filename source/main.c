#include <sf_graphics.h>

#include <stdlib.h>
#include <stdio.h>

int main(void) {
	struct sf_arena arena = {0};
	struct sf_graphics_glfw_platform *platform = NULL;
	struct sf_string title = {0};
	struct sf_string source = {0};
	struct sf_graphics_init_context_info info = {0};
	struct sf_graphics_context *context = NULL;

	arena.alignment = 16;
	arena.capacity = SF_MB(16);
	arena.data = malloc(SF_MB(16));
	if (!arena.data)
		return 0;

	title = SF_STRING("sf_graphics test");
	source = SF_STRING("resources\\test.jpg");

	platform = sf_graphics_init_glfw_platform(&arena, 800, 600, &title);
	if (!platform)
		goto error;

	sf_graphics_glfw_fill_init_context_info(&arena, platform, &info);
	context = sf_graphics_init_context(&arena, &info);
	if (!context)
		goto error;

	while (!sf_graphics_glfw_should_close(platform)) {
		sf_graphics_begin_frame(context);
		sf_graphics_end_frame(context);
		sf_graphics_glfw_process_events(platform);
	}

error:
	sf_graphics_deinit_context(context);
	sf_graphics_deinit_glfw_platform(platform);
	free(arena.data);
}
