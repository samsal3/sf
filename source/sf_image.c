#include "sf_image.h"

#include <stb_image.h>

sf_private enum sf_image_channels sf_image_channels_from_stb(int stb_channels) {
	switch (stb_channels) {
		case 1:
			return SF_GRAPHICS_FORMAT_R8_UNORM;
		case 3:
			return SF_GRAPHICS_FORMAT_R8G8_UNORM;
		case 4:
			return SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM;
		default:
			return SF_GRAPHICS_FORMAT_UNDEFINED;
	}
}


sf_public void *sf_image_load_from_file(struct sf_arena *arena, struct sf_string *path, u32 *width, u32 *height, enum sf_graphics_format *format) {
	struct sf_string null_terminated_path = {0};
	int loaded_width = 0;
	int loaded_height = 0;
	int loaded_channels = 0;
	stbi_uc *data = NULL;

	if (!arena || !path || !width || !height || !format)
		return NULL;

	*width = 0;
	*height = 0;
	*format = SF_GRAPHICS_FORMAT_UNDEFINED;

	sf_string_null_terminate(arena, path, &null_terminated_path);
	if (!null_terminated_path.data)
		return NULL;

	data =  stbi_load(&null_terminated_path.data, &loaded_width, &loaded_height, &loaded_channels, 0);
	if (!data)
		return NULL;

	*width = (u32)loaded_width;
	*height = (u32)loaded_height;
	*format = sf_image_channels_from_stb(loaded_channels);

	return data;
}

sf_public void sf_image_free(void *image_data) {
	stbi_image_free(image_data);
}