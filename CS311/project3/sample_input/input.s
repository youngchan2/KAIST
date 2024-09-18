	.data
data1:	.word	3
data2:  .word   5
data3:  .word   7
data4:  .word   9
    .text	
main:
	la $4, data1
    lw $5, 0($4)
    lw $6, 4($4)
    add $9, $5, $6
    lw $7, 8($4)
    lw $8, 12($4)
    add $10, $7, $8
    add $11, 49, $10
    sub $12, $10, $9
    