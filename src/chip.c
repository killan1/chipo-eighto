#include "chip.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct chip8 {
  uint16_t pc;
  uint8_t *mem;
  uint8_t *vram;
  size_t vram_size;
  uint8_t screen_width;
  uint8_t screen_height;
  uint16_t stack[STACK_SIZE];

  uint16_t index;
  uint8_t dt;
  uint8_t st;
  uint8_t sp;
  uint8_t regs[REGS_COUNT];

  uint16_t input;
  uint8_t input_key;

  uint16_t opcode;
  void (*exec)(CHIP8);

  uint8_t quirks;
#ifdef SUPER_CHIP
  bool hires_mode_enabled;
#endif
};

static void fetch(CHIP8);
static void decode(CHIP8);
static void execute(CHIP8);

CHIP8 chip_init(ChipConfig conf) {
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

  size_t vram_size = VRAM_SIZE;

  chip->vram = calloc(vram_size, sizeof(uint8_t));
  if (chip->vram == NULL) {
    chip_destroy(chip);
    terminate("Failed to allocate memory");
  }

  chip->vram_size = vram_size;
  chip->screen_width = SCREEN_WIDTH;
  chip->screen_height = SCREEN_HEIGHT;
  chip->pc = START_ADDRESS;
  chip->index = 0;
  chip->dt = 0;
  chip->st = 0;
  chip->sp = 0;
  chip->input = 0;
  chip->input_key = 0;
  chip->quirks = conf.quirks;
#ifdef SUPER_CHIP
  chip->hires_mode_enabled = false;
#endif

  uint8_t i;
  for (i = 0; i < REGS_COUNT; i++) {
    chip->regs[i] = 0;
  }
  for (i = 0; i < STACK_SIZE; i++) {
    chip->stack[i] = 0;
  }
  for (i = 0; i < 80; i++) {
    chip->mem[i] = font[i];
  }
#ifdef SUPER_CHIP
  for (i = 0; i < 100; i++) {
    chip->mem[WIDE_FONTS_START_ADDRESS + i] = wide_font[i];
  }
#endif

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
uint8_t chip_get_screen_width(CHIP8 chip) { return chip->screen_width; }
uint8_t chip_get_screen_height(CHIP8 chip) { return chip->screen_height; }

void chip_update_timers(CHIP8 chip) {
  if (chip->dt)
    chip->dt--;
  if (chip->st)
    chip->st--;
}

bool chip_is_sound_timer_active(CHIP8 chip) { return chip->st > 0; }

static void set_vram_data_at_pos(CHIP8 chip, size_t x, size_t y,
                                 bool wide_pixel_mode) {
  size_t screen_width = (size_t)chip->screen_width;
  if (chip->vram[y * screen_width + x])
#ifdef SUPER_CHIP
    chip->regs[0xF] += 1;
#else
    chip->regs[0xF] = 1;
#endif
  chip->vram[y * screen_width + x] ^= 1;

  if (wide_pixel_mode) {
    chip->vram[y * screen_width + x + 1] ^= 1;
    chip->vram[(y + 1) * screen_width + x] ^= 1;
    chip->vram[(y + 1) * screen_width + x + 1] ^= 1;
  }
}

static void opcode_unsupported(CHIP8 chip) {
  printf("WARNING: unsupported opcode %04X\n", chip->opcode);
  exit(EXIT_FAILURE);
}

static void opcode_1xxx(CHIP8 chip) { chip->pc = chip->opcode & 0xFFF; }

static void opcode_6xkk(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t value = (uint8_t)(chip->opcode & 0xFF);

  chip->regs[x] = value;
}

static void opcode_7xkk(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t value = (uint8_t)(chip->opcode & 0xFF);

  chip->regs[x] += value;
}

static void opcode_00E0(CHIP8 chip) {
  for (uint16_t i = 0; i < chip->vram_size; i++)
    chip->vram[i] = 0;
}

static void opcode_00EE(CHIP8 chip) {
  if (chip->sp > 0)
    chip->pc = chip->stack[chip->sp--];
}

static void opcode_2nnn(CHIP8 chip) {
  chip->stack[++chip->sp] = chip->pc;
  chip->pc = chip->opcode & 0xFFF;
}

static void opcode_3xkk(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t value = (uint8_t)(chip->opcode & 0xFF);

  if (chip->regs[x] == value)
    chip->pc += 2;
}

static void opcode_4xkk(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t value = (uint8_t)(chip->opcode & 0xFF);

  if (chip->regs[x] != value)
    chip->pc += 2;
}

static void opcode_5xy0(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  if (chip->regs[x] == chip->regs[y])
    chip->pc += 2;
}

static void opcode_9xy0(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  if (chip->regs[x] != chip->regs[y])
    chip->pc += 2;
}

static void opcode_8xy0(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  chip->regs[x] = chip->regs[y];
}

static void opcode_8xy1(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  chip->regs[x] |= chip->regs[y];
  if ((chip->quirks & VF_RESET))
    chip->regs[0xF] = 0;
}

static void opcode_8xy2(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  chip->regs[x] &= chip->regs[y];
  if ((chip->quirks & VF_RESET))
    chip->regs[0xF] = 0;
}

static void opcode_8xy3(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  chip->regs[x] ^= chip->regs[y];
  if ((chip->quirks & VF_RESET))
    chip->regs[0xF] = 0;
}

static void opcode_8xy4(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  uint8_t vf = 255 - chip->regs[x] < chip->regs[y] ? 1 : 0;
  chip->regs[x] += chip->regs[y];
  chip->regs[0xF] = vf;
}

static void opcode_8xy5(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  uint8_t vf = chip->regs[x] >= chip->regs[y] ? 1 : 0;
  chip->regs[x] -= chip->regs[y];
  chip->regs[0xF] = vf;
}

static void opcode_8xy6(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t vf;

  if (!(chip->quirks & SHIFTING)) {
    uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);
    chip->regs[x] = chip->regs[y];
  }

  vf = chip->regs[x] & 0x1;
  chip->regs[x] >>= 1;
  chip->regs[0xF] = vf;
}

