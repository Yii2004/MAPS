.section .text.start
.globl _start
_start:
    la sp, _stack_top

    la t0, __bss_start
    la t1, __bss_end
1:
    beq t0, t1, 2f
    sw zero, 0(t0)
    addi t0, t0, 4
    j 1b

2:
    la t0, maps_runtime_init
    beqz t0, 3f
    jalr ra, t0, 0

3:
    call main
    mv s0, a0

    la t0, maps_runtime_fini
    beqz t0, 4f
    jalr ra, t0, 0

4:
    mv a0, s0
    call maps_sys_exit

5:
    j 5b

.weak maps_runtime_init
.weak maps_runtime_fini
