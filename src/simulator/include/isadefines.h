#ifndef ISA_DEFINES_H
#define ISA_DEFINES_H

//python ppu_assembler.py -a -i tetris.asm && PPU.exe ./out
#include <stdint.h>

#define byte_ uint8_t
#define halfword uint16_t
#define word uint32_t
#define doubleword uint64_t

#define XSP 13
#define XPC 14
#define XZR 15

#define HLT 0x00
#define ADD 0x20
#define SUB 0x21
#define DIV 0x22
#define MOD 0x23
#define MUL 0x24
#define AND 0x25
#define ORR 0x26
#define EOR 0x27
#define LSL 0x28
#define LSR 0x29
#define BR 0x3A
#define ADDI 0x40
#define SUBI 0x41
#define ANDI 0x42
#define ORRI 0x43
#define EORI 0x44
#define LDUR 0x45
#define LDURB 0x46
#define STUR 0x50
#define STURB 0x51
#define B_ 0x80
#define BEQ 0x81
#define BNE 0x83
#define BLT 0x84
#define BLE 0x85
#define BGT 0x86
#define BGE 0x87
#define BLO 0x88
#define BLS 0x89
#define BHI 0x8A
#define BHS 0x8B
#define MOVK 0xA0
#define MOVZ 0xA1
#define MOVB 0xA2
#define PPU_ 0xC0
#define FLU 0xC1
#define WAI 0xC2
#define INP 0xC3
#define NOP 0xE0

#define HFORMAT 0x00
#define RFORMAT 0x01
#define IFORMAT 0x02
#define DFORMAT 0x02
#define BFORMAT 0x04
#define IMFORMAT 0x05
#define MEFORMAT 0x06
#define NOPFORMAT 0x07

#endif