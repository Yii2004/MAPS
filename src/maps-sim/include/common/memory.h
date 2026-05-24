#ifndef MAPS_SIM_COMMON_MEMORY_H
#define MAPS_SIM_COMMON_MEMORY_H

#include <vector>
#include "common/types.h"
#include "common/config.h"

namespace maps_sim {

    class Memory {
        public:
            explicit Memory(UINT32 capacity = config::DRAM_SIZES);

            bool read(UINT32 addr, INT32& out) const;
            bool write(UINT32 addr, INT32 value);

            bool load(UINT32 base, const INT32* src, UINT32 len);
            bool dump(UINT32 base, INT32* dst, UINT32 len) const;

            UINT32 size() const;

        private:
            std::vector<INT32> data_;
    };
}

#endif // MAPS_SIM_COMMON_MEMORY_H
