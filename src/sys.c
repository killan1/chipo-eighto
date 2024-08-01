#include "sys.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FREQ_DEFAULT 20
#define FREQ_INCREASE 50

SYS *sys_init() {
  SYS *sys = malloc(sizeof(SYS));

  if (sys == NULL)
    terminate("Failed to allocate memory");

  sys->chip_cycles_left = FREQ_DEFAULT;
  sys->chip_freq = FREQ_DEFAULT;
  sys->show_fps = false;

  return sys;
}

void sys_inc_freq(SYS *sys) {
  sys->chip_freq += FREQ_INCREASE;
  printf("CPF: %d\n", sys->chip_freq);
}

void sys_dec_freq(SYS *sys) {
  if (sys->chip_freq <= FREQ_INCREASE)
    sys->chip_freq = FREQ_DEFAULT;
  else
    sys->chip_freq -= FREQ_INCREASE;
  printf("CPF: %d\n", sys->chip_freq);
}

bool sys_is_chip_active(SYS *sys) { return sys->chip_cycles_left-- > 0; }

void sys_reset_cycles(SYS *sys) { sys->chip_cycles_left = sys->chip_freq; }

void sys_destroy(SYS *sys) { free(sys); }
