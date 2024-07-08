#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct media *MEDIA;
typedef struct {
  uint16_t i : 16;
  uint8_t l : 4;
} ChipInput;
typedef enum { UP, DOWN, PRESSED, RELEASED } MediaBtnEvent;
typedef struct InputHandler {
  uint8_t keycode;
  uint8_t alt;
  MediaBtnEvent event;
  void *ctx;
  void (*handle)(struct InputHandler *);
} InputHandler;

MEDIA media_init(void);
bool media_is_active(MEDIA);
void media_update_screen(MEDIA, uint8_t *);
void media_frame_start(MEDIA);
void media_frame_end(MEDIA);
void media_destroy(MEDIA);
bool media_is_key_pressed(MEDIA, uint8_t);
bool media_is_key_released(MEDIA, uint8_t);
ChipInput read_chip_input(MEDIA media);
void media_play_sound(MEDIA);
void media_read_input(MEDIA);
void media_register_input_handler(MEDIA, InputHandler);

#endif
