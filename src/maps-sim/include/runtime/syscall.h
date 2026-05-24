#ifndef MAPS_RUNTIME_SYSCALL_H
#define MAPS_RUNTIME_SYSCALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAPS_SYSCALL_EXIT 0u
#define MAPS_SYSCALL_PUTCHAR 1u
#define MAPS_SYSCALL_WRITE 2u

void maps_sys_exit(int code);
int maps_sys_putchar(char ch);
int maps_sys_write(const char* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // MAPS_RUNTIME_SYSCALL_H
