.text
lda 0 root
lda 1 node1
lda 2 node2
stur 1 0 4
stur 2 0 8
lda 0 node1
lda 1 node3
lda 2 node4
stur 1 0 4
stur 2 0 8
lda 0 node2
lda 1 node5
lda 2 node6
stur 1 0 4
stur 2 0 8
lda 0 root
addi 8 8 1
lda 9 arrayresult
subi 13 13 4
stur 0 13 0
movz 10 0
cmpi 10 100
beq 96
cmpi 8 0
bne 8
b 84
ldur 0 13 0
addi 13 13 4
subi 8 8 1
ldur 1 0 0
stur 1 9 0
addi 9 9 4
ldur 1 0 8
cmpi 1 0
beq 16
addi 8 8 1
subi 13 13 4
stur 1 13 0
ldur 1 0 4
cmpi 1 0
beq 16
addi 8 8 1
subi 13 13 4
stur 1 13 0
addi 10 10 1
b -96
flu
hlt
.data
root: .word 0 0 0
node1: .word 1 0 0
node3: .word 3 0 0
node4: .word 4 0 0
node2: .word 2 0 0
node5: .word 5 0 0
node6: .word 6 0 0
arrayresult: .word 0
