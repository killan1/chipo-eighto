#include "../media.h"
#include "../chip.h"
#include "../utils.h"
#include <limits.h>
#include <stdlib.h>

#define MAX_INPUT_HANDLERS 100

struct media {
  InputHandler *ihandlers;
  uint16_t ihandler_count;
};

MEDIA media_init(MediaConfig config) {
  MEDIA headless = malloc(sizeof(struct media));

  headless->ihandlers = malloc(MAX_INPUT_HANDLERS * sizeof(InputHandler));

  if (headless->ihandlers == NULL) {
    free(headless);
    terminate("Failed to allocate memory");
  }

  headless->ihandler_count = 0;

  return headless;
}

bool media_is_active(MEDIA media) { return true; }
void media_toggle_fps(MEDIA media) {}
void media_update_screen(MEDIA media, const CHIP8 chip) {}
void media_start_drawing(MEDIA media) {}
void media_stop_drawing(MEDIA media) {}
void media_destroy(MEDIA media) {
  free(media->ihandlers);
  free(media);
}
void media_read_input(MEDIA media) {}
void media_register_input_handler(MEDIA media, InputHandler handler) {}
void media_play_sound(MEDIA media) {}
void media_pause_sound(MEDIA media) {}
