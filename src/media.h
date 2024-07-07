#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct media *MEDIA;
typedef struct {
  uint16_t i : 16;
  uint8_t l : 4;
} ChipInput;

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

#endif
