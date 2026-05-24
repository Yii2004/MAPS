#ifndef MAPS_SIM_NPU_BUFFER_H
#define MAPS_SIM_NPU_BUFFER_H

#include <vector>
#include "common/types.h"
#include "common/config.h"

namespace maps_sim {

    class Buffer {
        public:
            explicit Buffer(UINT32 capacity = config::BUFFER_SIZES);

            void reset();

            bool read(UINT32 addr, INT32& out) const;
            bool write(UINT32 addr, INT32 data);
            bool load(const INT32* src, UINT32 len);
            bool dump(INT32* dst, UINT32 len) const;

            UINT32 size() const;
            UINT32 capacity() const;

            UINT64 read_count() const;
            UINT64 write_count() const;

        private:
            std::vector<INT32> data_;
            mutable UINT64 read_count_;
            UINT64 write_count_;

    };
}

#endif // MAPS_SIM_NPU_BUFFER_H
