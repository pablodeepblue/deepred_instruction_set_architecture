.include loadppu.asm
.eqv SECTION    0x0003
.eqv OAM        0xEAE0
.eqv NAMETABLE  0xEBE0
.eqv ATRTABLE   0xEFA0
.eqv PALLET     0xEFE0
.eqv TILE       0xF000
.eqv tile1 0x7B
.eqv tile2 0x7C
.eqv tile3 0x7D
.eqv tileHidden 0xFF
.data
tetremino: 0, tile1,-1, 0, tile1, 0, 0, tile1, 1,-1, tile1, 0,
        -1, tile1, 0, 0, tile1, 0, 0, tile1, 1, 1, tile1, 0,
         0, tile1,-1, 0, tile1, 0, 0, tile1, 1, 1, tile1, 0,
        -1, tile1, 0, 0, tile1,-1, 0, tile1, 0, 1, tile1, 0,
        -1, tile3, 0, 0, tile3, 0, 1, tile3,-1, 1, tile3, 0,
        -1, tile3,-1, 0, tile3,-1, 0, tile3, 0, 0, tile3, 1,
        -1, tile3, 0,-1, tile3, 1, 0, tile3, 0, 1, tile3, 0,
         0, tile3,-1, 0, tile3, 0, 0, tile3, 1, 1, tile3, 1,
         0, tile2,-1, 0, tile2, 0, 1, tile2, 0, 1, tile2, 1,
        -1, tile2, 1, 0, tile2, 0, 0, tile2, 1, 1, tile2, 0,
         0, tile1,-1, 0, tile1, 0, 1, tile1,-1, 1, tile1, 0,
         0, tile3, 0, 0, tile3, 1, 1, tile3,-1, 1, tile3, 0,
        -1, tile3, 0, 0, tile3, 0, 0, tile3, 1, 1, tile3, 1,
        -1, tile2, 0, 0, tile2, 0, 1, tile2, 0, 1, tile2, 1,
         0, tile2,-1, 0, tile2, 0, 0, tile2, 1, 1, tile2,-1,
        -1, tile2,-1,-1, tile2, 0, 0, tile2, 0, 1, tile2, 0,
        -1, tile2, 1, 0, tile2,-1, 0, tile2, 0, 0, tile2, 1,
        -2, tile1, 0,-1, tile1, 0, 0, tile1, 0, 1, tile1, 0,
         0, tile1,-2, 0, tile1,-1, 0, tile1, 0, 0, tile1, 1,
         0, tileHidden, 0, 0, tileHidden, 0, 0, tileHidden, 0, 0, tileHidden, 0
.text
main:
    MOVZ X0 SECTION SH 1
    MOVK X0 OAM
    MOVZ X1 0x307B SH 1
    MOVK X1 0x0060
    MOVZ X2 0x307B SH 1
    MOVK X2 0x0068
    MOVZ X3 0x387B SH 1
    MOVK X3 0x0068
    MOVZ X4 0x387B SH 1
    MOVK X4 0x0060
    MOVZ X5 0x0800 SH 1
    STUR X1 [X0 0]
    STUR X2 [X0 4]
    STUR X3 [X0 8]
    STUR X4 [X0 12]
    FLU
    PPU
    .for X6 = 0 X6 != 19 X6 += 1
        .for X8 = 0 X8!=100 X8+=1
            INP X7
            .if X7==6 /*left*/
                SUBI X1 X1 8
                SUBI X2 X2 8
                SUBI X3 X3 8
                SUBI X4 X4 8
            .endif
            .if X7==7 /*right*/
                ADDI X1 X1 8
                ADDI X2 X2 8
                ADDI X3 X3 8
                ADDI X4 X4 8
            .endif
            MOV X7 XZR
            FLU
            PPU
        .endfor
        ADD X1 X1 X5
        ADD X2 X2 X5
        ADD X3 X3 X5
        ADD X4 X4 X5
        STUR X1 [X0 0]
        STUR X2 [X0 4]
        STUR X3 [X0 8]
        STUR X4 [X0 12]
    .endfor
    

