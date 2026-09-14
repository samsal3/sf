#include <sf_graphics.h>

#include <stdlib.h>

int main(void) {
	sf_handle test_image = SF_NULL_HANDLE;
	struct sf_arena arena = {0};
	struct sf_graphics_glfw_platform *platform = NULL;
	struct sf_string title = {0};
	struct sf_string source = {0};
	struct sf_graphics_renderer_info info = {0};
	struct sf_graphics_renderer *renderer = NULL;

	arena.alignment = 16;
	arena.capacity = 1024 * 1024 * 16;
	arena.data = malloc(1024 * 1024);
	if (!arena.data)
		return 0;

	title.data = "sf_graphics test";
	title.size = sizeof("sf_graphics test");

	source.data = "resources\\test.jpg";
	source.size = sizeof("resources\\test.jpg");

	platform = sf_graphics_glfw_platform_init(&arena, 800, 600, &title);
	if (!platform)
		goto error;

	sf_graphics_glfw_platform_fill_renderer_info(&arena, platform, &info);
	renderer = sf_graphics_renderer_init(&arena, &info);
	if (!renderer)
		goto error;

	test_image = sf_graphics_image_init_from_file(renderer, &source);

	while (!sf_graphics_glfw_platform_should_close(platform)) {
		sf_graphics_glfw_platform_process_events(platform);
	}

error:
	sf_graphics_image_deinit(renderer, test_image);
	sf_graphics_renderer_deinit(renderer);
	sf_graphics_glfw_platform_deinit(platform);
	free(arena.data);
}
