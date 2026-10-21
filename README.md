# DEEPRED Instruction Set Architecture

A 32-bit, ARM LEGv8-inspired instruction set architecture (ISA) simulator written in C++20. It was built as the final project for **CS535** by James Dolle and Pablo Ghezzi. It includes a pipeline simulator, a Qt desktop UI, and a Python assembler.

![Image Example of Project](example.jpg)


## Table of contents

- [Overview](#overview)
- [Project status](#project-status)
- [Repository layout](#repository-layout)
- [Building](#building)
- [Using the desktop simulator](#using-the-desktop-simulator)
- [Architecture reference](#architecture-reference)
- [Memory and cache design](#memory-and-cache-design)
- [PPU (graphics)](#ppu-graphics)
- [Planned benchmarks](#planned-benchmarks)
- [Documentation](#documentation)
- [License](#license)

## Overview

DEEPRED is a general-purpose, signed-integer ISA heavily based on ARM LEGv8, simplified for simulation:

- 32-bit instructions and a 32-bit word size
- 16 registers (13 general purpose, stack pointer, program counter, zero register)
- Byte-addressed 256 KiB memory with byte and word load/store
- Split direct-mapped, write-back L1 instruction and data caches in front of RAM
- An SDL3 PPU (picture processing unit) backed by simulated memory
- A Qt 6 desktop UI for registers, pipeline stages, caches, RAM, and controls

## Project status

| Component | Status |
|---|---|
| CPU pipeline, instruction decoder, execution and write-back (`src/simulator/cpu.cpp`) | Implemented |
| RAM, bus, split L1 caches and latency model (`src/simulator/memory.cpp`) | Implemented |
| Qt control and inspection UI (`src/simulator/cpu.cpp`) | Implemented |
| Worker thread for run, step, reset and program loading (`src/simulator/worker_pipe.cpp`) | Implemented |
| Memory-backed SDL3 PPU (`src/simulator/include/ppu.h`) | Implemented |
| Assembler source and sample binaries (`programs/`) | Included |

## Repository layout

```
DEEPRED_ISA/
├── src/
│   └── simulator/
│       ├── main.cpp           # Qt application entry point
│       ├── cpu.cpp            # Pipeline simulator and UI implementation
│       ├── memory.cpp         # RAM, bus and cache implementation
│       ├── worker_pipe.cpp    # UI worker-thread actions
│       └── include/           # CPU, memory, PPU and ISA headers
├── programs/              # Sample binaries and assembler source
├── CMakeLists.txt
├── CS535_Final_Report_Draft.pdf  # Full ISA specification and design report
├── Makefile
└── LICENSE
```

## Building

### Requirements

- A C++20 compiler
- `make`
- CMake 3.21 or newer
- Qt 6 Widgets development files
- SDL3 development files

On Debian/Ubuntu: `sudo apt-get install build-essential cmake pkg-config qt6-base-dev libsdl3-dev`.

### Commands

```bash
# Configure and build the Qt desktop simulator
make

# Open the simulator UI
make run

# Compile every target without launching an executable (the CI command)
make clean check

# Remove generated executables
make clean
```

The GUI executable is `build/deepred`. The GitHub Actions workflow installs Qt 6
and SDL3, runs `make clean check`, and verifies that it was produced.

## Using the desktop simulator

Run `make run` from a graphical desktop session. In the UI, enter the path to a
binary program (for example `programs/add.bin`) and select **Load Mem**. Then use
**Single-Step**, **Multi-Step**, or **Run The Pipe**. The checkboxes toggle pipeline
and cache behavior; the tables show instruction/data caches, registers, RAM, and
pipeline state.

## Architecture reference

### Registers

| Name | Number | Use |
|---|---|---|
| X0 to X12 | 0 to 12 | General purpose |
| X13 (SP) | 13 | Stack pointer |
| X14 (PC) | 14 | Program counter |
| XZR | 15 | Zero register |

Condition flags (NZVC): **N** negative, **Z** zero, **V** overflow, **C** carry.

### Instruction formats

All instructions are one 32-bit word. The top three bits select the format.

| Format | Bits [31:29] | Fields |
|---|---|---|
| H (halt) | `000` | Any invalid instruction halts |
| R | `001` | Opcode, Rm, shamt, Rn, Rd |
| I | `010` | Opcode, ALU immediate, Rn, Rd |
| D | `010` | Opcode, DT address, Rn, Rt |
| B | `011` | Opcode, branch address |
| CB | `100` | Opcode, conditional branch address |
| IW | `111` | Opcode, move immediate, SH, Rd |
| ME | `110` | Opcode, immediate / Rt |

> The report's format table and per-instruction listings differ slightly in a few places (for example, I and D share `010`, and B/CB/IW/ME encodings vary between sections). Treat `CS535_Final_Report_Draft.pdf` as the working source and verify encodings there before building an assembler or decoder.

### Instruction set

**Arithmetic and logic (R format)**

| Mnemonic | Operation | Flags |
|---|---|---|
| `ADD` | `Rd = Rn + Rm` | N Z V C |
| `SUB` | `Rd = Rn - Rm` | N Z V C |
| `MUL` | `Rd = Rn * Rm` | N Z V C |
| `DIV` | `Rd = Rn / Rm` | N Z V C |
| `MOD` | `Rd = Rn % Rm` | N Z V C |
| `AND` | `Rd = Rn & Rm` | N Z |
| `ORR` | `Rd = Rn \| Rm` | N Z |
| `EOR` | `Rd = Rn ^ Rm` | N Z |
| `LSL` | `Rd = Rn << shamt` | Z V C |
| `LSR` | `Rd = Rn >>> shamt` | Z V C |
| `BR` | `PC = R[Rt]` | none |

**Immediate arithmetic and logic (I format):** `ADDI`, `SUBI`, `ANDI`, `ORRI`, `EORI`

**Load/store (D format)**

| Mnemonic | Operation |
|---|---|
| `LDUR` | `Rt = M[Rn + DTAddr]` (word) |
| `LDURB` | `Rt = zero-extended byte at M[Rn + DTAddr]` |
| `STUR` | `M[Rn + DTAddr] = Rt` (word) |
| `STURB` | store low byte of `Rt` |

**Branches**

| Mnemonic | Condition |
|---|---|
| `B` | Unconditional, PC-relative |
| `BL` | Branch with link (return address saved) |
| `BEQ` | `Z == 1` |
| `BNE` | `Z == 0` |
| `BLT` | `N != V` |
| `BLE` | `Z == 1` or `N != V` |
| `BGT` | `Z == 0` and `N == V` |
| `BGE` | `N == V` |

For conditional branches, set the flags first (for example `CMP A, B`), then branch on the condition.

**Wide moves (IW format):** `MOVK` (keep other bits), `MOVZ` (zero other bits)

**Miscellaneous (ME format):** `PPU` (call the PPU), `FLU` (flush cache), `WAI` (wait)

**Aliases and macros (assembler text replacement)**

| Name | Expands to |
|---|---|
| `NOP` | no operation |
| `CMP Xn, Xm` | `SUB XZR, Xn, Xm` |
| `CMPI Xm, IMM` | `SUBI XZR, Xm, IMM` |
| `MOV Xd, Xn` | `ORR Xd, XZR, Xn` |
| `LDA Xd, IMM` | `ORRI Xd, XZR, IMM` |
| `NOT Xd, Xn` | `EORI Xd, Xn, -1` |
| `FOR` / `IF` | Macros that expand to compares and branches. Nesting is supported. Compare register to value or register to register (not value to value). |

### Assembler directives

| Directive | Meaning |
|---|---|
| `Label:` | Marks a location in `.text` or `.data` |
| `.text` | Instruction section |
| `.data` | Data section (placed after instructions in the binary) |
| `.padding` | Aligns sections to cache lines to avoid instruction/data conflicts |

### ISA modes

The design calls for four run modes, selectable from the GUI: (1) pipeline and cache disabled, (2) memory only, (3) pipeline only, (4) cache and pipeline enabled.

## Memory and cache design

### Implemented in code (`src/simulator/memory.cpp`)

| Parameter | Value |
|---|---|
| RAM size | 262,144 bytes / 256 KiB (`MAXMEMORY`) |
| Line size | 8 words (32 bytes) |
| RAM access latency | 100 cycles (RAM is blocked while busy) |
| L1 cache | Split direct-mapped instruction/data caches, 32 lines of 8 words each |
| Address split | tag = `addr >> 10`, index = bits [9:5], word offset = bits [4:2], byte offset = bits [1:0] |
| Cache states | `cache_hit`, `cache_miss`, `cache_fetch`, `cache_wait`, plus `blocked_error` (`0xFF`) |

Accesses are non-blocking requests that return a status immediately. If the unit is busy, the call returns `blocked_error` and the caller must retry on a later cycle. Dirty lines are written back to RAM through the bus.

## PPU (graphics)

The PPU in `src/simulator/include/ppu.h` follows the NES model and renders a 256 x 224 SDL3 window (scaled 4x). Its registers and graphics data live in the simulated memory map.

- 64 sprites in OAM (4 bytes each: Y, tile, attributes, X) with horizontal/vertical flip
- A 32 x 30 nametable and attribute table
- Foreground and background palettes, plus a 4 KiB tile region
- Keyboard input mapped to simulator input operations

Use `programs/loadppu.bin` or `programs/tetris.bin` to exercise the PPU.

## Planned benchmarks

- **Matrix multiplication** (starting at 2x2 and 3x3, scaling up to around 50x50) to fill the cache and exercise misses, with results checked against the known answer
- **Exchange sort** over a large set of numbers, verified by inspecting the final ordering
- **Graphical benchmark**: write an image's bytes into the PPU memory region and confirm it displays correctly
- Metrics: cache hit rate, CPI, final clock count and execution time

## Documentation

`CS535_Final_Report_Draft.pdf` contains the complete ISA specification: instruction encodings, register conventions, assembler directives, cache layout, benchmark plan, GUI design (Qt, with the simulation on a separate thread) and project management notes.

## License

Released under the [MIT License](LICENSE). Copyright (c) 2025 pghezzi.
