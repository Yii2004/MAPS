#include "maps_os/machine_syscall.h"

static inline u32 ecall1(u32 number, u32 arg0) {
    register u32 a0 asm("a0") = arg0;
    register u32 a7 asm("a7") = number;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return a0;
}

static inline u32 ecall2(u32 number, u32 arg0, u32 arg1) {
    register u32 a0 asm("a0") = arg0;
    register u32 a1 asm("a1") = arg1;
    register u32 a7 asm("a7") = number;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return a0;
}

void machine_exit(i32 code) {
    asm volatile("csrw mtvec, zero" ::: "memory");
    (void)ecall1(MAPS_SYSCALL_EXIT, (u32)code);
    for (;;) {}
}

i32 machine_putchar(char ch) {
    return (i32)ecall1(MAPS_SYSCALL_PUTCHAR, (u32)(u8)ch);
}

i32 machine_write(const char* data, u32 len) {
    return (i32)ecall2(MAPS_SYSCALL_WRITE, (u32)(uintptr_t)data, len);
}
