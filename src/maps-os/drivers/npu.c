#include "maps_os/npu.h"

#define NPU_BASE 0x10000000u
#define NPU_DESC_ADDR 0x00u
#define NPU_CMD 0x04u
#define NPU_STATUS 0x08u
#define NPU_ERROR 0x0cu

#define NPU_CMD_START 1u
#define NPU_STATE_DONE 2u
#define NPU_STATE_ERROR 3u

#define NPU_REG(offset) (*(volatile u32*)(uintptr_t)(NPU_BASE + (offset)))

void npu_desc_init(npu_desc_t* desc,
                   const i32* a,
                   const i32* b,
                   i32* c,
                   u32 m,
                   u32 n,
                   u32 k) {
    if (desc == 0) {
        return;
    }

    desc->matrix_m = m;
    desc->matrix_n = n;
    desc->matrix_k = k;
    desc->matrix_a_base = (u32)(uintptr_t)a;
    desc->matrix_b_base = (u32)(uintptr_t)b;
    desc->matrix_c_base = (u32)(uintptr_t)c;
    desc->dataflow = NPU_DATAFLOW_OS;
    desc->tile_m = 16u;
    desc->tile_n = 16u;
    desc->tile_k = 16u;
}

i32 npu_submit(npu_desc_t* desc) {
    if (desc == 0) {
        return -1;
    }

    NPU_REG(NPU_DESC_ADDR) = (u32)(uintptr_t)desc;
    NPU_REG(NPU_CMD) = NPU_CMD_START;

    for (u32 i = 0; i < 1000000u; ++i) {
        const u32 status = NPU_REG(NPU_STATUS);
        if (status == NPU_STATE_DONE) {
            return 0;
        }
        if (status == NPU_STATE_ERROR) {
            return -(i32)NPU_REG(NPU_ERROR);
        }
    }
    return -2;
}

i32 npu_gemm(const i32* a, const i32* b, i32* c, u32 m, u32 n, u32 k) {
    static npu_desc_t desc;
    npu_desc_init(&desc, a, b, c, m, n, k);
    return npu_submit(&desc);
}