static void opcode_8xy7(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);

  uint8_t vf = chip->regs[y] >= chip->regs[x] ? 1 : 0;
  chip->regs[x] = chip->regs[y] - chip->regs[x];
  chip->regs[0xF] = vf;
}

static void opcode_8xyE(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t vf;

  if (!(chip->quirks & SHIFTING)) {
    uint8_t y = (uint8_t)(chip->opcode >> 4 & 0xF);
    chip->regs[x] = chip->regs[y];
  }

  vf = (chip->regs[x] >> 7) & 0x1;
  chip->regs[x] <<= 1;
  chip->regs[0xF] = vf;
}

static void opcode_Annn(CHIP8 chip) { chip->index = chip->opcode & 0xFFF; }

static void opcode_Bnnn(CHIP8 chip) {
  uint8_t x = 0x0;

  if (chip->quirks & JUMPING)
    x = (uint8_t)(chip->opcode >> 8 & 0xF);

  chip->pc = (chip->opcode & 0xFFF) + chip->regs[x];
}

static void opcode_Cxkk(CHIP8 chip) {
  uint8_t rv = (uint8_t)(rand() / (RAND_MAX / 256));
  uint8_t x = (uint8_t)(chip->opcode >> 8) & 0xF;
  uint8_t value = (uint8_t)(chip->opcode & 0xFF);

  chip->regs[x] = rv & value;
}

