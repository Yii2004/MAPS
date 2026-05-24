#ifndef MAPS_SIM_NPU_DEVICE_H
#define MAPS_SIM_NPU_DEVICE_H

#include "common/memory.h"
#include "npu/controller.h"

namespace maps_sim {

    namespace npu_mmio {
        inline constexpr UINT32 DESC_ADDR = 0x00;
        inline constexpr UINT32 CMD = 0x04;
        inline constexpr UINT32 STATUS = 0x08;
        inline constexpr UINT32 ERROR = 0x0c;
        inline constexpr UINT32 CYCLE_COUNT_LO = 0x10;
        inline constexpr UINT32 CYCLE_COUNT_HI = 0x14;
        inline constexpr UINT32 DMA_READ_COUNT_LO = 0x18;
        inline constexpr UINT32 DMA_READ_COUNT_HI = 0x1c;
        inline constexpr UINT32 DMA_WRITE_COUNT_LO = 0x20;
        inline constexpr UINT32 DMA_WRITE_COUNT_HI = 0x24;
        inline constexpr UINT32 MAC_COUNT_LO = 0x28;
        inline constexpr UINT32 MAC_COUNT_HI = 0x2c;

        inline constexpr UINT32 CMD_START = 1;
        inline constexpr UINT32 CMD_RESET = 2;
    }

    class NpuDevice {
        public:
            NpuDevice();

            void bind_memory(Memory* memory);
            void reset();

            bool write32(UINT32 offset, UINT32 value);
            bool read32(UINT32 offset, UINT32& value) const;

            Controller& controller();
            const Controller& controller() const;

        private:
            bool start();
            UINT32 perf_lo(UINT64 value) const;
            UINT32 perf_hi(UINT64 value) const;

            Controller controller_;
            Memory* memory_;
            UINT32 desc_addr_;
    };
}

#endif // MAPS_SIM_NPU_DEVICE_H
