#include "maps_os/syscall.h"

static inline u32 os_ecall1(u32 number, u32 arg0) {
    register u32 a0 asm("a0") = arg0;
    register u32 a7 asm("a7") = number;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return a0;
}

static inline u32 os_ecall2(u32 number, u32 arg0, u32 arg1) {
    register u32 a0 asm("a0") = arg0;
    register u32 a1 asm("a1") = arg1;
    register u32 a7 asm("a7") = number;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return a0;
}

i32 os_sys_exit(i32 code) {
    return (i32)os_ecall1(OS_SYSCALL_EXIT, (u32)code);
}

i32 os_sys_write(const char* data, u32 len) {
    return (i32)os_ecall2(OS_SYSCALL_WRITE, (u32)(uintptr_t)data, len);
}

i32 os_sys_npu_submit(npu_desc_t* desc) {
    return (i32)os_ecall1(OS_SYSCALL_NPU_SUBMIT, (u32)(uintptr_t)desc);
}

