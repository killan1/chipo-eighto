#include "chip.h"
#include "media.h"
#include "sys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

static const uint8_t input_keys[16] = {'X', '1', '2', '3', 'Q', 'W', 'E', 'A',
                                       'S', 'D', 'Z', 'C', '4', 'R', 'F', 'V'};

void chip_handler(InputHandler *h) {
  CHIP8 chip = h->ctx;

  switch (h->event) {
  case DOWN:
    chip_kb_btn_pressed(chip, h->alt);
    break;
  case RELEASED:
    chip_kb_btn_released(chip, h->alt);
    break;
  default:
    break;
  }
}

void sys_handler(InputHandler *h) {
  SYS *sys = h->ctx;

  switch (h->alt) {
  case INCREMENT_CHIP_FREQ:
    sys_inc_freq(sys);
    break;
  case DECREMENT_CHIP_FREQ:
    sys_dec_freq(sys);
    break;
  }
}

void media_handler(InputHandler *h) {
  MEDIA media = h->ctx;

  switch (h->alt) {
  case TOGGLE_FPS:
    media_toggle_fps(media);
    break;
  }
}

int main(int argc, char **argv) {
  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  SYS *sys = sys_init();
  CHIP8 chip = chip_init();
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  MediaConfig mconfig = {.background_color = (MediaColor){0, 0, 0},
                         .foreground_color = (MediaColor){0, 238, 0}};
  MEDIA media = media_init(mconfig);

  for (uint8_t i = 0; i < 16; i++) {
    InputHandler dh = {.keycode = input_keys[i],
                       .alt = i,
                       .event = DOWN,
                       .ctx = chip,
                       .handle = &chip_handler};
    media_register_input_handler(media, dh);
    InputHandler uh = {.keycode = input_keys[i],
                       .alt = i,
                       .event = RELEASED,
                       .ctx = chip,
                       .handle = &chip_handler};
    media_register_input_handler(media, uh);
  }

  InputHandler ih = {.keycode = '=',
                     .alt = INCREMENT_CHIP_FREQ,
                     .event = PRESSED,
                     .ctx = sys,
                     .handle = &sys_handler};
  media_register_input_handler(media, ih);
  InputHandler dh = {.keycode = '-',
                     .alt = DECREMENT_CHIP_FREQ,
                     .event = PRESSED,
                     .ctx = sys,
                     .handle = &sys_handler};
  media_register_input_handler(media, dh);
  InputHandler fps = {.keycode = '`',
                      .alt = TOGGLE_FPS,
                      .event = PRESSED,
                      .ctx = media,
                      .handle = &media_handler};
  media_register_input_handler(media, fps);

  uint8_t *vram = chip_get_vram_ref(chip);

  while (media_is_active(media)) {
    while (sys_is_chip_active(sys)) {
      chip_run_cycle(chip);
    }
    sys_reset_cycles(sys);

    media_frame_start(media);
    media_read_input(media);
    media_update_screen(media, vram);

    if (chip_is_sound_timer_active(chip)) {
      media_play_sound(media);
    }

    chip_update_timers(chip);
    media_frame_end(media);
  }

  chip_destroy(chip);
  media_destroy(media);

  return 0;
}
