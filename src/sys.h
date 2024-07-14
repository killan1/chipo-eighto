#ifndef SYS_H
#define SYS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t chip_cycles_left;
  uint16_t chip_freq;
  bool show_fps;
  unsigned char *bg_color;
  unsigned char *spr_color;
} SYS;
typedef enum { INCREMENT_CHIP_FREQ, DECREMENT_CHIP_FREQ } SysEvent;

SYS *sys_init();
void sys_inc_freq(SYS *);
void sys_dec_freq(SYS *);
bool sys_is_chip_active(SYS *);
void sys_reset_cycles(SYS *);

#endif
