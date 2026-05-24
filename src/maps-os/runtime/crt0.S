.section .text.start
.globl _start
_start:
    la sp, _stack_top

    la t0, trap_vector
    csrw mtvec, t0

    la t0, __bss_start
    la t1, __bss_end
1:
    beq t0, t1, 2f
    sw zero, 0(t0)
    addi t0, t0, 4
    j 1b

2:
    call kmain
    mv a0, a0
    call machine_exit

3:
    j 3b
