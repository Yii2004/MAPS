#ifndef MAPS_OS_USER_RUNTIME_H
#define MAPS_OS_USER_RUNTIME_H

#include "maps_os/types.h"

void user_exit(i32 code);
i32 user_write(const char* data, u32 len);
i32 user_write_cstr(const char* text);
i32 user_npu_gemm(const i32* a, const i32* b, i32* c, u32 m, u32 n, u32 k);

#endif // MAPS_OS_USER_RUNTIME_H

