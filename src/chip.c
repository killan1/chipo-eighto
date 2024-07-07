#include "chip.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct chip8 {
  uint16_t pc;
  uint8_t *mem;
  uint8_t *vram;
  uint16_t stack[16];

  uint16_t index;
  uint8_t dt;
  uint8_t st;
  uint8_t sp;
  uint8_t regs[16];

  uint16_t input;
  uint8_t input_key;

  uint16_t opcode;
  void (*exec)(CHIP8);
};

static void terminate(const char *msg);
static void fetch(CHIP8);
static void decode(CHIP8);
static void execute(CHIP8);

CHIP8 chip_init() {
  srand(time(NULL));
  CHIP8 chip = malloc(sizeof(struct chip8));

  if (chip == NULL) {
    terminate("Failed to allocate memory");
  }

  chip->mem = malloc(MEM_SIZE * sizeof(uint8_t));
  if (chip->mem == NULL) {
    chip_destroy(chip);
    terminate("Failed to allocate memory");
  }

  chip->vram = calloc(VMEM_SIZE, sizeof(uint8_t));
  if (chip->vram == NULL) {
    chip_destroy(chip);
    terminate("Failed to allocate memory");
  }
  chip->pc = START_ADDRESS;
  chip->index = 0;
  chip->dt = 0;
  chip->st = 0;
  chip->sp = 0;
  chip->input = 0;
  chip->input_key = 0;

  int8_t i;
  for (i = 0; i < 16; i++) {
    chip->stack[i] = 0;
    chip->regs[i] = 0;
  }

  for (i = 0; i < 80; i++) {
    chip->mem[i] = font[i];
  }

  return chip;
}

void chip_destroy(CHIP8 chip) {
  free(chip->mem);
  free(chip->vram);
  free(chip);
}

void chip_run_cycle(CHIP8 chip) {
  fetch(chip);
  decode(chip);
  execute(chip);
}

void chip_kb_btn_pressed(CHIP8 chip, uint8_t key) {
  chip->input |= 1 << key;
  chip->input_key = key;
}

void chip_kb_btn_released(CHIP8 chip, uint8_t key) { chip->input &= ~1 << key; }

void chip_update_input(CHIP8 chip, uint16_t input, uint8_t key) {
  chip->input = input;
  chip->input_key = key;
}

void chip_load_rom(CHIP8 chip, uint8_t *rom, size_t size) {
  memcpy(&chip->mem[START_ADDRESS], rom, size);
}

uint8_t *chip_get_vram_ref(CHIP8 chip) { return chip->vram; }

void chip_update_timers(CHIP8 chip) {
  if (chip->dt)
    chip->dt--;
  if (chip->st)
    chip->st--;
}

bool chip_is_sound_timer_active(CHIP8 chip) { return chip->st > 0; }

static void terminate(const char *msg) {
  printf("%s\n", msg);
  exit(EXIT_FAILURE);
}

static void op_unsupported(CHIP8 chip) {
  printf("WARNING: unsupported opcode %04X\n", chip->opcode);
  exit(EXIT_FAILURE);
}

static void op_jmp(CHIP8 chip) {
  chip->pc = chip->opcode & 0x0FFF;
  /* printf("JMP OP=%04X PC=%04X\n", chip->opcode, chip->pc); */
}

static void op_regload(CHIP8 chip) {
  /* printf("REG LD OP=%04X PC=%04X\n", chip->opcode, chip->pc); */
  uint8_t reg = (uint8_t)(chip->opcode >> 8) & 0x000F;
  uint8_t value = (uint8_t)(chip->opcode & 0x00FF);

  chip->regs[reg] = value;
  /* printf("V%X = %04X\n", reg, value); */
}

static void op_regadd(CHIP8 chip) {
  /* printf("REG ADD OP=%04X PC=%04X\n", chip->opcode, chip->pc); */
  uint8_t reg = (uint8_t)(chip->opcode >> 8) & 0x000F;
  uint8_t value = (uint8_t)(chip->opcode & 0x00FF);

  /* printf("V%X = %04X prev=%04X\n", reg, value, chip->regs[reg]); */
  chip->regs[reg] += value;
  /* printf("V%X = %04X\n", reg, chip->regs[reg]); */
}

static void op_clrvram(CHIP8 chip) {
  /* printf("CLR OP=%04X PC=%04X\n", chip->opcode, chip->pc); */
  for (uint16_t i = 0; i < VMEM_SIZE; i++)
    chip->vram[i] = 0;
}

static void op_ret(CHIP8 chip) {
  if (chip->sp > 0)
    chip->pc = chip->stack[chip->sp--];
}

static void op_calladdr(CHIP8 chip) {
  chip->stack[++chip->sp] = chip->pc;
  chip->pc = chip->opcode & 0x0FFF;
}

static void op_regse(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8) & 0x000F;
  uint8_t value = (uint8_t)(chip->opcode & 0x00FF);

  if (chip->regs[x] == value)
    chip->pc += 2;
}

