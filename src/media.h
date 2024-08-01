#ifndef MEDIA_H
#define MEDIA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

typedef struct {
  unsigned char r : 8;
  unsigned char g : 8;
  unsigned char b : 8;
  unsigned char a : 8;
} MediaColor;

typedef struct {
  MediaColor background_color;
  MediaColor foreground_color;
  size_t screen_height;
  size_t screen_width;
  size_t screen_scaling;
} MediaConfig;

MEDIA media_init(MediaConfig);
bool media_is_active(MEDIA);
void media_update_screen(MEDIA, uint8_t *);
void media_start_drawing(MEDIA);
void media_stop_drawing(MEDIA);
void media_destroy(MEDIA);
void media_play_sound(MEDIA);
void media_pause_sound(MEDIA);
void media_read_input(MEDIA);
void media_register_input_handler(MEDIA, InputHandler);
void media_toggle_fps(MEDIA);

#endif
