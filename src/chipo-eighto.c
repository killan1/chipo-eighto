#include "args.h"
#include "chip.h"
#include "config.h"
#include "media.h"
#include "sys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  Config *config = config_init();

  ArgParserOption options[] = {
      (ArgParserOption){.str = "help",
                        .ch = 'h',
                        .parse = &display_help_message,
                        .set = NULL},
      (ArgParserOption){.str = "bg",
                        .ch = 'b',
                        .parse = &parse_color_arg_value,
                        .set = &config_set_background},
      (ArgParserOption){.str = "fg",
                        .ch = 'f',
                        .parse = &parse_color_arg_value,
                        .set = &config_set_foreground},
      (ArgParserOption){.str = "width",
                        .ch = 'w',
                        .parse = &parse_screen_arg_value,
                        .set = &config_set_screen_width},
      (ArgParserOption){.str = "height",
                        .ch = 'h',
                        .parse = &parse_screen_arg_value,
                        .set = &config_set_screen_heigth},
      (ArgParserOption){.str = "scaling",
                        .ch = 's',
                        .parse = &parse_screen_arg_value,
                        .set = &config_set_screen_scaling}};

  parse_args(options, 6, argc, argv, config);
  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  SYS *sys = sys_init();
  CHIP8 chip = chip_init();
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  MediaConfig mconfig = {.background_color = config->background,
                         .foreground_color = config->foreground,
                         .screen_height = config->screen_height,
                         .screen_width = config->screen_width,
                         .screen_scaling = config->screen_scaling};
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
  sys_destroy(sys);
  free(config);

  return 0;
}
