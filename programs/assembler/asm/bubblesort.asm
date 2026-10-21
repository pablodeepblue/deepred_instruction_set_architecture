.text
movz 0 array0+32-4
cmpi 0 array0
beq 68
movz 1 array0
cmp 1 0
beq 48
ldur 2 1 0
ldur 3 1 4
cmp 2 3
ble 16
stur 2 1 4
stur 3 1 0
b 12
stur 2 1 0
stur 3 1 4
addi 1 1 4
b -48
subi 0 0 4
b -68
hlt
hlt
hlt
hlt
hlt
.data
array0: .word 0 2 -4 9 6 20 0 30