static void opcode_Dxyn(CHIP8 chip) {
  uint8_t vx = chip->regs[(uint8_t)(chip->opcode >> 8 & 0xF)];
  uint8_t vy = chip->regs[(uint8_t)(chip->opcode >> 4 & 0xF)];

  uint8_t screen_width = chip->screen_width;
  uint8_t screen_height = chip->screen_height;
  uint8_t x = vx & (screen_width - 1);
  uint8_t y = vy & (screen_height - 1);

  uint16_t posx = 0, posy = 0, rows = chip->opcode & 0xF;
  uint8_t sprite_data;

  chip->regs[0xF] = 0;
#ifdef SUPER_CHIP
  for (uint16_t row = 0; row < rows; row++) {
    sprite_data = chip->mem[chip->index + row];

    posy = y + row;

    if (!chip->hires_mode_enabled) {
      posy *= 2;
    }

    if (posy > (screen_height - 1)) {
      if (chip->quirks & CLIPPING)
        continue;
      else
        posy %= screen_height;
    }

    for (uint8_t col = 0; col < SPRITE_SIZE; col++) {
      posx = x + col;

      if (!chip->hires_mode_enabled) {
        posx *= 2;
      }

      if (posx > (screen_width - 1)) {
        if (chip->quirks & CLIPPING)
          continue;
        else
          posx %= screen_width;
      }

      if (sprite_data & 1 << (SPRITE_SIZE - 1 - col)) {
        set_vram_data_at_pos(chip, posx, posy, !chip->hires_mode_enabled);
      }
    }
  }
#else
  for (uint16_t row = 0; row < rows; row++) {
    sprite_data = chip->mem[chip->index + row];

    posy = y + row;

    if (posy > (screen_height - 1)) {
      if (chip->quirks & CLIPPING)
        continue;
      else
        posy %= screen_height;
    }

    for (uint8_t col = 0; col < SPRITE_SIZE; col++) {
      posx = x + col;

      if (posx > (screen_width - 1)) {
        if (chip->quirks & CLIPPING)
          continue;
        else
          posx %= screen_width;
      }

      if (sprite_data & 1 << (SPRITE_SIZE - 1 - col)) {
        set_vram_data_at_pos(chip, posx, posy, false);
      }
    }
  }
#endif
}

static void opcode_Ex9E(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t vx = chip->regs[x] & 0xF;

  if (chip->input & (1 << vx))
    chip->pc += 2;
}

static void opcode_ExA1(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t vx = chip->regs[x] & 0xF;

  if (!(chip->input & (1 << vx)))
    chip->pc += 2;
}

static void opcode_Fx07(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  chip->regs[x] = chip->dt;
}

static void opcode_Fx0A(CHIP8 chip) {
  if (chip->input) {
    uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
    chip->regs[x] = chip->input_key;
  } else {
    chip->pc -= 2;
  }
}

static void opcode_Fx15(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  chip->dt = chip->regs[x];
}

static void opcode_Fx18(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  chip->st = chip->regs[x];
}

static void opcode_Fx29(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  chip->index = (chip->regs[x] & 0xF) * 5;
}

static void opcode_Fx33(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  uint8_t i = 3;
  uint8_t vx = chip->regs[x];

  while (i) {
    chip->mem[chip->index + i - 1] = vx % 10;
    i--;
    vx /= 10;
  }
}

static void opcode_Fx55(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);

  for (int i = 0; i <= x; i++)
    chip->mem[chip->index + i] = chip->regs[i];

  if (chip->quirks & MEMORY)
    chip->index += x + 1;
}

static void opcode_Fx65(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);

  for (int i = 0; i <= x; i++)
    chip->regs[i] = chip->mem[chip->index + i];

  if (chip->quirks & MEMORY)
    chip->index += x + 1;
}

static void opcode_Fx1E(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  chip->index += chip->regs[x];
}

#ifdef SUPER_CHIP
static void opcode_00FF(CHIP8 chip) { chip->hires_mode_enabled = true; }

static void opcode_00FE(CHIP8 chip) { chip->hires_mode_enabled = false; }

static void opcode_00Cn(CHIP8 chip) {
  int16_t x, y, rows = chip->opcode & 0xF;

  if (!chip->hires_mode_enabled)
    rows *= 2;

  for (x = 0; x < chip->screen_width; x++)
    for (y = chip->screen_height - 1; y >= 0; y--) {
      chip->vram[y * chip->screen_width + x] =
          y < rows ? 0 : chip->vram[(y - rows) * chip->screen_width + x];
    }
}

