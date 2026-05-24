.section .text.trap
.globl trap_vector
trap_vector:
    addi sp, sp, -16
    sw ra, 12(sp)

    mv a3, a2
    mv a2, a1
    mv a1, a0
    mv a0, a7
    call os_syscall_dispatch

    lw ra, 12(sp)
    addi sp, sp, 16
    mret

