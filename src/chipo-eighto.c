#include "args.h"
#include "chip.h"
#include "config.h"
#include "media.h"
#include "sys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

Config *parse_args_into_config(int argc, char **argv) {
  Config *config = config_init();

  ArgParserOptions *options = args_init_options(4);
  args_add_options(
      options, 4,
      (ArgParserOption){.lng = "bg",
                        .shrt = 'b',
                        .description =
                            "rgba color for screen background in format "
                            "255,255,255,255. Default: 0,0,0,255",
                        .parse = &parse_color_arg_value,
                        .set = &config_set_background},
      (ArgParserOption){.lng = "fg",
                        .shrt = 'f',
                        .description =
                            "rgba color for screen foreground in format "
                            "255,255,255,255. Default: 0,238,0,255",
                        .parse = &parse_color_arg_value,
                        .set = &config_set_foreground},
      (ArgParserOption){
          .lng = "quirk",
          .shrt = 'q',
          .description =
              "enable a quirk to tweak some known behaviors. Can be used "
              "multiple times.\n"
              "\t\t\tPossible values:\n"
              "\t\t\t\tvfreset  - set VF register to zero for 8XY1, 8XY2, 8XY3 "
              "instructions\n"
              "\t\t\t\tmemory   - don't modify index register for FX55, FX65 "
              "instructions\n"
              "\t\t\t\tdisplay  - limiting sprites drawing by 60 per frame\n"
              "\t\t\t\tclipping - clip sprites instead of wrapping around to "
              "the top of the screen\n"
              "\t\t\t\tshifting - ignore VY register and 8XY6, 8XYE "
              "instructions and directly modify VX register.\n"
              "\t\t\t\tjumping  - add VX register instead of V0 to address for "
              "BNNN instruction",
          .parse = &parse_chip_quirk_arg_value,
          .set = &config_set_chip_quirks},
      (ArgParserOption){.lng = "help",
                        .shrt = 'h',
                        .description = "display this help and exit",
                        .parse = &display_help_message,
                        .set = NULL});

  args_parse(options, argc, argv, config);
  args_destroy(options);

  return config;
}

int main(int argc, char **argv) {
  Config *config = parse_args_into_config(argc, argv);

  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  SYS *sys = sys_init();
  CHIP8 chip = chip_init((ChipConfig){.quirks = config->chip_quirks});
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  MediaConfig mconfig = {.background_color = config->background,
                         .foreground_color = config->foreground};
  MEDIA media = media_init(mconfig);

  register_input_handlers(media, sys, chip);

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
