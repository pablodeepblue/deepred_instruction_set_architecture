.padding
.data
root:  .word 0 0 0
node1: .word 1 0 0
node3: .word 3 0 0
node4: .word 4 0 0
node2: .word 2 0 0
node5: .word 5 0 0
node6: .word 6 0 0

arrayresult: .word 0

.text
main:
; assemler does not allow for datalable datalables, 
; so tree will be made here
LDA X0 root
LDA X1 node1
LDA X2 node2
STUR  X1 [X0, 4]
STUR  X2 [X0, 8]
LDA X0 node1
LDA X1 node3
LDA X2 node4
STUR  X1 [X0, 4]
STUR  X2 [X0, 8]
LDA X0 node2
LDA X1 node5
LDA X2 node6
STUR  X1 [X0, 4]
STUR  X2 [X0, 8]
; this loop will control the rest
LDA X0 root
ADDI X8 X8 1
LDA X9 arrayresult
PUSH X0
.for x10=0 x10!=100 x10+=1 ; to avoid eternal loop
    .if X8==0
        B end
    .endif
    POP X0
    SUBI X8 X8 1
    LDUR X1 [X0 0]
    STUR X1 [X9 0]
    ADDI X9 X9 4
    LDUR X1 [X0 8]
    .if X1 != 0
        ADDI X8 X8 1
        PUSH X1
    .endif
    LDUR X1 [X0 4]
    .if X1 != 0
        ADDI X8 X8 1
        PUSH X1
    .endif
.endfor
end: FLU
