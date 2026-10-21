.text
lda 0 array
addi 1 15 40
mov 3 15
cmp 15 1
beq 24
ldur 2 0 0
add 3 3 2
addi 0 0 4
subi 1 1 4
b -24
hlt
.data
array: .word 1 2 3 4 5 6 7 8 9 10
