#include "media.h"
#include "raylib.h"
#include "sys.h"
#include "utils.h"
#include <stdlib.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALING 10
#define CHIP_KEYBOARD_SIZE 16
#define TARGET_FPS 60
#define MAX_INPUT_HANDLERS 100

struct media {
  SYS sys;
  Sound sound;
  InputHandler *ihandlers;
  uint16_t ihandler_count;
};

MEDIA media_init(SYS sys) {
  MEDIA media = malloc(sizeof(struct media));

  if (media == NULL) {
    terminate("Failed to allocate memory");
  }

  media->sys = sys;

  InitWindow(SCREEN_WIDTH * SCALING, SCREEN_HEIGHT * SCALING, "Chipo EIGHTo");

  SetTargetFPS(TARGET_FPS);

  media->ihandlers = malloc(MAX_INPUT_HANDLERS * sizeof(InputHandler));

  if (media->ihandlers == NULL) {
    free(media);
    terminate("Failed to allocate memory");
  }

  media->ihandler_count = 0;
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
  if (sys_is_show_fps(media->sys))
    DrawFPS(10, 10);
  EndDrawing();
}

void media_destroy(MEDIA media) {
  /* UnloadSound(media->sound); */
  CloseWindow();
}

void media_read_input(MEDIA media) {
  InputHandler handler;
  for (uint8_t i = 0; i < media->ihandler_count; i++) {
    handler = media->ihandlers[i];

    switch (handler.event) {
    case UP:
      if (IsKeyUp(handler.keycode))
        handler.handle(&handler);
      break;
    case DOWN:
      if (IsKeyDown(handler.keycode))
        handler.handle(&handler);
      break;
    case PRESSED:
      if (IsKeyPressed(handler.keycode))
        handler.handle(&handler);
      break;
    case RELEASED:
      if (IsKeyReleased(handler.keycode))
        handler.handle(&handler);
      break;
    default:
      break;
    }
  }
}

void media_register_input_handler(MEDIA media, InputHandler handler) {
  if (media->ihandler_count < MAX_INPUT_HANDLERS) {
    media->ihandlers[media->ihandler_count++] = handler;
  }
}

void media_play_sound(MEDIA media) { /* PlaySound(media->sound); */ }
