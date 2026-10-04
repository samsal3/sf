#include <sf_graphics.h>
#include <sf_os.h>

#include <stdlib.h>
#include <stdio.h>

int main(void) {
	struct sf_arena arena = {0};
	struct sf_os_window *window = NULL;
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

	window = sf_os_init_window(&arena, 800, 600, &title);
	if (!window)
		goto error;


	sf_os_fill_graphics_init_context_info(&arena, window, &info);

	context = sf_graphics_init_context(&arena, &info);
	if (!context)
		goto error;

	while (!sf_os_window_should_close(window)) {
		sf_graphics_begin_frame(context);
		sf_graphics_end_frame(context);
		sf_os_process_events(window);
	}

error:
	sf_graphics_deinit_context(context);
	sf_os_deinit_window(window);
	free(arena.data);
}
