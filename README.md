# chipo-eighto or simply chipo8o
A simple Chip-8 and Super-Chip Interpreter

## Table of Contents

- [Dependencies](#dependencies)
- [Build](#build)
- [Usage](#usage)
- [Keyboard](#keyboard)
- [License](#license)

## Dependencies
* GNU C Compiler
* GNU Make
* [Raylib](https://www.raylib.com/index.html) installed on your machine

## Build
To build a debug build of a Chip-8 interpreter run:
```bash
make debug
```
for a release build run:
```bash
make release
```
In order to build a Super-Chip version run:
```bash
CHIP_BACKEND=super-chip make debug
```

The executable file will be placed in the `target/{debug|release}/bin` directory.
## Usage
To run a rom:
```bash
chipo8o path/to/rom [options]
```

### Quirks
Due to different implementations and ambiguous behavior of the instructions, some roms may require different behavior. If a rom is acting strangely, try toggling such `quirks` with the --quirk option:
```bash
chipo8o path/to/rom -q [quirk-name]
```
Available quirks:
```
vfreset  - set VF register to zero for 8XY1, 8XY2, 8XY3 instructions
memory   - don't modify index register for FX55, FX65 instructions
display  - limiting sprites drawing by 60 per frame (do nothing for now)
clipping - clip sprites instead of wrapping around to the top of the screen
shifting - ignore VY register and 8XY6, 8XYE instructions and directly modify VX register.
jumping  - add VX register instead of V0 to address for BNNN instruction
```
You can combine these quirks together:
```bash
chipo8o path/to/rom -q vfreset --quirk memory -q clipping
```

### Colors
It is possible to change the background and foreground colors using the following options:
```bash
chipo8o path/to/rom --bg=100,100,100,255 --fg=50,0,128,255
```

## Keyboard
### CHIP-8 layout
|   |   |   |   |
|---|---|---|---|
| 1 | 2 | 3 | C |
| 4 | 5 | 6 | D |
| 7 | 8 | 9 | E |
| A | 0 | B | F |

### chipo8o layout
|   |   |   |   |
|---|---|---|---|
| 1 | 2 | 3 | 4 |
| Q | W | E | R |
| A | S | D | F |
| Z | X | C | V |

### Additional key bindings
| Key | Description |
|-----|-------------|
|  -  | Reduce chip frequency (ops/frame) by 50, but not less than 20 |
|  =  | Increase chip frequency (ops/frame) by 50, but not more than UINT16_MAX |
|  `  | Toggle FPS counter |

## License
This project is open source and available under the [The Unlicense License](LICENSE).
