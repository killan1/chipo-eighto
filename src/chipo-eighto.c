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
      (ArgParserOption){.str = "shift-quirk",
                        .ch = '\0',
                        .parse = &parse_chip_quirk_arg_value,
                        .set = &config_set_chip_quirks},
      (ArgParserOption){.str = "mem-quirk",
                        .ch = '\0',
                        .parse = &parse_chip_quirk_arg_value,
                        .set = &config_set_chip_quirks},
      (ArgParserOption){.str = "jump-quirk",
                        .ch = '\0',
                        .parse = &parse_chip_quirk_arg_value,
                        .set = &config_set_chip_quirks},
      (ArgParserOption){.str = "vfreset-quirk",
                        .ch = '\0',
                        .parse = &parse_chip_quirk_arg_value,
                        .set = &config_set_chip_quirks}};

  parse_args(options, 7, argc, argv, config);
  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  SYS *sys = sys_init();
  printf("%d\n", config->chip_quirks);
  CHIP8 chip = chip_init((ChipConfig){.quirks = config->chip_quirks});
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  MediaConfig mconfig = {.background_color = config->background,
                         .foreground_color = config->foreground};
  MEDIA media = media_init(mconfig);

  register_input_handlers(media, sys, chip);

  /* uint8_t *vram = chip_get_vram_ref(chip); */

  while (media_is_active(media)) {
    while (sys_is_chip_active(sys)) {
      chip_run_cycle(chip);
    }
    sys_reset_cycles(sys);

    media_start_drawing(media);
    media_read_input(media);
    media_update_screen(media, chip);

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
