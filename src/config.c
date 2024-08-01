#include "config.h"
#include "media.h"
#include "utils.h"

#define DEFAULT_SCREEN_WIDTH 64
#define DEFAULT_SCREEN_HEIGHT 32
#define DEFAULT_SCREEN_SCALING 10

Config *config_init() {
  Config *config = malloc(sizeof(Config));

  if (config == NULL)
    terminate("Failed to allocate memory");

  config->screen_width = DEFAULT_SCREEN_WIDTH;
  config->screen_height = DEFAULT_SCREEN_HEIGHT;
  config->screen_scaling = DEFAULT_SCREEN_SCALING;
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

void config_set_screen_width(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->screen_width = *(size_t *)valp;
}

void config_set_screen_heigth(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->screen_height = *(size_t *)valp;
}

void config_set_screen_scaling(void *valp, void *confp) {
  Config *conf = (Config *)confp;
  conf->screen_scaling = *(size_t *)valp;
}
