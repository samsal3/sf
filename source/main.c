#include <sf_graphics.h>

#include <stdlib.h>

int
main(void)
{
	sf_arena arena	= {0};
	arena.alignment = 16;
	arena.capacity	= 1024 * 1024 * 16;
	arena.data	= malloc(1024 * 1024);
	if (arena.data)
	{
		sf_graphics_glfw_platform *platform = sf_graphics_create_glfw_platform(&arena, 800, 600, SF_STRING("sf_graphics test"));
		if (platform)
		{
			sf_graphics_renderer_description description = {0};
			sf_graphics_glfw_platform_fill_renderer_description(&arena, platform, &description);

			sf_graphics_renderer *renderer = sf_graphics_create_renderer(&arena, &description);
			if (renderer)
			{

				while (!sf_graphics_glfw_platform_should_close(platform))
				{
					sf_graphics_glfw_platform_process_events(platform);
				}

				sf_graphics_destroy_renderer(renderer);
			}

			sf_graphics_destroy_glfw_platform(platform);
		}
		free(arena.data);
	}
}
