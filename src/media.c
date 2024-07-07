#include "media.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALING 10
#define CHIP_KEYBOARD_SIZE 16
#define TARGET_FPS 60

static const uint8_t keys[CHIP_KEYBOARD_SIZE] = {
    KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, KEY_Q,    KEY_W, KEY_E, KEY_A,
    KEY_S, KEY_D,   KEY_Z,   KEY_C,     KEY_FOUR, KEY_R, KEY_F, KEY_V};

struct media {
  Sound sound;
  ChipInput input;
};

MEDIA media_init() {
  MEDIA media = malloc(sizeof(struct media));
  InitWindow(SCREEN_WIDTH * SCALING, SCREEN_HEIGHT * SCALING, "Chipo EIGHTo");

  SetTargetFPS(TARGET_FPS);

  ChipInput input = {.i = 0, .l = 0};
  media->input = input;
  /* media->sound = LoadSound("beep.wav"); */

  return media;
}

bool media_is_active(MEDIA media) { return !WindowShouldClose(); }

void media_update_screen(MEDIA media, uint8_t *vram) {
  uint16_t x, y;
  for (x = 0; x < SCREEN_WIDTH; x++) {
    for (y = 0; y < SCREEN_HEIGHT; y++)
      if (vram[y * SCREEN_WIDTH + x])
        DrawRectangle(x * SCALING, y * SCALING, SCALING, SCALING, WHITE);
  }
}

void media_frame_start(MEDIA media) {
  BeginDrawing();
  ClearBackground(BLACK);
}

void media_frame_end(MEDIA media) {
  DrawFPS(10, 10);
  EndDrawing();
}

void media_destroy(MEDIA media) {
  /* UnloadSound(media->sound); */
  CloseWindow();
}

bool media_is_key_pressed(MEDIA media, uint8_t key) {
  return IsKeyDown(keys[key]);
}

bool media_is_key_released(MEDIA media, uint8_t key) {
  return IsKeyReleased(keys[key]);
}

ChipInput read_chip_input(MEDIA media) {
  for (uint8_t key = 0; key < CHIP_KEYBOARD_SIZE; key++) {
    if (IsKeyPressed(keys[key]))
      media->input.l = key;

    if (IsKeyDown(keys[key])) {
      media->input.i |= 1 << key;
    }

    if (IsKeyReleased(keys[key])) {
      media->input.i &= ~1 << key;
    }
  }

  return media->input;
}

void media_play_sound(MEDIA media) { /* PlaySound(media->sound); */ }
