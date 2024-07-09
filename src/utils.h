#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
  uint8_t *data;
  size_t size;
} RomData;

RomData read_rom_file(char *);
void terminate(const char *msg);

#endif
