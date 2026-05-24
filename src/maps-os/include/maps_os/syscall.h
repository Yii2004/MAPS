#ifndef MAPS_OS_SYSCALL_H
#define MAPS_OS_SYSCALL_H

#include "maps_os/npu.h"
#include "maps_os/types.h"

#define OS_SYSCALL_EXIT 0u
#define OS_SYSCALL_WRITE 1u
#define OS_SYSCALL_NPU_SUBMIT 2u

i32 os_sys_exit(i32 code);
i32 os_sys_write(const char* data, u32 len);
i32 os_sys_npu_submit(npu_desc_t* desc);

#endif // MAPS_OS_SYSCALL_H

