#include "common/bus.h"

namespace maps_sim {

    Bus::Bus()
        : memory_(nullptr),
          npu_(nullptr),
          console_output_(nullptr) {}

    void Bus::bind_memory(Memory* memory) {
        memory_ = memory;
    }

    void Bus::bind_npu(NpuDevice* npu) {
        npu_ = npu;
    }

    void Bus::bind_console(std::string* output) {
        console_output_ = output;
    }

    bool Bus::read8(UINT32 addr, UINT8& value) const {
        if (!is_dram_addr(addr)) {
            return false;
        }

        INT32 raw = 0;
        if (memory_ == nullptr || !memory_->read(dram_word_addr(addr), raw)) {
            return false;
        }
        value = static_cast<UINT8>((static_cast<UINT32>(raw) >> (dram_byte_offset(addr) * 8)) & 0xffU);
        return true;
    }

    bool Bus::read16(UINT32 addr, UINT16& value) const {
        if (!is_aligned(addr, sizeof(UINT16)) || !is_dram_addr(addr)) {
            return false;
        }

        UINT8 lo = 0;
        UINT8 hi = 0;
        if (!read8(addr, lo) || !read8(addr + 1, hi)) {
            return false;
        }
        value = static_cast<UINT16>(lo | (static_cast<UINT16>(hi) << 8));
        return true;
    }

    bool Bus::read32(UINT32 addr, UINT32& value) const {
        if (!is_aligned(addr, sizeof(UINT32))) {
            return false;
        }

        if (is_npu_addr(addr)) {
            return npu_ != nullptr &&
                   npu_->read32(addr - address_map::NPU_BASE, value);
        }

        if (is_console_addr(addr)) {
            switch (addr - address_map::CONSOLE_BASE) {
                case console_mmio::STATUS:
                    value = 1;
                    return true;
                default:
                    return false;
            }
        }

        if (is_dram_addr(addr)) {
            INT32 raw = 0;
            if (memory_ == nullptr || !memory_->read(dram_word_addr(addr), raw)) {
                return false;
            }
            value = static_cast<UINT32>(raw);
            return true;
        }

        return false;
    }

    bool Bus::write8(UINT32 addr, UINT8 value) {
        if (is_console_addr(addr)) {
            return write_console(addr - address_map::CONSOLE_BASE, value);
        }

        if (!is_dram_addr(addr)) {
            return false;
        }

        INT32 raw = 0;
        const UINT32 word_addr = dram_word_addr(addr);
        if (memory_ == nullptr || !memory_->read(word_addr, raw)) {
            return false;
        }

        const UINT32 shift = dram_byte_offset(addr) * 8;
        UINT32 word = static_cast<UINT32>(raw);
        word = (word & ~(0xffU << shift)) | (static_cast<UINT32>(value) << shift);
        return memory_->write(word_addr, static_cast<INT32>(word));
    }

    bool Bus::write16(UINT32 addr, UINT16 value) {
        if (!is_aligned(addr, sizeof(UINT16)) || !is_dram_addr(addr)) {
            return false;
        }
        return write8(addr, static_cast<UINT8>(value & 0xffU)) &&
               write8(addr + 1, static_cast<UINT8>((value >> 8) & 0xffU));
    }

    bool Bus::write32(UINT32 addr, UINT32 value) {
        if (!is_aligned(addr, sizeof(UINT32))) {
            return false;
        }

        if (is_npu_addr(addr)) {
            return npu_ != nullptr &&
                   npu_->write32(addr - address_map::NPU_BASE, value);
        }

        if (is_console_addr(addr)) {
            return write_console(addr - address_map::CONSOLE_BASE, value);
        }

        if (is_dram_addr(addr)) {
            return memory_ != nullptr &&
                   memory_->write(dram_word_addr(addr), static_cast<INT32>(value));
        }

        return false;
    }

    bool Bus::is_aligned(UINT32 addr, UINT32 size) const {
        return (addr % size) == 0;
    }

    bool Bus::is_dram_addr(UINT32 addr) const {
        if (memory_ == nullptr || addr < address_map::DRAM_BASE) {
            return false;
        }
        const UINT32 word = dram_word_addr(addr);
        return word < memory_->size();
    }

    bool Bus::is_npu_addr(UINT32 addr) const {
        return addr >= address_map::NPU_BASE &&
               addr < address_map::NPU_BASE + address_map::NPU_SIZE;
    }

    bool Bus::is_console_addr(UINT32 addr) const {
        return addr >= address_map::CONSOLE_BASE &&
               addr < address_map::CONSOLE_BASE + address_map::CONSOLE_SIZE;
    }

    bool Bus::write_console(UINT32 offset, UINT32 value) {
        if (console_output_ == nullptr || offset != console_mmio::DATA) {
            return false;
        }
        console_output_->push_back(static_cast<char>(value & 0xffU));
        return true;
    }

    UINT32 Bus::dram_word_addr(UINT32 addr) const {
        return (addr - address_map::DRAM_BASE) / sizeof(UINT32);
    }

    UINT32 Bus::dram_byte_offset(UINT32 addr) const {
        return (addr - address_map::DRAM_BASE) % sizeof(UINT32);
    }
}
