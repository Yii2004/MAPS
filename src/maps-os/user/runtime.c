#include "maps_os/user_runtime.h"

#include "maps_os/npu.h"
#include "maps_os/syscall.h"

void user_exit(i32 code) {
    (void)os_sys_exit(code);
    for (;;) {}
}

i32 user_write(const char* data, u32 len) {
    return os_sys_write(data, len);
}

i32 user_write_cstr(const char* text) {
    u32 len = 0;
    if (text == 0) {
        return -1;
    }
    while (text[len] != '\0') {
        ++len;
    }
    return user_write(text, len);
}

i32 user_npu_gemm(const i32* a, const i32* b, i32* c, u32 m, u32 n, u32 k) {
    static npu_desc_t desc;
    npu_desc_init(&desc, a, b, c, m, n, k);
    return os_sys_npu_submit(&desc);
}

