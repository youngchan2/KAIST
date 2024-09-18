	.data
array1:	.word	1
array2:	.word	2
    .word	4
	.word	8
	.text
main:
	add $1, $0, $0
    add $5, $0, $0
    lui $7, 0x1000
    ori $7, $7, 0x04
    lw $6, 0($7)
    lw $2, 2($7) 