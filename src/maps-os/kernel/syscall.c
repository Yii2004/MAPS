#include "maps_os/syscall.h"

#include "maps_os/console.h"
#include "maps_os/npu.h"

i32 os_syscall_dispatch(u32 number, u32 arg0, u32 arg1, u32 arg2) {
    (void)arg2;

    switch (number) {
        case OS_SYSCALL_EXIT:
            return (i32)arg0;

        case OS_SYSCALL_WRITE:
            console_write((const char*)(uintptr_t)arg0, arg1);
            return (i32)arg1;

        case OS_SYSCALL_NPU_SUBMIT:
            return npu_submit((npu_desc_t*)(uintptr_t)arg0);

        default:
            return -1;
    }
}