static void op_regsne(CHIP8 chip) {
  uint8_t reg = (uint8_t)(chip->opcode >> 8) & 0x000F;
  uint8_t value = (uint8_t)(chip->opcode & 0x00FF);

  if (chip->regs[reg] != value)
    chip->pc += 2;
}

static void op_regsse(CHIP8 chip) {
  uint8_t regx = (uint8_t)(chip->opcode >> 8) & 0x000F;
  uint8_t regy = (uint8_t)(chip->opcode >> 4) & 0x000F;

  if (chip->regs[regx] == chip->regs[regy])
    chip->pc += 2;
}

static void op_regssne(CHIP8 chip) {
  uint8_t regx = (uint8_t)(chip->opcode >> 8) & 0x000F;
  uint8_t regy = (uint8_t)(chip->opcode >> 4) & 0x000F;

  if (chip->regs[regx] != chip->regs[regy])
    chip->pc += 2;
}

static void op_arlog(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8) & 0x000F;
  uint8_t y = (uint8_t)(chip->opcode >> 4) & 0x000F;

  switch (chip->opcode & 0x000F) {
  case 0x0:
    chip->regs[x] = chip->regs[y];
    break;
  case 0x1:
    chip->regs[x] |= chip->regs[y];
    break;
  case 0x2:
    chip->regs[x] &= chip->regs[y];
    break;
  case 0x3:
    chip->regs[x] ^= chip->regs[y];
    break;
  case 0x4:
    chip->regs[0xF] = 255 - chip->regs[x] < chip->regs[y] ? 1 : 0;
    chip->regs[x] += chip->regs[y];
    break;
  case 0x5:
    chip->regs[0xF] = chip->regs[x] > chip->regs[y] ? 1 : 0;
    chip->regs[x] -= chip->regs[y];
    break;
  case 0x6:
    chip->regs[0xF] = chip->regs[x] & 0x1;
    chip->regs[x] >>= 1;
    break;
  case 0x7:
    chip->regs[0xF] = chip->regs[y] > chip->regs[x] ? 1 : 0;
    chip->regs[x] = chip->regs[y] - chip->regs[x];
    break;
  case 0xE:
    chip->regs[0xF] = (chip->regs[x] >> 7) & 0x1;
    chip->regs[x] <<= 1;
    break;
  default:
    break;
  }
}

static void op_ldi(CHIP8 chip) {
  /* printf("LDI OP=%04X PC=%04X\n", chip->opcode, chip->pc); */
  chip->index = chip->opcode & 0x0FFF;
}

static void op_jmpv0(CHIP8 chip) {
  chip->pc = (chip->opcode & 0x0FFF) + chip->regs[0x0];
}

static void op_rnd(CHIP8 chip) {
  uint8_t rv = (uint8_t)(rand() / (RAND_MAX / 256));
  uint8_t x = (uint8_t)(chip->opcode >> 8) & 0x000F;
  chip->regs[x] &= rv;
}

static void op_display(CHIP8 chip) {
  uint8_t vx = chip->regs[(uint8_t)(chip->opcode >> 8) & 0x000F];
  uint8_t vy = chip->regs[(uint8_t)(chip->opcode >> 4) & 0x000F];
  uint8_t rows = (uint8_t)(chip->opcode & 0x000F);
  /* printf("Display: X=%04X Y=%04X I=%04X n=%d\n", vx, vy, chip->index, rows);
   */
  vx &= 63;
  vy &= 31;
  chip->regs[0xF] = 0;

  /* printf("Display: X=%04X Y=%04X I=%04X n=%d\n", vx, vy, chip->index, rows);
   */
  uint8_t pixel, i, j;
  uint16_t posx = 0, posy = 0;
  for (j = 0; j < rows; j++) {
    pixel = chip->mem[chip->index + (uint16_t)j];
    /* printf("Display: mem addr=%04X val=%08b\n", chip->index + (uint16_t) j,
     * pixel); */
    posy = vy + j;
    posy &= 31;
    for (i = 0; i < 8; i++) {
      if (pixel & 0x80) {
        posx = vx + i;
        posx &= 63;
        if (chip->vram[posy * 64 + posx] == 1)
          chip->regs[0xF] = 1;
        chip->vram[posy * 64 + posx] ^= 1;
        /* printf("POS: (%d,%d) V=%d\n", posx, posy, chip->vram[posy * 64 +
         * posx]); */
      }

      pixel <<= 1;
    }
  }
}

static void op_skp(CHIP8 chip) {
  uint8_t vx = (chip->opcode & 0x0F00) >> 8;
  uint8_t key = chip->regs[vx] & 0xF;

  if (chip->input & (1 << key))
    chip->pc += 2;
}

static void op_sknp(CHIP8 chip) {
  uint8_t vx = (chip->opcode & 0x0F00) >> 8;
  uint8_t key = chip->regs[vx] & 0xF;

  if (!(chip->input & (1 << key)))
    chip->pc += 2;
}

static void op_lddt(CHIP8 chip) {
  uint8_t vx = (chip->opcode & 0x0F00) >> 8;
  chip->regs[vx] = chip->dt;
  /* printf("%d %d %d\n", vx, chip->regs[vx], chip->dt); */
}