static void opcode_00FB(CHIP8 chip) {
  int16_t x, y, pixels = chip->hires_mode_enabled ? 4 : 8;

  for (x = chip->screen_width - 1; x >= 0; x--)
    for (y = 0; y < chip->screen_height; y++) {
      chip->vram[y * chip->screen_width + x] =
          x < pixels ? 0 : chip->vram[y * chip->screen_width + x - pixels];
    }
}

static void opcode_00FC(CHIP8 chip) {
  uint16_t x, y, pixels = chip->hires_mode_enabled ? 4 : 8;

  for (x = 0; x < chip->screen_width; x++)
    for (y = 0; y < chip->screen_height; y++) {
      chip->vram[y * chip->screen_width + x] =
          x > chip->screen_width - 1 - pixels
              ? 0
              : chip->vram[y * chip->screen_width + x + pixels];
    }
}

static void opcode_Dxy0(CHIP8 chip) {
  uint8_t vx = chip->regs[(uint8_t)(chip->opcode >> 8 & 0xF)];
  uint8_t vy = chip->regs[(uint8_t)(chip->opcode >> 4 & 0xF)];

  uint8_t screen_width = chip->screen_width;
  uint8_t screen_height = chip->screen_height;
  uint8_t x = vx & (screen_width - 1);
  uint8_t y = vy & (screen_height - 1);

  uint16_t posx = 0, posy = 0;
  uint16_t sprite_data;

  chip->regs[0xF] = 0;
  for (uint16_t row = 0; row < WIDE_SPRITE_SIZE; row++) {
    sprite_data = chip->mem[chip->index + row * 2] << 8 |
                  chip->mem[chip->index + row * 2 + 1];

    posy = y + row;

    if (!chip->hires_mode_enabled) {
      posy *= 2;
    }

    if (posy > (screen_height - 1)) {
      if (chip->quirks & CLIPPING)
        continue;
      else
        posy %= screen_height;
    }

    for (uint16_t col = 0; col < WIDE_SPRITE_SIZE; col++) {
      posx = x + col;

      if (!chip->hires_mode_enabled) {
        posx *= 2;
      }

      if (posx > (screen_width - 1)) {
        if (chip->quirks & CLIPPING)
          continue;
        else
          posx %= screen_width;
      }

      if (sprite_data & 1 << (WIDE_SPRITE_SIZE - 1 - col)) {
        set_vram_data_at_pos(chip, posx, posy, !chip->hires_mode_enabled);
      }
    }
  }
}

static void opcode_Fx30(CHIP8 chip) {
  uint8_t x = (uint8_t)(chip->opcode >> 8 & 0xF);
  chip->index = WIDE_FONTS_START_ADDRESS + (chip->regs[x] & 0xF) * 10;
}

static void opcode_Fx75(CHIP8 chip) {}
static void opcode_Fx85(CHIP8 chip) {}

static void opcode_00FD(CHIP8 chip) {
  chip_destroy(chip);
  terminate("Executing 00FD. Bye.");
}
#endif

static void opcode_nop(CHIP8 chip) {}

static void fetch(CHIP8 chip) {
  uint16_t op_h, op_l;

  op_h = chip->mem[chip->pc] << 8;
  op_l = chip->mem[chip->pc + 1];
  chip->opcode = op_h | op_l;
  chip->pc += 2;
}

