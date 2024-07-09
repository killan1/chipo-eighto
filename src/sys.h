#ifndef SYS_H
#define SYS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct sys *SYS;
typedef enum { INCREMENT_CHIP_FREQ, DECREMENT_CHIP_FREQ } SysEvent;

SYS sys_init();
void sys_inc_freq(SYS);
void sys_dec_freq(SYS);
bool sys_is_chip_active(SYS);
void sys_reset_cycles(SYS);

#endif
