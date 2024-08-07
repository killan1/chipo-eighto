#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEM_SIZE 0x1000
#define CHIP_VRAM_SIZE 0x0800
#define CHIP_SCREEN_WIDTH 64
#define CHIP_SCREEN_HEIGHT 32
#define SCHIP_VRAM_SIZE 0x2000
#define SCHIP_SCREEN_WIDTH 128
#define SCHIP_SCREEN_HEIGHT 64
#define START_ADDRESS 0x0200
#define SPRITE_WIDTH 8
#define REGS_COUNT 16
#ifndef SUPER_CHIP
#define STACK_SIZE 12
#else
#define STACk_SIZE 16
#endif

typedef enum {
  SHIFT_IGNORE_VY = 1,
  MEM_NOT_MODIFY_I = 2,
  JUMP_USE_VX = 4,
  VF_RESET = 8
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
