#ifndef MAPS_SIM_NPU_STATUS_H
#define MAPS_SIM_NPU_STATUS_H

#include "common/types.h"

namespace maps_sim {

    enum class NpuState {
        IDLE,
        BUSY,
        DONE,
        ERROR
    };

    enum class NpuError {
        NONE,
        INVALID_DESC,
        OUT_OF_RANGE,
        BUFFER_FAIL
    };

    struct PerfCounters {
        UINT64 cycle_count = 0;
        UINT64 dma_read_count = 0;
        UINT64 dma_write_count = 0;
        UINT64 mac_count = 0;
    };

}


#endif
