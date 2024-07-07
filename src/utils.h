#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
	uint8_t *data;
	size_t size;
} RomData;

RomData read_rom_file(char *);

#endif
