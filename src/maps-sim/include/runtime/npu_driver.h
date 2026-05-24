#ifndef MAPS_RUNTIME_NPU_DRIVER_H
#define MAPS_RUNTIME_NPU_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAPS_NPU_BASE 0x10000000u

#define MAPS_NPU_DESC_ADDR 0x00u
#define MAPS_NPU_CMD 0x04u
#define MAPS_NPU_STATUS 0x08u
#define MAPS_NPU_ERROR 0x0cu
#define MAPS_NPU_CYCLE_COUNT_LO 0x10u
#define MAPS_NPU_CYCLE_COUNT_HI 0x14u
#define MAPS_NPU_DMA_READ_COUNT_LO 0x18u
#define MAPS_NPU_DMA_READ_COUNT_HI 0x1cu
#define MAPS_NPU_DMA_WRITE_COUNT_LO 0x20u
#define MAPS_NPU_DMA_WRITE_COUNT_HI 0x24u
#define MAPS_NPU_MAC_COUNT_LO 0x28u
#define MAPS_NPU_MAC_COUNT_HI 0x2cu

#define MAPS_NPU_CMD_START 1u
#define MAPS_NPU_CMD_RESET 2u

#define MAPS_NPU_STATE_IDLE 0u
#define MAPS_NPU_STATE_BUSY 1u
#define MAPS_NPU_STATE_DONE 2u
#define MAPS_NPU_STATE_ERROR 3u

#define MAPS_NPU_ERROR_NONE 0u
#define MAPS_NPU_ERROR_INVALID_DESC 1u
#define MAPS_NPU_ERROR_OUT_OF_RANGE 2u
#define MAPS_NPU_ERROR_BUFFER_FAIL 3u

#define MAPS_NPU_DATAFLOW_WS 0u
#define MAPS_NPU_DATAFLOW_OS 1u
#define MAPS_NPU_DATAFLOW_IS 2u

#define MAPS_NPU_DESC_WORDS 10u

typedef struct maps_npu_desc {
    uint32_t matrix_m;
    uint32_t matrix_n;
    uint32_t matrix_k;
    uint32_t matrix_a_base;
    uint32_t matrix_b_base;
    uint32_t matrix_c_base;
    uint32_t dataflow;
    uint32_t tile_m;
    uint32_t tile_n;
    uint32_t tile_k;
} maps_npu_desc_t;

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
                        uint32_t tile_k);

int maps_npu_desc_valid(const maps_npu_desc_t* desc);
void maps_npu_reset(void);
void maps_npu_start(uint32_t desc_addr);
int maps_npu_wait_done(uint32_t max_polls);

int maps_npu_gemm(maps_npu_desc_t* desc,
                  uint32_t desc_addr,
                  uint32_t a_base,
                  uint32_t b_base,
                  uint32_t c_base,
                  uint32_t m,
                  uint32_t n,
                  uint32_t k);

#ifdef __cplusplus
}
#endif

#endif // MAPS_RUNTIME_NPU_DRIVER_H
