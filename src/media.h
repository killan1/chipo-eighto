#ifndef MEDIA_H
#define MEDIA_H

#include <stdbool.h>
#include <stdint.h>

typedef struct media *MEDIA;
typedef enum { UP, DOWN, PRESSED, RELEASED } MediaBtnEvent;
typedef enum { TOGGLE_FPS } MediaEvent;
typedef struct InputHandler {
  uint8_t keycode;
  uint8_t alt;
  MediaBtnEvent event;
  void *ctx;
  void (*handle)(struct InputHandler *);
} InputHandler;

MEDIA media_init(unsigned char *bg, unsigned char *spr);
bool media_is_active(MEDIA);
void media_update_screen(MEDIA, uint8_t *);
void media_frame_start(MEDIA);
void media_frame_end(MEDIA);
void media_destroy(MEDIA);
void media_play_sound(MEDIA);
void media_read_input(MEDIA);
void media_register_input_handler(MEDIA, InputHandler);
void media_toggle_fps(MEDIA);

#endif