static void decode(CHIP8 chip) {
  switch (chip->opcode & 0xF000) {
  case 0x0000:
    switch (chip->opcode & 0x00FF) {
    case 0x00E0:
      chip->exec = &opcode_00E0;
      break;
    case 0x00EE:
      chip->exec = &opcode_00EE;
      break;
#ifdef SUPER_CHIP
    case 0x00FF:
      chip->exec = &opcode_00FF;
      break;
    case 0x00FE:
      chip->exec = &opcode_00FE;
      break;
    case 0x00FB:
      chip->exec = &opcode_00FB;
      break;
    case 0x00FC:
      chip->exec = &opcode_00FC;
      break;
    case 0x00FD:
      chip->exec = &opcode_00FD;
      break;
    case 0x00C0:
    case 0x00C1:
    case 0x00C2:
    case 0x00C3:
    case 0x00C4:
    case 0x00C5:
    case 0x00C6:
    case 0x00C7:
    case 0x00C8:
    case 0x00C9:
    case 0x00CA:
    case 0x00CB:
    case 0x00CC:
    case 0x00CD:
    case 0x00CE:
    case 0x00CF:
      chip->exec = &opcode_00Cn;
      break;
#endif
    default:
      chip->exec = &opcode_nop;
      break;
    }
    break;
  case 0x1000:
    chip->exec = &opcode_1xxx;
    break;
  case 0x2000:
    chip->exec = &opcode_2nnn;
    break;
  case 0x3000:
    chip->exec = &opcode_3xkk;
    break;
  case 0x4000:
    chip->exec = &opcode_4xkk;
    break;
  case 0x5000:
    chip->exec = (chip->opcode & 0xF) == 0 ? &opcode_5xy0 : &opcode_nop;
    break;
  case 0x6000:
    chip->exec = &opcode_6xkk;
    break;
  case 0x7000:
    chip->exec = &opcode_7xkk;
    break;
  case 0x8000: {
    switch (chip->opcode & 0xF) {
    case 0x0:
      chip->exec = &opcode_8xy0;
      break;
    case 0x1:
      chip->exec = &opcode_8xy1;
      break;
    case 0x2:
      chip->exec = &opcode_8xy2;
      break;
    case 0x3:
      chip->exec = &opcode_8xy3;
      break;
    case 0x4:
      chip->exec = &opcode_8xy4;
      break;
    case 0x5:
      chip->exec = &opcode_8xy5;
      break;
    case 0x6:
      chip->exec = &opcode_8xy6;
      break;
    case 0x7:
      chip->exec = &opcode_8xy7;
      break;
    case 0xE:
      chip->exec = &opcode_8xyE;
      break;
    default:
      chip->exec = &opcode_unsupported;
    }
    break;
  }
  case 0x9000:
    chip->exec = (chip->opcode & 0xF) == 0 ? &opcode_9xy0 : &opcode_nop;
    break;
  case 0xA000:
    chip->exec = &opcode_Annn;
    break;
  case 0xB000:
    chip->exec = &opcode_Bnnn;
    break;
  case 0xC000:
    chip->exec = &opcode_Cxkk;
    break;
  case 0xD000:
#ifdef SUPER_CHIP
    chip->exec = (chip->opcode & 0xF) == 0 ? &opcode_Dxy0 : &opcode_Dxyn;
#else
    chip->exec = &opcode_Dxyn;
#endif
    break;
  case 0xE000:
    switch (chip->opcode & 0xFF) {
    case 0x009E:
      chip->exec = &opcode_Ex9E;
      break;
    case 0x00A1:
      chip->exec = &opcode_ExA1;
      break;
    default:
      chip->exec = &opcode_unsupported;
      break;
    }
    break;
  case 0xF000:
    switch (chip->opcode & 0xFF) {
    case 0x0007:
      chip->exec = &opcode_Fx07;
      break;
    case 0x000A:
      chip->exec = &opcode_Fx0A;
      break;
    case 0x0015:
      chip->exec = &opcode_Fx15;
      break;
    case 0x0018:
      chip->exec = &opcode_Fx18;
      break;
    case 0x001E:
      chip->exec = &opcode_Fx1E;
      break;
    case 0x0029:
      chip->exec = &opcode_Fx29;
      break;
    case 0x0033:
      chip->exec = &opcode_Fx33;
      break;
    case 0x0055:
      chip->exec = &opcode_Fx55;
      break;
    case 0x0065:
      chip->exec = &opcode_Fx65;
      break;
#ifdef SUPER_CHIP
    case 0x0030:
      chip->exec = &opcode_Fx30;
      break;
    case 0x0075:
      chip->exec = &opcode_Fx75;
      break;
    case 0x0085:
      chip->exec = &opcode_Fx85;
      break;
#endif
    default:
      chip->exec = &opcode_unsupported;
      break;
    }
    break;
  default:
    chip->exec = &opcode_unsupported;
    break;
  }
}

static void execute(CHIP8 chip) { chip->exec(chip); }
