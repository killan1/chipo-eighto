#ifndef CONFIG_H
#define CONFIG_H

#include "media.h"
#include <stdlib.h>

typedef struct Config {
  MediaColor background;
  MediaColor foreground;
  size_t screen_height;
  size_t screen_width;
  size_t screen_scaling;
} Config;

Config *config_init(void);

void config_set_background(void *, void *);
void config_set_foreground(void *, void *);
void config_set_screen_width(void *, void *);
void config_set_screen_heigth(void *, void *);
void config_set_screen_scaling(void *, void *);

#endif
