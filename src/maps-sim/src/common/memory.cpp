#include "common/memory.h"

namespace maps_sim {

    Memory::Memory(UINT32 capacity) : data_(capacity, 0) {}

    bool Memory::read(UINT32 addr, INT32& out) const {
        if (addr >= data_.size()) {
            return false;
        }
        out = data_[addr];
        return true;
    }

    bool Memory::write(UINT32 addr, INT32 value) {
        if (addr >= data_.size()) {
            return false;
        }
        data_[addr] = value;
        return true;
    }

    bool Memory::load(UINT32 base, const INT32* src, UINT32 len) {
        if (src == nullptr) {
            return false;
        }
        if (base > data_.size() || len > (data_.size() - base)) {
            return false;
        }
        for (UINT32 i = 0; i < len; ++i) {
            data_[base + i] = src[i];
        }
        return true;
    }

    bool Memory::dump(UINT32 base, INT32* dst, UINT32 len) const {
        if (dst == nullptr) {
            return false;
        }
        if (base > data_.size() || len > (data_.size() - base)) {
            return false;
        }
        for (UINT32 i = 0; i < len; ++i) {
            dst[i] = data_[base + i];
        }
        return true;
    }

    UINT32 Memory::size() const {
        return static_cast<UINT32>(data_.size());
    }
}

