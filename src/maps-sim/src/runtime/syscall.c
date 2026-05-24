#include "runtime/syscall.h"

#if defined(__riscv)
static inline uint32_t maps_ecall1(uint32_t number, uint32_t arg0) {
    register uint32_t a0 asm("a0") = arg0;
    register uint32_t a7 asm("a7") = number;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return a0;
}

static inline uint32_t maps_ecall2(uint32_t number, uint32_t arg0, uint32_t arg1) {
    register uint32_t a0 asm("a0") = arg0;
    register uint32_t a1 asm("a1") = arg1;
    register uint32_t a7 asm("a7") = number;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return a0;
}
#endif

void maps_sys_exit(int code) {
#if defined(__riscv)
    (void)maps_ecall1(MAPS_SYSCALL_EXIT, (uint32_t)code);
    for (;;) {}
#else
    (void)code;
#endif
}

int maps_sys_putchar(char ch) {
#if defined(__riscv)
    return (int)maps_ecall1(MAPS_SYSCALL_PUTCHAR, (uint32_t)(uint8_t)ch);
#else
    (void)ch;
    return -1;
#endif
}

int maps_sys_write(const char* data, uint32_t len) {
#if defined(__riscv)
    return (int)maps_ecall2(MAPS_SYSCALL_WRITE, (uint32_t)(uintptr_t)data, len);
#else
    (void)data;
    (void)len;
    return -1;
#endif
}
