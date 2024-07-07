#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

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
    printf("Failed to allocate memory for rom data\n");
    exit(EXIT_FAILURE);
  }

  size_t total_read = fread(rom_data, sizeof(uint8_t), size, fp);

  if (total_read != size) {
    free(rom_data);
    fclose(fp);
    printf("Failed to read file\n");
    exit(EXIT_FAILURE);
  }

  fclose(fp);

  RomData result = {.data = rom_data, .size = size};

  return result;
}
