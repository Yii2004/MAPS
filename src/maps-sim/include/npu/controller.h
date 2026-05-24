#ifndef MAPS_SIM_NPU_CONTROLLER_H
#define MAPS_SIM_NPU_CONTROLLER_H

#include <vector>
#include "common/memory.h"
#include "npu/array.h"
#include "npu/buffer.h"
#include "npu/descriptor.h"
#include "npu/dma.h"
#include "npu/schedule.h"
#include "npu/status.h"

namespace maps_sim {

    class Controller {
        public:
            Controller();

            void reset();

            bool submit(const Descriptor& desc);
            bool run();
            void bind_memory(Memory* memory);

            bool load_weight_data(const INT32* src, UINT32 len);
            bool load_input_data(const INT32* src, UINT32 len);
            bool read_output_data(INT32* dst, UINT32 len);

            NpuState state() const;
            NpuError error() const;
            PerfCounters perf() const;

        private:
            bool check_matrix_range() const;
            bool build_schedule();
            bool run_tile(const GemmTile& tile);
            UINT32 byte_offset_a(UINT32 m, UINT32 k) const;
            UINT32 byte_offset_b(UINT32 k, UINT32 n) const;
            UINT32 byte_offset_c(UINT32 m, UINT32 n) const;
            UINT32 word_addr(UINT32 byte_addr) const;

            Array array_;
            Buffer weight_buffer_;
            Buffer input_buffer_;
            Buffer output_buffer_;
            Buffer host_buffer_;
            DmaEngine dma_;
            Scheduler scheduler_;
            std::vector<GemmTile> plan_;
            Memory* memory_;

            Descriptor current_desc_;

            NpuState state_;
            NpuError error_;
            PerfCounters perf_;
    };
}

#endif // MAPS_SIM_NPU_CONTROLLER_H
