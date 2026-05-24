#ifndef MAPS_OS_NPU_H
#define MAPS_OS_NPU_H

#include "maps_os/types.h"

#define NPU_DATAFLOW_WS 0u
#define NPU_DATAFLOW_OS 1u
#define NPU_DATAFLOW_IS 2u

typedef struct npu_desc {
    u32 matrix_m;
    u32 matrix_n;
    u32 matrix_k;
    u32 matrix_a_base;
    u32 matrix_b_base;
    u32 matrix_c_base;
    u32 dataflow;
    u32 tile_m;
    u32 tile_n;
    u32 tile_k;
} npu_desc_t;

void npu_desc_init(npu_desc_t* desc,
                   const i32* a,
                   const i32* b,
                   i32* c,
                   u32 m,
                   u32 n,
                   u32 k);

i32 npu_submit(npu_desc_t* desc);
i32 npu_gemm(const i32* a, const i32* b, i32* c, u32 m, u32 n, u32 k);

#endif // MAPS_OS_NPU_H