static void op_waitkey(CHIP8 chip) {
  if (chip->input) {
    uint8_t vx = (chip->opcode & 0x0F00) >> 8;
    chip->regs[vx] = chip->input_key;
    /* printf("%d %d\n", vx, chip->input_key); */
  } else {
    /* printf("%016b %d\n", chip->input, chip->input_key); */
    chip->pc -= 2;
  }
}

static void op_setdt(CHIP8 chip) {
  uint8_t vx = (chip->opcode & 0x0F00) >> 8;
  chip->dt = chip->regs[vx];
}

static void op_setst(CHIP8 chip) {
  uint8_t vx = (chip->opcode & 0x0F00) >> 8;
  chip->st = chip->regs[vx];
}

static void op_ldf(CHIP8 chip) {
  uint8_t vx = (chip->opcode & 0x0F00) >> 8;
  chip->index = (chip->regs[vx] & 0xF) * 5;
}

static void op_ldb(CHIP8 chip) {
  uint8_t x = (chip->opcode & 0x0F00) >> 8;
  uint8_t i = 3;
  uint8_t vx = chip->regs[x];

  while (i) {
    chip->mem[chip->index + i - 1] = vx % 10;
    i--;
    vx /= 10;
  }
}

static void op_ldix(CHIP8 chip) {
  uint8_t x = ((chip->opcode & 0x0F00) >> 8) & 0xF;

  for (int i = 0; i <= x; i++) {
    chip->mem[chip->index + i] = chip->regs[i];
  }
}

static void op_ldxi(CHIP8 chip) {
  uint8_t x = ((chip->opcode & 0x0F00) >> 8) & 0xF;

  for (int i = 0; i <= x; i++) {
    chip->regs[i] = chip->mem[chip->index + i];
  }
}

static void op_addi(CHIP8 chip) {
  uint8_t vx = (chip->opcode & 0x0F00) >> 8;
  chip->index += chip->regs[vx];
}

static void fetch(CHIP8 chip) {
  uint16_t op_h, op_l;

  op_h = chip->mem[chip->pc] << 8;
  op_l = chip->mem[chip->pc + 1];
  chip->opcode = op_h | op_l;
  /* printf("OP=%04X PC=%4X\n", chip->opcode, chip->pc); */
  chip->pc += 2;
}

static void op_ignore(CHIP8 chip) {}

static void decode(CHIP8 chip) {
  /* printf("OP=%4X PC=%4X\n", chip->opcode, chip->pc); */
  switch (chip->opcode & 0xF000) {
  case 0x0000:
    switch (chip->opcode & 0x00FF) {
    case 0x00E0:
      chip->exec = &op_clrvram;
      break;
    case 0x00EE:
      chip->exec = &op_ret;
      break;
    default:
      chip->exec = &op_ignore;
      break;
    }
    break;
  case 0x1000:
    chip->exec = &op_jmp;
    break;
  case 0x2000:
    chip->exec = &op_calladdr;
    break;
  case 0x3000:
    chip->exec = &op_regse;
    break;
  case 0x4000:
    chip->exec = &op_regsne;
    break;
  case 0x5000:
    chip->exec = &op_regsse;
    break;
  case 0x6000:
    chip->exec = &op_regload;
    break;
  case 0x7000:
    chip->exec = &op_regadd;
    break;
  case 0x8000:
    chip->exec = &op_arlog;
    break;
  case 0x9000:
    chip->exec = &op_regssne;
    break;
  case 0xA000:
    chip->exec = &op_ldi;
    break;
  case 0xB000:
    chip->exec = &op_jmpv0;
    break;
  case 0xC000:
    chip->exec = &op_rnd;
    break;
  case 0xD000:
    chip->exec = &op_display;
    break;
  case 0xE000:
    switch (chip->opcode & 0x00FF) {
    case 0x009E:
      chip->exec = &op_skp;
      break;
    case 0x00A1:
      chip->exec = &op_sknp;
      break;
    default:
      chip->exec = &op_unsupported;
      break;
    }
    break;
  case 0xF000:
    switch (chip->opcode & 0x00FF) {
    case 0x0007:
      chip->exec = &op_lddt;
      break;
    case 0x000A:
      chip->exec = &op_waitkey;
      break;
    case 0x0015:
      chip->exec = &op_setdt;
      break;
    case 0x0018:
      chip->exec = &op_setst;
      break;
    case 0x001E:
      chip->exec = &op_addi;
      break;
    case 0x0029:
      chip->exec = &op_ldf;
      break;
    case 0x0033:
      chip->exec = &op_ldb;
      break;
    case 0x0055:
      chip->exec = &op_ldix;
      break;
    case 0x0065:
      chip->exec = &op_ldxi;
      break;
    default:
      chip->exec = &op_unsupported;
      break;
    }
    break;
  default:
    chip->exec = &op_unsupported;
    break;
  }
}

static void execute(CHIP8 chip) { chip->exec(chip); }
