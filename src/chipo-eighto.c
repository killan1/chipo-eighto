#include "chip.h"
#include "media.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define CYCLES_PER_FRAME 20

int main(int argc, char **argv) {
  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  CHIP8 chip = chip_init();
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  MEDIA media = media_init();

  uint16_t cc = CYCLES_PER_FRAME, rc = CYCLES_PER_FRAME;
  uint8_t kb_index;
  uint8_t *vram = chip_get_vram_ref(chip);

  while (media_is_active(media)) {
    for (kb_index = 0; kb_index < 16; kb_index++) {
      if (media_is_key_pressed(media, kb_index))
        chip_kb_btn_pressed(chip, kb_index);
      if (media_is_key_released(media, kb_index))
        chip_kb_btn_released(chip, kb_index);
    }
    while (cc--) {
      chip_run_cycle(chip);
    }
    cc = rc;
    media_frame_start(media);
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
