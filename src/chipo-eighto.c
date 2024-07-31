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
  uint16_t win_height;
  uint16_t win_width;
} Config;

void parse_color(char *key, char *value, void *data) {
  if (value == NULL)
    terminate("Wrong value for color arg");

  MediaColor color;
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
      color.r = i;
    if (p == 1)
      color.g = i;
    if (p == 2)
      color.b = i;
    if (p == 3)
      color.a = i;
    p++;
  }
  Config *conf = (Config *)data;
  if (!strncmp(key, "bg", 2)) {
    conf->background = color;
  }
  if (!strncmp(key, "fg", 2)) {
    conf->foreground = color;
  }
}

void parse_window_size(char *key, char *value, void *data) {
  if (value == NULL)
    terminate("Wrong number value");

  char *end;
  size_t val = strtol(value, &end, 10);
  Config *conf = (Config *)data;
  if (!strncmp(key, "width", 5)) {
    conf->win_width = val;
  }
  if (!strncmp(key, "height", 6)) {
    conf->win_height = val;
  }
}

int main(int argc, char **argv) {
  Config *config = malloc(sizeof(Config));
  ArgParseOption options[] = {
      (ArgParseOption){.str = "bg", .ch = 'b', .parse = &parse_color},
      (ArgParseOption){.str = "fg", .ch = 'f', .parse = &parse_color},
      (ArgParseOption){.str = "width", .ch = 'w', .parse = &parse_window_size},
      (ArgParseOption){
          .str = "height", .ch = 'h', .parse = &parse_window_size}};
  parse_args(options, 4, argc, argv, config);
  RomData rd = read_rom_file(argv[1]);
  printf("Loading rom %s (%ld)\n", argv[1], rd.size);

  SYS *sys = sys_init();
  CHIP8 chip = chip_init();
  chip_load_rom(chip, rd.data, rd.size);
  free(rd.data);

  MediaConfig mconfig = {.background_color = config->background,
                         .foreground_color = config->foreground,
                         .win_height = config->win_height,
                         .win_width = config->win_width};
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
