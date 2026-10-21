.include loadppu.asm
.padding
.data
.eqv I 40
.eqv J 40
matrixC: .word -5, 3, -10, -5, 4, 4, 0, 3, 6, 1, -4, 7, -1, 8, -10, -2, -7, 7, 5, 9, 0, 9, -4, -9, 10, -5, -3, -1, -2, 1, 1, 3, -8, -2, 6, -2, 8, -2, -10, 7, 3, 0, 10, -8, 2, -1, -6, 7, 10, 1, -1, -1, 0, 7, -10, -8, -5, 8, 2, 4, 8, -10, -7, 7, -10, -4, -5, -9, 8, -1, 3, -2, 4, 0, 5, 1, 7, 8, -2, -4, 8, 4, -6, 7, 0, 5, -7, 0, 3, 7, -1, 8, -4, -9, 5, 7, 0, 5, -7, -7

.text
main:
    MOVZ X9 0x0003 SH 1
    MOVK X9 0xEBE0
    .for X0=matrixC X0!=400+matrixC X0+=4
        LDUR X1, [X0, 0]
        .if X1<0 
            NOT X1 X1
            ADDI X1 X1 1
            MOVZ X3 0x2400 SH 1
            ORR X1 X1 X3
        .endif
        STUR X1 [X9, 0]
        ADDI X9 X9 4
    .endfor
    MOVZ X1 0x0E17 SH 1
    MOVK X1 0x0DFF
    STUR X1 [X9, 0]
    FLU
    PPU
    WAI 10000