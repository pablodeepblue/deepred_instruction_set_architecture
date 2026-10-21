.text
main:
    ADDI X0, XZR, 5
    ADDI X1, XZR, 1
    ADDI X2, XZR, 64
    ADDI X3, XZR, 68
    STUR X0, [X2, 0]
    STUR X1, [X3, 0]
    LDUR X5, [X3, 0]
    LDUR X6, [X3, 0]
    B demo
demo:
    CMP	    X5, X0
	BEQ     end
    ADD X5, X5, X6
    B demo
end:
    ADDI X5, X5, 1
    STUR X5, [X2, 0]
    HLT