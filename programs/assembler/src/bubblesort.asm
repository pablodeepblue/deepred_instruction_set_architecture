.padding
.data
.eqv N 32
array0: .word 0, 2, -4, 9, 6, 20, 0, 30

.text
main:
.for x0=array0+N-4 x0!=array0 x0-=4
	.for x1=array0 x1!=x0 x1+=4
		LDUR X2 [x1, 0]
		LDUR X3 [x1, 4]
		.if X2 > X3
			STUR X2 [x1, 4]
			STUR X3 [x1, 0]
		.else
			STUR X2 [x1, 0]
			STUR X3 [x1, 4]
		.endif
	.endfor
.endfor
		