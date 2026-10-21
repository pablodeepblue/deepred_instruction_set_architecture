.data
sto: .word 0

.text
LDA X1 sto
.for X0=0 X0!=264 X0+=1
	STUR X0, [X1 , 0]
	ADDI X1, X1, 4
.endfor