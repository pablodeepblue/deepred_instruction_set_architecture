.text
lda 1 sto
movz 0 0
cmpi 0 264
beq 20
stur 0 1 0
addi 1 1 4
addi 0 0 1
b -20
hlt
.data
sto: .word 0
