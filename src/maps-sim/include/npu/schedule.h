#ifndef MAPS_SIM_NPU_SCHEDULE_H
#define MAPS_SIM_NPU_SCHEDULE_H

#include <vector>
#include "common/types.h"
#include "npu/descriptor.h"

namespace maps_sim {

    struct GemmTile {
        UINT32 m0;
        UINT32 n0;
        UINT32 k0;

        UINT32 m_len;
        UINT32 n_len;
        UINT32 k_len;
    };

    class Scheduler {
        public:
            bool build_plan(const Descriptor& desc, std::vector<GemmTile>& out_plan) const;
    };
}

#endif // MAPS_SIM_NPU_SCHEDULE_H
