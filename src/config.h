#ifndef CONFIG_H
#define CONFIG_H

#include "media.h"
#include <stdlib.h>

typedef struct Config {
  MediaColor background;
  MediaColor foreground;
  uint8_t chip_quirks;
} Config;

Config *config_init(void);

void config_set_background(void *, void *);
void config_set_foreground(void *, void *);
void config_set_chip_quirks(void *, void *);

#endif
