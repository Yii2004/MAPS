#include "runtime/npu_driver.h"

#define MAPS_NPU_REG(offset) (*(volatile uint32_t*)(uintptr_t)(MAPS_NPU_BASE + (offset)))

void maps_npu_desc_init(maps_npu_desc_t* desc,
                        uint32_t a_base,
                        uint32_t b_base,
                        uint32_t c_base,
                        uint32_t m,
                        uint32_t n,
                        uint32_t k,
                        uint32_t dataflow,
                        uint32_t tile_m,
                        uint32_t tile_n,
                        uint32_t tile_k) {
    if (desc == 0) {
        return;
    }

    desc->matrix_m = m;
    desc->matrix_n = n;
    desc->matrix_k = k;
    desc->matrix_a_base = a_base;
    desc->matrix_b_base = b_base;
    desc->matrix_c_base = c_base;
    desc->dataflow = dataflow;
    desc->tile_m = tile_m;
    desc->tile_n = tile_n;
    desc->tile_k = tile_k;
}

int maps_npu_desc_valid(const maps_npu_desc_t* desc) {
    if (desc == 0) {
        return 0;
    }
    if (desc->matrix_m == 0 || desc->matrix_n == 0 || desc->matrix_k == 0) {
        return 0;
    }
    if (desc->matrix_a_base == 0 || desc->matrix_b_base == 0 || desc->matrix_c_base == 0) {
        return 0;
    }
    if ((desc->matrix_a_base & 3u) != 0 || (desc->matrix_b_base & 3u) != 0 ||
        (desc->matrix_c_base & 3u) != 0) {
        return 0;
    }
    if (desc->tile_m == 0 || desc->tile_n == 0 || desc->tile_k == 0) {
        return 0;
    }
    return desc->dataflow <= MAPS_NPU_DATAFLOW_IS;
}

void maps_npu_reset(void) {
    MAPS_NPU_REG(MAPS_NPU_CMD) = MAPS_NPU_CMD_RESET;
}

void maps_npu_start(uint32_t desc_addr) {
    MAPS_NPU_REG(MAPS_NPU_DESC_ADDR) = desc_addr;
    MAPS_NPU_REG(MAPS_NPU_CMD) = MAPS_NPU_CMD_START;
}

int maps_npu_wait_done(uint32_t max_polls) {
    for (uint32_t i = 0; i < max_polls; ++i) {
        const uint32_t status = MAPS_NPU_REG(MAPS_NPU_STATUS);
        if (status == MAPS_NPU_STATE_DONE) {
            return 0;
        }
        if (status == MAPS_NPU_STATE_ERROR) {
            return -2;
        }
    }
    return -1;
}

int maps_npu_gemm(maps_npu_desc_t* desc,
                  uint32_t desc_addr,
                  uint32_t a_base,
                  uint32_t b_base,
                  uint32_t c_base,
                  uint32_t m,
                  uint32_t n,
                  uint32_t k) {
    maps_npu_desc_init(desc,
                       a_base,
                       b_base,
                       c_base,
                       m,
                       n,
                       k,
                       MAPS_NPU_DATAFLOW_OS,
                       16u,
                       16u,
                       16u);
    if (!maps_npu_desc_valid(desc)) {
        return -3;
    }

    maps_npu_start(desc_addr);
    return maps_npu_wait_done(1000000u);
}
