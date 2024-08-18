#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEM_SIZE 0x1000
#define START_ADDRESS 0x0200
#define REGS_COUNT 16
#define SPRITE_SIZE 8

typedef enum {
  VF_RESET = 1,
  MEMORY = 2,
  DISPLAY = 4,
  CLIPPING = 8,
  SHIFTING = 16,
  JUMPING = 32
} InstrQuirk;
typedef struct chip8 *CHIP8;
typedef struct ChipConfig {
  uint8_t quirks;
} ChipConfig;

CHIP8 chip_init(ChipConfig);
void chip_destroy(CHIP8);
void chip_run_cycle(CHIP8);
void chip_update_timers(CHIP8);
bool chip_is_sound_timer_active(CHIP8);
void chip_load_rom(CHIP8, uint8_t *, size_t);
void chip_kb_btn_pressed(CHIP8, uint8_t);
void chip_kb_btn_released(CHIP8, uint8_t);
void chip_update_input(CHIP8, uint16_t, uint8_t);
uint8_t *chip_get_vram_ref(CHIP8);
uint8_t chip_get_screen_width(CHIP8);
uint8_t chip_get_screen_height(CHIP8);

static const uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

#endif
