#include "utils.h"
#include "chip.h"
#include <stdlib.h>
#include <string.h>

RomData read_rom_file(char *filename) {
  FILE *fp = fopen(filename, "rb");

  if (!fp) {
    printf("Failed to open %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(fp, 0L, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  uint8_t *rom_data = (uint8_t *)malloc(size * sizeof(uint8_t));

  if (rom_data == NULL) {
    fclose(fp);
    terminate("Failed to allocate memory for rom data");
  }

  size_t total_read = fread(rom_data, sizeof(uint8_t), size, fp);

  if (total_read != size) {
    free(rom_data);
    fclose(fp);
    terminate("Failed to read file");
  }

  fclose(fp);

  RomData result = {.data = rom_data, .size = size};

  return result;
}

void terminate(const char *msg) {
  printf("Error: %s\n", msg);
  exit(EXIT_FAILURE);
}

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

void register_input_handlers(MEDIA media, SYS *sys, CHIP8 chip) {
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
}

void *parse_color_arg_value(char *key, char *value) {
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

void *parse_chip_quirk_arg_value(char *key, char *value) {
  uint8_t *val = malloc(sizeof(uint8_t));

  if (strncmp(key, "shift-quirk", strlen(key)) == 0)
    *val = SHIFT_IGNORE_VY;
  if (strncmp(key, "mem-quirk", strlen(key)) == 0)
    *val = MEM_NOT_MODIFY_I;

  return (void *)val;
}

void *display_help_message(char *key, char *value) {
  printf("Usage: chipo8o [FILE] [OPTION]...\n\
Available options:\n\
  -b, --bg[=COLOR]          rgba color for screen background in format 255,255,255,255. Default: 0,0,0,255\n\
  -f, --fg[=COLOR]          rgba color for screen foreground in format 255,255,255,255. Default: 0,238,0,255\n\
      --shift-quirk         ignore VY register for 8XY6, 8XYE instructions and directly modify VX register.\n\
      --mem-quirk           do not modify index register (I) for FX55, FX65 instructions.\n\
  -h, --help                display this help and exit\n");
  exit(0);
}
