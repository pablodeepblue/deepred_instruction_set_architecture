.text
addi 0 15 5
addi 1 15 1
addi 2 15 64
addi 3 15 68
stur 0 2 0
stur 1 3 0
ldur 5 3 0
ldur 6 3 0
b 4
cmp 5 0
beq 12
add 5 5 6
b -12
addi 5 5 1
stur 5 2 0
hlt
.data

