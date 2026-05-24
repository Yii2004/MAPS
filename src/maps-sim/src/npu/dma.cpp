#include "npu/dma.h"

namespace maps_sim {
    namespace {
        constexpr UINT32 WORD_BYTES = sizeof(INT32);

        bool is_word_aligned(UINT32 addr) {
            return (addr % WORD_BYTES) == 0;
        }

        UINT32 word_addr(UINT32 byte_addr) {
            return byte_addr / WORD_BYTES;
        }
    }

    DmaEngine::DmaEngine()
        : buffer_(nullptr),
          memory_(nullptr),
          dram_size_(0),
          current_req_{DmaDirection::DRAM_TO_BUFFER, 0, 0, 0, 0, 0, 0},
          has_request_(false),
          state_(DmaState::IDLE),
          error_(DmaError::NONE),
          stats_{} {}

    void DmaEngine::bind_memory(Memory* mem) {
        if (mem == nullptr) {
            state_ = DmaState::ERROR;
            error_ = DmaError::NULL_PTR;
            return;
        }
        memory_ = mem;
        dram_size_ = memory_->size();
        if (state_ != DmaState::BUSY) {
            state_ = DmaState::IDLE;
            error_ = DmaError::NONE;
        }
    }

    void DmaEngine::bind_buffer(Buffer* buf) {
        if (buf == nullptr) {
            state_ = DmaState::ERROR;
            error_ = DmaError::NULL_PTR;
            return;
        }
        buffer_ = buf;
        if (state_ != DmaState::BUSY) {
            state_ = DmaState::IDLE;
            error_ = DmaError::NONE;
        }
    }

    bool DmaEngine::submit(const DmaRequest& req) {
        if (state_ == DmaState::BUSY) {
            state_ = DmaState::ERROR;
            error_ = DmaError::NOT_READY;
            return false;
        }

        if (buffer_ == nullptr || memory_ == nullptr) {
            state_ = DmaState::ERROR;
            error_ = DmaError::NULL_PTR;
            return false;
        }

        if (req.length == 0 || req.rows == 0 || req.src_stride == 0 || req.dst_stride == 0 ||
            !is_word_aligned(req.dram_base)) {
            state_ = DmaState::ERROR;
            error_ = DmaError::INVALID_PARAM;
            return false;
        }

        const UINT32 dram_base_word = word_addr(req.dram_base);
        if (req.direction == DmaDirection::DRAM_TO_BUFFER) {
            const UINT64 src_last = static_cast<UINT64>(dram_base_word) +
                                    static_cast<UINT64>(req.rows - 1) * req.src_stride +
                                    static_cast<UINT64>(req.length - 1);
            const UINT64 dst_last = static_cast<UINT64>(req.buffer_base) +
                                    static_cast<UINT64>(req.rows - 1) * req.dst_stride +
                                    static_cast<UINT64>(req.length - 1);
            if (src_last >= dram_size_ || dst_last >= buffer_->size()) {
                state_ = DmaState::ERROR;
                error_ = DmaError::OUT_OF_RANGE;
                return false;
            }
        } else if (req.direction == DmaDirection::BUFFER_TO_DRAM) {
            const UINT64 src_last = static_cast<UINT64>(req.buffer_base) +
                                    static_cast<UINT64>(req.rows - 1) * req.src_stride +
                                    static_cast<UINT64>(req.length - 1);
            const UINT64 dst_last = static_cast<UINT64>(dram_base_word) +
                                    static_cast<UINT64>(req.rows - 1) * req.dst_stride +
                                    static_cast<UINT64>(req.length - 1);
            if (src_last >= buffer_->size() || dst_last >= dram_size_) {
                state_ = DmaState::ERROR;
                error_ = DmaError::OUT_OF_RANGE;
                return false;
            }
        } else {
            state_ = DmaState::ERROR;
            error_ = DmaError::INVALID_PARAM;
            return false;
        }

        current_req_ = req;
        has_request_ = true;
        state_ = DmaState::BUSY;
        error_ = DmaError::NONE;
        return true;
    }

    bool DmaEngine::run_sync() {
        if (!has_request_ || state_ != DmaState::BUSY) {
            state_ = DmaState::ERROR;
            error_ = DmaError::NOT_READY;
            return false;
        }

        for (UINT32 r = 0; r < current_req_.rows; ++r) {
            for (UINT32 c = 0; c < current_req_.length; ++c) {
                INT32 value = 0;
                const UINT32 dram_base_word = word_addr(current_req_.dram_base);

                switch (current_req_.direction) {
                    case DmaDirection::DRAM_TO_BUFFER:
                        if (!memory_->read(dram_base_word + r * current_req_.src_stride + c, value)) {
                            state_ = DmaState::ERROR;
                            error_ = DmaError::OUT_OF_RANGE;
                            return false;
                        }
                        if (!buffer_->write(current_req_.buffer_base + r * current_req_.dst_stride + c, value)) {
                            state_ = DmaState::ERROR;
                            error_ = DmaError::OUT_OF_RANGE;
                            return false;
                        }
                        break;

                    case DmaDirection::BUFFER_TO_DRAM:
                        if (!buffer_->read(current_req_.buffer_base + r * current_req_.src_stride + c, value)) {
                            state_ = DmaState::ERROR;
                            error_ = DmaError::OUT_OF_RANGE;
                            return false;
                        }
                        if (!memory_->write(dram_base_word + r * current_req_.dst_stride + c, value)) {
                            state_ = DmaState::ERROR;
                            error_ = DmaError::OUT_OF_RANGE;
                            return false;
                        }
                        break;

                    default:
                        state_ = DmaState::ERROR;
                        error_ = DmaError::INVALID_PARAM;
                        return false;
                }
            }
        }

        const UINT64 elems = static_cast<UINT64>(current_req_.length) * current_req_.rows;
        stats_.bytes_moved += elems * sizeof(INT32);
        stats_.transfer_count += 1;
        stats_.cycle_count += elems;

        has_request_ = false;
        state_ = DmaState::DONE;
        error_ = DmaError::NONE;
        return true;
    }

    DmaState DmaEngine::state() const {
        return state_;
    }

    DmaError DmaEngine::error() const {
        return error_;
    }

    DmaStats DmaEngine::stats() const {
        return stats_;
    }

    bool DmaEngine::busy() const {
        return state_ == DmaState::BUSY;
    }

    bool DmaEngine::done() const {
        return state_ == DmaState::DONE;
    }

    void DmaEngine::reset() {
        has_request_ = false;
        current_req_ = {DmaDirection::DRAM_TO_BUFFER, 0, 0, 0, 0, 0, 0};
        state_ = DmaState::IDLE;
        error_ = DmaError::NONE;
        stats_ = {};
    }

}
