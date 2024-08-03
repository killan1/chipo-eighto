#include "config.h"
#include "media.h"
#include "utils.h"

Config *config_init() {
  Config *config = malloc(sizeof(Config));

  if (config == NULL)
    terminate("Failed to allocate memory");

  config->background = (MediaColor){0, 0, 0, 255};
  config->foreground = (MediaColor){0, 238, 0, 255};

  return config;
}

void config_set_background(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->background = *(MediaColor *)valp;
}

void config_set_foreground(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->foreground = *(MediaColor *)valp;
}
