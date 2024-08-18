#include "../media.h"
#include "../chip.h"
#include "../utils.h"
#include <limits.h>
#include <stdlib.h>

struct media {};

MEDIA media_init(MediaConfig config) {
  MEDIA headless = malloc(sizeof(struct media));

  if (headless == NULL) {
    terminate("Failed to allocate memory");
  }

  return headless;
}

bool media_is_active(MEDIA media) { return true; }
void media_toggle_fps(MEDIA media) {}
void media_update_screen(MEDIA media, const CHIP8 chip) {}
void media_start_drawing(MEDIA media) {}
void media_stop_drawing(MEDIA media) {}
void media_destroy(MEDIA media) {
  free(media);
}
void media_read_input(MEDIA media) {}
void media_register_input_handler(MEDIA media, InputHandler handler) {}
void media_play_sound(MEDIA media) {}
void media_pause_sound(MEDIA media) {}
