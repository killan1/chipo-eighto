#include "args.h"
#include "chip.h"
#include "media.h"
#include "sys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Config {
  MediaColor background;
  MediaColor foreground;
} Config;

void parse_color(char *key, char *value, void *data) {
  if (value == NULL)
    terminate("Wrong value for color arg");

  printf("parse_color key=%s value=%s\n", key, value);
  /* Config *conf = (Config *) data; */
  /* conf->background = (MediaColor){0,0,0,255}; */
}

int main(int argc, char **argv) {
  Config *config = malloc(sizeof(Config));
  ArgParseOption options[] = {
      (ArgParseOption){.str = "bg", .ch = 'b', .parse = &parse_color},
      (ArgParseOption){.str = "fg", .ch = 'f', .parse = &parse_color}};
  parse_args(options, 2, argc, argv, config);
  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  SYS *sys = sys_init();
  CHIP8 chip = chip_init();
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  MediaConfig mconfig = {.background_color = (MediaColor){0, 0, 0, 255},
                         .foreground_color = (MediaColor){0, 238, 0, 255}};
  MEDIA media = media_init(mconfig);

  register_input_handlers(media, sys, chip);

  uint8_t *vram = chip_get_vram_ref(chip);

  while (media_is_active(media)) {
    while (sys_is_chip_active(sys)) {
      chip_run_cycle(chip);
    }
    sys_reset_cycles(sys);

    media_start_drawing(media);
    media_read_input(media);
    media_update_screen(media, vram);

    if (chip_is_sound_timer_active(chip)) {
      media_play_sound(media);
    } else {
      media_pause_sound(media);
    }

    chip_update_timers(chip);
    media_stop_drawing(media);
  }

  chip_destroy(chip);
  media_destroy(media);

  return 0;
}
