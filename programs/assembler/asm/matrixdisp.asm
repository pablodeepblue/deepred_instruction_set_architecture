.text
movz 9 3 sh 1
movk 9 60384
movz 0 matrixc
cmpi 0 400+matrixc
beq 48
ldur 1 0 0
cmpi 1 0
bge 20
not 1 1
addi 1 1 1
movz 3 9216 sh 1
orr 1 1 3
stur 1 9 0
addi 9 9 4
addi 0 0 4
b -48
movz 1 3607 sh 1
movk 1 3583
stur 1 9 0
flu
ppu
wai 10000
hlt
hlt
.data
matrixc: .word -5 3 -10 -5 4 4 0 3 6 1 -4 7 -1 8 -10 -2 -7 7 5 9 0 9 -4 -9 10 -5 -3 -1 -2 1 1 3 -8 -2 6 -2 8 -2 -10 7 3 0 10 -8 2 -1 -6 7 10 1 -1 -1 0 7 -10 -8 -5 8 2 4 8 -10 -7 7 -10 -4 -5 -9 8 -1 3 -2 4 0 5 1 7 8 -2 -4 8 4 -6 7 0 5 -7 0 3 7 -1 8 -4 -9 5 7 0 5 -7 -7
