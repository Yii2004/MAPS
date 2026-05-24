#include "maps_os/types.h"
#include "maps_os/user_runtime.h"

__attribute__((section(".maps_os_data")))
volatile u32 maps_os_status = 0;

__attribute__((section(".maps_os_data")))
i32 maps_os_a[4] = {1, 2, 3, 4};

__attribute__((section(".maps_os_data")))
i32 maps_os_b[4] = {5, 6, 7, 8};

__attribute__((section(".maps_os_data")))
i32 maps_os_c[4] = {0, 0, 0, 0};

u32 maps_os_bss_probe;

int user_main(void) {
    const i32 rc = user_npu_gemm(maps_os_a, maps_os_b, maps_os_c, 2u, 2u, 2u);
    const u32 ok = rc == 0 &&
                   maps_os_bss_probe == 0u &&
                   maps_os_c[0] == 19 &&
                   maps_os_c[1] == 22 &&
                   maps_os_c[2] == 43 &&
                   maps_os_c[3] == 50;

    maps_os_status = ok ? 0x0badc0deu : 0xdeadc0deu;
    if (ok) {
        (void)user_write_cstr("user npu ok\n");
    } else {
        (void)user_write_cstr("user npu bad\n");
    }
    return ok ? 0 : 1;
}
