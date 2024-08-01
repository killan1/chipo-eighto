#include "args.h"
#include "chip.h"
#include "media.h"
#include "sys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Config {
  MediaColor background;
  MediaColor foreground;
  size_t screen_height;
  size_t screen_width;
  size_t screen_scaling;
} Config;

void *parse_color(char *key, char *value) {
  if (value == NULL)
    terminate("Wrong value for color arg");

  MediaColor *color = malloc(sizeof(MediaColor));
  int p = 0;
  char *end;
  for (;;) {
    const long i = strtol(value, &end, 10);
    if (value == end)
      break;
    while (*end == ',')
      end++;
    value = end;
    if (p == 0)
      color->r = i;
    if (p == 1)
      color->g = i;
    if (p == 2)
      color->b = i;
    if (p == 3)
      color->a = i;
    p++;
  }

  return (void *)color;
}

void *parse_window_size(char *key, char *value) {
  if (value == NULL)
    terminate("Wrong window size value");

  char *end;
  size_t *val = malloc(sizeof(size_t));
  *val = strtol(value, &end, 10);

  return (void *)val;
}

static void cleanup_parse_value(void *valp) { free(valp); }

void set_config_bg(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->background = *(MediaColor *)valp;
  cleanup_parse_value(valp);
}

void set_config_fg(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->foreground = *(MediaColor *)valp;
  cleanup_parse_value(valp);
}

void set_config_screen_width(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->screen_width = *(size_t *)valp;
  cleanup_parse_value(valp);
}

void set_config_screen_heigth(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->screen_height = *(size_t *)valp;
  cleanup_parse_value(valp);
}

void set_config_screen_scaling(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->screen_scaling = *(size_t *)valp;
  cleanup_parse_value(valp);
}

int main(int argc, char **argv) {
  Config *config = malloc(sizeof(Config));
  config->screen_width = 64;
  config->screen_height = 32;
  config->screen_scaling = 10;

  ArgParseOption options[] = {
      (ArgParseOption){
          .str = "bg", .ch = 'b', .parse = &parse_color, .set = &set_config_bg},
      (ArgParseOption){
          .str = "fg", .ch = 'f', .parse = &parse_color, .set = &set_config_fg},
      (ArgParseOption){.str = "width",
                       .ch = 'w',
                       .parse = &parse_window_size,
                       .set = &set_config_screen_width},
      (ArgParseOption){.str = "height",
                       .ch = 'h',
                       .parse = &parse_window_size,
                       .set = &set_config_screen_heigth},
      (ArgParseOption){.str = "scaling",
                       .ch = 's',
                       .parse = &parse_window_size,
                       .set = &set_config_screen_scaling}};

  parse_args(options, 5, argc, argv, config);
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

  return 0;
}
