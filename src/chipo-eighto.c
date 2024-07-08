#include "chip.h"
#include "media.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define CYCLES_PER_FRAME 20

typedef struct {
  uint16_t cycles;
  uint16_t cycles_per_frame;
} ChipSystem;
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

void sys_inc_cpf(InputHandler *h) {
  ChipSystem *sys = h->ctx;

  sys->cycles_per_frame += 50;
  printf("CPF: %d\n", sys->cycles_per_frame);
}

void sys_dec_cpf(InputHandler *h) {
  ChipSystem *sys = h->ctx;

  if (sys->cycles_per_frame <= 50)
    sys->cycles_per_frame = CYCLES_PER_FRAME;
  else
    sys->cycles_per_frame -= 50;
  printf("CPF: %d\n", sys->cycles_per_frame);
}

int main(int argc, char **argv) {
  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  CHIP8 chip = chip_init();
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  ChipSystem sys = {.cycles = CYCLES_PER_FRAME,
                    .cycles_per_frame = CYCLES_PER_FRAME};

  MEDIA media = media_init();

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
                     .alt = 0,
                     .event = PRESSED,
                     .ctx = &sys,
                     .handle = &sys_inc_cpf};
  media_register_input_handler(media, ih);
  InputHandler dh = {.keycode = '-',
                     .alt = 0,
                     .event = PRESSED,
                     .ctx = &sys,
                     .handle = &sys_dec_cpf};
  media_register_input_handler(media, dh);

  uint8_t *vram = chip_get_vram_ref(chip);

  while (media_is_active(media)) {
    while (sys.cycles--) {
      chip_run_cycle(chip);
    }
    sys.cycles = sys.cycles_per_frame;

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
