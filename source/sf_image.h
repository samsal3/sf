#ifndef SF_IMAGE_H
#define SF_IMAGE_H

#include "sf_core.h"

#include "sf_graphics.h"

sf_public void *sf_image_load_from_file(struct sf_arena *arena, struct sf_string *path, u32 *width, u32 *height, enum sf_graphics_format *format);
sf_public void sf_image_free(void *image_data);


#endif
