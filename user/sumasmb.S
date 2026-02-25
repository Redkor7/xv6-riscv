.data
    buf:        .space 256
    fmt_bound:  .asciz "|%s|\n"
    fmt_sum:    .asciz "%d\n"

    e_empty:    .asciz "Error: Empty input\n"
    e_space:    .asciz "Error: Invalid input format (missing space)\n"
    e_num1:     .asciz "Error: No first number\n"
    e_num2:     .asciz "Error: No second number\n"

.text
.global main

main:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)

    la a0, buf
    li a1, 256
    call gets
    la t0, buf
    lb t1, 0(t0)
    beqz t1, err_empty
    li t2, 10
    beq t1, t2, err_empty

    la t0, buf
remove_nl:
    lb t1, 0(t0)
    beqz t1, print_bounded    
    li t2, 10                 
    beq t1, t2, found_nl      
    addi t0, t0, 1
    j remove_nl
found_nl:
    sb zero, 0(t0)            

print_bounded:
    la a0, fmt_bound
    la a1, buf
    call printf

    la t0, buf
find_space:
    lb t1, 0(t0)
    beqz t1, err_space        
    li t2, 32
    beq t1, t2, found_space   
    addi t0, t0, 1
    j find_space

found_space:
    sb zero, 0(t0)
    addi s0, t0, 1

    la t0, buf
    lb t1, 0(t0)
    li t2, 48
    blt t1, t2, err_num1
    li t2, 57
    bgt t1, t2, err_num1


    lb t1, 0(s0)
    li t2, 48
    blt t1, t2, err_num2
    li t2, 57
    bgt t1, t2, err_num2

    la a0, buf
    call atoi
    mv s1, a0

    mv a0, s0                 
    call atoi
    add a1, s1, a0
    la a0, fmt_sum            
    call printf               

    li a7, 2
    li a0, 0
    ecall

err_empty:
    li a7, 16
    li a0, 2
    la a1, e_empty
    li a2, 19
    ecall

    li a7, 2
    li a0, 1
    ecall

err_space:
    li a7, 16
    li a0, 2
    la a1, e_space
    li a2, 44
    ecall

    li a7, 2
    li a0, 1
    ecall

err_num1:
    li a7, 16
    li a0, 2
    la a1, e_num1
    li a2, 23
    ecall

    li a7, 2
    li a0, 1
    ecall

err_num2:
    li a7, 16
    li a0, 2
    la a1, e_num2
    li a2, 24
    ecall

    li a7, 2
    li a0, 1
    ecall