#ifndef MAPS_SIM_COMMON_BUS_H
#define MAPS_SIM_COMMON_BUS_H

#include <string>

#include "common/memory.h"
#include "npu/device.h"

namespace maps_sim {

    namespace address_map {
        inline constexpr UINT32 DRAM_BASE = 0x00000000;
        inline constexpr UINT32 NPU_BASE = 0x10000000;
        inline constexpr UINT32 NPU_SIZE = 0x00000100;
        inline constexpr UINT32 CONSOLE_BASE = 0x10000100;
        inline constexpr UINT32 CONSOLE_SIZE = 0x00000100;
    }

    namespace console_mmio {
        inline constexpr UINT32 DATA = 0x00;
        inline constexpr UINT32 STATUS = 0x04;
    }

    class Bus {
        public:
            Bus();

            void bind_memory(Memory* memory);
            void bind_npu(NpuDevice* npu);
            void bind_console(std::string* output);

            bool read8(UINT32 addr, UINT8& value) const;
            bool read16(UINT32 addr, UINT16& value) const;
            bool read32(UINT32 addr, UINT32& value) const;
            bool write8(UINT32 addr, UINT8 value);
            bool write16(UINT32 addr, UINT16 value);
            bool write32(UINT32 addr, UINT32 value);

        private:
            bool is_aligned(UINT32 addr, UINT32 size) const;
            bool is_dram_addr(UINT32 addr) const;
            bool is_npu_addr(UINT32 addr) const;
            bool is_console_addr(UINT32 addr) const;
            bool write_console(UINT32 offset, UINT32 value);
            UINT32 dram_word_addr(UINT32 addr) const;
            UINT32 dram_byte_offset(UINT32 addr) const;

            Memory* memory_;
            NpuDevice* npu_;
            std::string* console_output_;
    };
}

#endif // MAPS_SIM_COMMON_BUS_H
