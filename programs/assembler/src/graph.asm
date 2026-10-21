.padding
.data
node1: .word 1 0 0 0
node2: .word 2 0 0 0
node3: .word 3 0 0 0
node4: .word 4 0 0 0
node5: .word 5 0 0 0
node6: .word 6 0 0 0
arrayresult: .word 0

.text
;construct graph here
LDA X0 node1
LDA X1 node2
LDA X2 node5
STUR X1 [X0 4]
STUR X2 [X0 8]

LDA X0 node2
LDA X1 node3
LDA X2 node5
STUR X1 [X0 4]
STUR X2 [X0 8]

LDA X0 node3
LDA X1 node4
STUR X1 [X0 4]

LDA X0 node5
LDA X1 node4
STUR X1 [X0 4]

LDA X0 node4
LDA X1 node6
STUR X1 [X0 4]

LDA X0 node1
LDA X9 arrayresult
.for x10=0 x10!=100 x10+=1 ; to avoid eternal loop
    LDUR  X8 [X0 0]
    STUR X8 [X9 0]
    ADDI X9 X9 4
    .if X0==node6
        B end
    .endif
    .for x11=x0 x11!=0 x11+=0
        ADDI x11 x11 4
        LDUR X1 [x11 0]
        .if X1 != 0
            PUSH X1
        .else
            b break
        .endif
    .endfor
    break: POP X0
.endfor
end: FLU
