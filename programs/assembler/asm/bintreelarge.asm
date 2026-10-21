.text
lda 0 node0
lda 1 node1
lda 2 node3
stur 1 0 4
stur 2 0 8
lda 0 node1
lda 1 node2
lda 2 node4
stur 1 0 4
stur 2 0 8
lda 0 node2
lda 1 0
lda 2 0
stur 1 0 4
stur 2 0 8
lda 0 node3
lda 1 node8
lda 2 node6
stur 1 0 4
stur 2 0 8
lda 0 node4
lda 1 node9
lda 2 node5
stur 1 0 4
stur 2 0 8
lda 0 node5
lda 1 node7
lda 2 0
stur 1 0 4
stur 2 0 8
lda 0 node6
lda 1 0
lda 2 0
stur 1 0 4
stur 2 0 8
lda 0 node7
lda 1 0
lda 2 0
stur 1 0 4
stur 2 0 8
lda 0 node8
lda 1 0
lda 2 0
stur 1 0 4
stur 2 0 8
lda 0 node9
lda 1 0
lda 2 0
stur 1 0 4
stur 2 0 8
lda 0 node0
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
hlt
hlt
hlt
hlt
hlt
.data
node0: .word 2 0 0
node1: .word 8 0 0
node2: .word 2 0 0
node3: .word 6 0 0
node4: .word 7 0 0
node5: .word 8 0 0
node6: .word 5 0 0
node7: .word 4 0 0
node8: .word 8 0 0
node9: .word 10 0 0
arrayresult: .word 0
