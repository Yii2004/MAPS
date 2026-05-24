#ifndef MAPS_OS_MACHINE_SYSCALL_H
#define MAPS_OS_MACHINE_SYSCALL_H

#include "maps_os/types.h"

#define MAPS_SYSCALL_EXIT 0u
#define MAPS_SYSCALL_PUTCHAR 1u
#define MAPS_SYSCALL_WRITE 2u

void machine_exit(i32 code);
i32 machine_putchar(char ch);
i32 machine_write(const char* data, u32 len);

#endif // MAPS_OS_MACHINE_SYSCALL_H

