#include "npu/buffer.h"
#include <algorithm>

namespace maps_sim {

    Buffer::Buffer(UINT32 capacity)
        : data_(capacity, 0), read_count_(0), write_count_(0) {}

    void Buffer::reset() {
        std::fill(data_.begin(), data_.end(), 0);
    
        read_count_ = 0;
        write_count_ = 0;
    }

    bool Buffer::read(UINT32 addr, INT32& out) const {
        if (addr >= data_.size()) {
            return false;
        }
        out = data_[addr];
        read_count_++;
        return true;
    }

    bool Buffer::write(UINT32 addr, INT32 data) {
        if (addr >= data_.size()) {
            return false;
        }
        data_[addr] = data;
        write_count_++;
        return true;
    }

    bool Buffer::load(const INT32* src, UINT32 len) {
        if (src == nullptr || len > data_.size()) {
            return false;
        }

        std::copy(src, src + len, data_.begin());

        write_count_ += len; 
        return true;
    }   

    bool Buffer::dump(INT32* dst, UINT32 len) const {
        if (dst == nullptr || len > data_.size()) {
            return false;
        }

        std::copy(data_.begin(), data_.begin() + len, dst);

        read_count_ += len; 
        return true;
    }

    UINT32 Buffer::size() const {
        return static_cast<UINT32>(data_.size());
    }

    UINT32 Buffer::capacity() const {
        return static_cast<UINT32>(data_.size());
    }

    UINT64 Buffer::read_count() const {
        return read_count_;
    }

    UINT64 Buffer::write_count() const {
        return write_count_;
    }
}
