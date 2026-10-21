.text
main:
    MOVZ X3, -1
    ADDI XZR, X3, -1
    CMPI XZR, 0
    BEQ lb1
    HLT
lb1:
    ADDI X0 X0 1
    CMPI XZR, 1
    BNE lb2
    HLT
lb2:
    ADDI X0 X0 1
    CMPI XZR, 1
    BLT lb3
    HLT
lb3:
    ADDI X0 X0 1
    CMPI XZR, 1
    BLE lb4
    HLT
lb4:
    ADDI X0 X0 1
    CMPI XZR, -1
    BGT lb5
    HLT
lb5:
    ADDI X0 X0 1
    CMPI XZR, 0
    BGE lb6
    HLT
lb6:
    ADDI X0 X0 1
    MOVZ X1, 1
    MOVZ X2, 2
    CMP XZR, X1
    BLO lb7
    HLT
lb7:
    ADDI X0 X0 1
    CMPI XZR, 0
    BLS lb8
    HLT
lb8:
    ADDI X0 X0 1
    CMP X1, XZR
    BHI lb9
    HLT
lb9:
    ADDI X0 X0 1
    CMPI XZR, 0
    BHS lb10
    HLT
lb10:
    ADDI X0 X0 1