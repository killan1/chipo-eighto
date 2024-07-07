#include "media.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define SCALING 10

static const uint8_t keys[16] = {
    KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, KEY_Q,    KEY_W, KEY_E, KEY_A,
    KEY_S, KEY_D,   KEY_Z,   KEY_C,     KEY_FOUR, KEY_R, KEY_F, KEY_V};

struct media {
  Sound sound;
};

MEDIA media_init() {
  MEDIA media = malloc(sizeof(struct media));
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chipo EIGHTo");

  SetTargetFPS(60);

  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(64);

  /* media->sound = LoadSound("beep.wav"); */

  return media;
}

bool media_is_active(MEDIA media) { return !WindowShouldClose(); }

void media_update_screen(MEDIA media, uint8_t *vram) {
  uint16_t x, y;
  for (x = 0; x < 64; x++) {
    for (y = 0; y < 32; y++)
      if (vram[y * 64 + x])
        DrawRectangle(x * SCALING, y * SCALING, SCALING, SCALING, WHITE);
  }
}

void media_frame_start(MEDIA media) {
  BeginDrawing();
  ClearBackground(BLACK);
}

void media_frame_end(MEDIA media) { EndDrawing(); }

void media_destroy(MEDIA media) {
  /* UnloadSound(media->sound); */
  CloseWindow();
}

bool media_is_key_pressed(MEDIA media, uint8_t key) {
  return IsKeyPressed(keys[key]);
}

bool media_is_key_released(MEDIA media, uint8_t key) {
  return IsKeyReleased(keys[key]);
}

void media_play_sound(MEDIA media) { /* PlaySound(media->sound); */ }
