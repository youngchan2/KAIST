    .data
data1:  .word   1
data2:  .word   4
    .text
main:
    la $2, data1
    lw $4, 0($2)
    lw $3, 4($2)
loop:
    addi $4, $4, 1
    slt $5, $4, $3
    bne $5, $0, loop
    addi $15, $15, 0
    addi $14, $14, 0
    addi $12, $12, 0
    addi $13, $13, 0
     