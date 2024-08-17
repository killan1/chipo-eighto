#ifndef UTILS_H
#define UTILS_H

#include "args.h"
#include "chip.h"
#include "media.h"
#include "sys.h"
#include <stdint.h>
#include <stdio.h>

static const uint8_t input_keys[16] = {'X', '1', '2', '3', 'Q', 'W', 'E', 'A',
                                       'S', 'D', 'Z', 'C', '4', 'R', 'F', 'V'};

typedef struct {
  uint8_t *data;
  size_t size;
} RomData;

RomData read_rom_file(char *);
void terminate(const char *);
void register_input_handlers(MEDIA, SYS *, CHIP8);
void *parse_color_arg_value(char *, char *, void *);
void *parse_chip_quirk_arg_value(char *, char *, void *);
void *display_help_message(char *, char *, void *);

#endif
