#include <SFGraphics.h>

#include <stdlib.h>

int main(void) {
  SFArena arena   = {0};
  arena.alignment = 16;
  arena.capacity  = 1024 * 1024 * 16;
  arena.data      = malloc(1024 * 1024);
  if (arena.data) {
    SFGraphicsGLFWPlatform *platform = sfGraphicsCreateGLFWPlatform(&arena, 800, 600, SF_STRING("SFGraphics Test"));

    if (platform) {
      SFGraphicsRendererDescription description = {0};
      sfGraphicsGLFWFillRendererDescription(&arena, platform, &description);

      SFGraphicsRenderer *renderer = sfGraphicsCreateRenderer(&arena, &description);
      if (renderer) {

        while (!sfGraphicsGLFWPlatformShouldClose(platform)) {
          sfGraphicsGLFWPlatformProcessEvents(platform);
        }

        sfGraphicsDestroyRenderer(renderer);
      }

      sfGraphicsDestroyGLFWPlatform(platform);
    }
    free(arena.data);
  }
}
