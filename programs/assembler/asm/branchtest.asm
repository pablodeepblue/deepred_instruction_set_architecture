.text
movz 3 -1
addi 15 3 -1
cmpi 15 0
beq 8
hlt
addi 0 0 1
cmpi 15 1
bne 8
hlt
addi 0 0 1
cmpi 15 1
blt 8
hlt
addi 0 0 1
cmpi 15 1
ble 8
hlt
addi 0 0 1
cmpi 15 -1
bgt 8
hlt
addi 0 0 1
cmpi 15 0
bge 8
hlt
addi 0 0 1
movz 1 1
movz 2 2
cmp 15 1
blo 8
hlt
addi 0 0 1
cmpi 15 0
bls 8
hlt
addi 0 0 1
cmp 1 15
bhi 8
hlt
addi 0 0 1
cmpi 15 0
bhs 8
hlt
addi 0 0 1
hlt
.data

