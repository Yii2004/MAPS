#include <stdint.h>

#include "runtime/npu_driver.h"
#include "runtime/syscall.h"

__attribute__((section(".maps_data")))
volatile uint32_t maps_baremetal_status = 0;

__attribute__((section(".maps_data")))
maps_npu_desc_t maps_baremetal_desc;

__attribute__((section(".maps_data")))
int32_t maps_baremetal_a[4] = {1, 2, 3, 4};

__attribute__((section(".maps_data")))
int32_t maps_baremetal_b[4] = {5, 6, 7, 8};

__attribute__((section(".maps_data")))
int32_t maps_baremetal_c[4] = {0, 0, 0, 0};

uint32_t maps_baremetal_bss_value;

int main(void) {
    static const char done_msg[] = "npu done\n";
    static const char fail_msg[] = "npu failed\n";

    const int rc = maps_npu_gemm(&maps_baremetal_desc,
                                 (uint32_t)(uintptr_t)&maps_baremetal_desc,
                                 (uint32_t)(uintptr_t)maps_baremetal_a,
                                 (uint32_t)(uintptr_t)maps_baremetal_b,
                                 (uint32_t)(uintptr_t)maps_baremetal_c,
                                 2u,
                                 2u,
                                 2u);
    maps_baremetal_status = (rc == 0 && maps_baremetal_bss_value == 0u)
                                 ? 0x600d600du
                                 : (0xbad00000u | (uint32_t)(-rc));
    if (rc == 0) {
        (void)maps_sys_write(done_msg, sizeof(done_msg) - 1u);
    } else {
        (void)maps_sys_write(fail_msg, sizeof(fail_msg) - 1u);
    }
    return rc;
}
