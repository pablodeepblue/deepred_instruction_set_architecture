.text
lda 0 node1
lda 1 node2
lda 2 node5
stur 1 0 4
stur 2 0 8
lda 0 node2
lda 1 node3
lda 2 node5
stur 1 0 4
stur 2 0 8
lda 0 node3
lda 1 node4
stur 1 0 4
lda 0 node5
lda 1 node4
stur 1 0 4
lda 0 node4
lda 1 node6
stur 1 0 4
lda 0 node1
lda 9 arrayresult
movz 10 0
cmpi 10 100
beq 96
ldur 8 0 0
stur 8 9 0
addi 9 9 4
cmpi 0 node6
bne 8
b 72
mov 11 0
cmpi 11 0
beq 44
addi 11 11 4
ldur 1 11 0
cmpi 1 0
beq 16
subi 13 13 4
stur 1 13 0
b 8
b 12
addi 11 11 0
b -44
ldur 0 13 0
addi 13 13 4
addi 10 10 1
b -96
flu
hlt
hlt
hlt
hlt
hlt
hlt
hlt
hlt
.data
node1: .word 1 0 0 0
node2: .word 2 0 0 0
node3: .word 3 0 0 0
node4: .word 4 0 0 0
node5: .word 5 0 0 0
node6: .word 6 0 0 0
arrayresult: .word 0
