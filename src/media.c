#include "media.h"
#include "raylib.h"
#include "utils.h"
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALING 10
#define CHIP_KEYBOARD_SIZE 16
#define TARGET_FPS 60
#define MAX_INPUT_HANDLERS 100
#define MAX_SAMPLES 512
#define MAX_SAMPLES_PER_UPDATE 4096
#define AUDIO_FREQUENCY 440.0f

static float idx = 0.0f;

struct media {
  InputHandler *ihandlers;
  uint16_t ihandler_count;
  bool show_fps;
  Color bg_color;
  Color fg_color;
  AudioStream stream;
};

static Color media_map_color(MediaColor mc);
static void media_audio_input_callback(void *buffer, unsigned int frames);

MEDIA media_init(MediaConfig config) {
  MEDIA media = malloc(sizeof(struct media));

  if (media == NULL) {
    terminate("Failed to allocate memory");
  }

  InitWindow(config.win_width, config.win_height, "Chipo EIGHTo");

  SetTargetFPS(TARGET_FPS);

  media->ihandlers = malloc(MAX_INPUT_HANDLERS * sizeof(InputHandler));

  if (media->ihandlers == NULL) {
    free(media);
    terminate("Failed to allocate memory");
  }

  media->ihandler_count = 0;
  media->show_fps = false;
  media->bg_color = media_map_color(config.background_color);
  media->fg_color = media_map_color(config.foreground_color);

  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

  media->stream = LoadAudioStream(44100, 16, 1);

  SetAudioStreamCallback(media->stream, media_audio_input_callback);
  PlayAudioStream(media->stream);

  return media;
}

static void media_audio_input_callback(void *buffer, unsigned int frames) {
  float incr = AUDIO_FREQUENCY / 44100.0f;
  short *d = (short *)buffer;

  for (unsigned int i = 0; i < frames; i++) {
    d[i] = sinf(2 * PI * idx) > 0 ? SHRT_MAX : SHRT_MIN;

    if ((idx += incr) > 1.0f)
      idx -= 1.0f;
  }
}

static Color media_map_color(MediaColor mc) {
  return (Color){mc.r, mc.g, mc.b, mc.a};
}

bool media_is_active(MEDIA media) { return !WindowShouldClose(); }

void media_toggle_fps(MEDIA media) { media->show_fps = !media->show_fps; }

void media_update_screen(MEDIA media, uint8_t *vram) {
  uint16_t x, y;
  for (x = 0; x < SCREEN_WIDTH; x++) {
    for (y = 0; y < SCREEN_HEIGHT; y++)
      if (vram[y * SCREEN_WIDTH + x])
        DrawRectangle(x * SCALING, y * SCALING, SCALING, SCALING,
                      media->fg_color);
  }
}

void media_start_drawing(MEDIA media) {
  BeginDrawing();
  ClearBackground(media->bg_color);
}

void media_stop_drawing(MEDIA media) {
  if (media->show_fps)
    DrawFPS(10, 10);
  EndDrawing();
}

void media_destroy(MEDIA media) {
  UnloadAudioStream(media->stream);
  CloseAudioDevice();
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

void media_play_sound(MEDIA media) {
  if (!IsAudioStreamPlaying(media->stream))
    PlayAudioStream(media->stream);
}

void media_pause_sound(MEDIA media) {
  if (IsAudioStreamPlaying(media->stream))
    PauseAudioStream(media->stream);
}
