#include "npu/controller.h"

namespace maps_sim {

    Controller::Controller()
        : array_(),
          weight_buffer_(config::BUFFER_SIZES),
          input_buffer_(config::BUFFER_SIZES),
          output_buffer_(config::BUFFER_SIZES),
          host_buffer_(config::BUFFER_SIZES),
          dma_(),
          scheduler_(),
          plan_(),
          memory_(nullptr),
          current_desc_(),
          state_(NpuState::IDLE),
          error_(NpuError::NONE),
          perf_() {
        dma_.bind_buffer(&input_buffer_);
    }

    void Controller::reset() {
        array_.reset();

        weight_buffer_.reset();
        input_buffer_.reset();
        output_buffer_.reset();
        host_buffer_.reset();
        dma_.reset();
        if (memory_ != nullptr) {
            dma_.bind_memory(memory_);
        }
        dma_.bind_buffer(&input_buffer_);
        plan_.clear();

        current_desc_.reset();
        state_ = NpuState::IDLE;
        error_ = NpuError::NONE;
        perf_ = PerfCounters{};
    }

    bool Controller::submit(const Descriptor& desc) {
        if (!desc.validate()) {
            state_ = NpuState::ERROR;
            error_ = NpuError::INVALID_DESC;
            return false;
        }

        current_desc_ = desc;
        state_ = NpuState::IDLE;
        error_ = NpuError::NONE;
        return true;
    }

    void Controller::bind_memory(Memory* memory) {
        memory_ = memory;
        if (memory_ != nullptr) {
            dma_.bind_memory(memory_);
        }
    }

    bool Controller::load_weight_data(const INT32* src, UINT32 len) {
        if (memory_ == nullptr || src == nullptr) {
            state_ = NpuState::ERROR;
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }
        if (!memory_->load(word_addr(current_desc_.matrix_B_base()), src, len)) {
            state_ = NpuState::ERROR;
            error_ = NpuError::OUT_OF_RANGE;
            return false;
        }
        perf_.dma_write_count += len;
        return true;
    }

    bool Controller::load_input_data(const INT32* src, UINT32 len) {
        if (memory_ == nullptr || src == nullptr) {
            state_ = NpuState::ERROR;
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }
        if (!memory_->load(word_addr(current_desc_.matrix_A_base()), src, len)) {
            state_ = NpuState::ERROR;
            error_ = NpuError::OUT_OF_RANGE;
            return false;
        }
        perf_.dma_write_count += len;
        return true;
    }

    bool Controller::read_output_data(INT32* dst, UINT32 len) {
        if (memory_ == nullptr || dst == nullptr) {
            return false;
        }
        host_buffer_.reset();
        DmaEngine dma_readback;
        dma_readback.bind_memory(memory_);
        dma_readback.bind_buffer(&host_buffer_);
        const DmaRequest req{
            DmaDirection::DRAM_TO_BUFFER,
            current_desc_.matrix_C_base(),
            0,
            len,
            1,
            len,
            len
        };
        if (!dma_readback.submit(req) || !dma_readback.run_sync()) {
            return false;
        }
        return host_buffer_.dump(dst, len);
    }

    bool Controller::run() {
        if (!current_desc_.validate()) {
            state_ = NpuState::ERROR;
            error_ = NpuError::INVALID_DESC;
            return false;
        }

        if (!check_matrix_range()) {
            state_ = NpuState::ERROR;
            error_ = NpuError::OUT_OF_RANGE;
            return false;
        }
        if (memory_ == nullptr) {
            state_ = NpuState::ERROR;
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }

        state_ = NpuState::BUSY;
        error_ = NpuError::NONE;

        if (!build_schedule()) {
            state_ = NpuState::ERROR;
            error_ = NpuError::INVALID_DESC;
            return false;
        }

        array_.reset();

        for (const auto& tile : plan_) {
            if (!run_tile(tile)) {
                state_ = NpuState::ERROR;
                if (error_ == NpuError::NONE) {
                    error_ = NpuError::BUFFER_FAIL;
                }
                return false;
            }
        }

        state_ = NpuState::DONE;
        return true;
    }

    NpuState Controller::state() const {
        return state_;
    }

    NpuError Controller::error() const {
        return error_;
    }

    PerfCounters Controller::perf() const {
        return perf_;
    }

    bool Controller::check_matrix_range() const {
        if (memory_ == nullptr) {
            return false;
        }
        const UINT64 m = current_desc_.matrix_m();
        const UINT64 n = current_desc_.matrix_n();
        const UINT64 k = current_desc_.matrix_k();

        const UINT64 a_need = m * k * sizeof(INT32);
        const UINT64 b_need = k * n * sizeof(INT32);
        const UINT64 c_need = m * n * sizeof(INT32);
        const UINT64 mem_size = static_cast<UINT64>(memory_->size()) * sizeof(INT32);
        const UINT64 a_end = static_cast<UINT64>(current_desc_.matrix_A_base()) + a_need;
        const UINT64 b_end = static_cast<UINT64>(current_desc_.matrix_B_base()) + b_need;
        const UINT64 c_end = static_cast<UINT64>(current_desc_.matrix_C_base()) + c_need;

        return a_end <= mem_size && b_end <= mem_size && c_end <= mem_size;
    }

    bool Controller::build_schedule() {
        return scheduler_.build_plan(current_desc_, plan_);
    }

    bool Controller::run_tile(const GemmTile& tile) {
        const UINT32 a_tile_elems = tile.m_len * tile.k_len;
        const UINT32 b_tile_elems = tile.k_len * tile.n_len;
        const UINT32 c_tile_elems = tile.m_len * tile.n_len;
        if (a_tile_elems > input_buffer_.size() ||
            b_tile_elems > weight_buffer_.size() ||
            c_tile_elems > output_buffer_.size()) {
            error_ = NpuError::OUT_OF_RANGE;
            return false;
        }

        // DMA load A tile into input buffer (2D request with stride).
        dma_.bind_buffer(&input_buffer_);
        DmaRequest req_a{
            DmaDirection::DRAM_TO_BUFFER,
            current_desc_.matrix_A_base() + byte_offset_a(tile.m0, tile.k0),
            0,
            tile.k_len,
            tile.m_len,
            current_desc_.matrix_k(),
            tile.k_len
        };
        if (!dma_.submit(req_a) || !dma_.run_sync()) {
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }
        perf_.dma_read_count += static_cast<UINT64>(tile.k_len) * tile.m_len;

        // DMA load B tile into weight buffer (2D request with stride).
        dma_.bind_buffer(&weight_buffer_);
        DmaRequest req_b{
            DmaDirection::DRAM_TO_BUFFER,
            current_desc_.matrix_B_base() + byte_offset_b(tile.k0, tile.n0),
            0,
            tile.n_len,
            tile.k_len,
            current_desc_.matrix_n(),
            tile.n_len
        };
        if (!dma_.submit(req_b) || !dma_.run_sync()) {
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }
        perf_.dma_read_count += static_cast<UINT64>(tile.n_len) * tile.k_len;

        // Initialize/restore C partial sums into output buffer.
        output_buffer_.reset();
        dma_.bind_buffer(&output_buffer_);
        if (tile.k0 != 0) {
            DmaRequest req_c_in{
                DmaDirection::DRAM_TO_BUFFER,
                current_desc_.matrix_C_base() + byte_offset_c(tile.m0, tile.n0),
                0,
                tile.n_len,
                tile.m_len,
                current_desc_.matrix_n(),
                tile.n_len
            };
            if (!dma_.submit(req_c_in) || !dma_.run_sync()) {
                error_ = NpuError::BUFFER_FAIL;
                return false;
            }
            perf_.dma_read_count += static_cast<UINT64>(tile.n_len) * tile.m_len;
        }

        std::vector<INT32> a_tile(a_tile_elems, 0);
        std::vector<INT32> b_tile(b_tile_elems, 0);
        std::vector<INT32> c_in_tile(c_tile_elems, 0);
        std::vector<INT32> c_out_tile(c_tile_elems, 0);
        if (!input_buffer_.dump(a_tile.data(), a_tile_elems) ||
            !weight_buffer_.dump(b_tile.data(), b_tile_elems)) {
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }

        const INT32* c_in_ptr = nullptr;
        if (tile.k0 != 0) {
            if (!output_buffer_.dump(c_in_tile.data(), c_tile_elems)) {
                error_ = NpuError::BUFFER_FAIL;
                return false;
            }
            c_in_ptr = c_in_tile.data();
        }

        UINT32 tile_cycles = 0;
        if (!array_.run_gemm_tile(current_desc_.mode(),
                                  a_tile.data(),
                                  b_tile.data(),
                                  c_in_ptr,
                                  tile.m_len,
                                  tile.n_len,
                                  tile.k_len,
                                  c_out_tile.data(),
                                  &tile_cycles)) {
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }
        perf_.cycle_count += tile_cycles;
        perf_.mac_count += static_cast<UINT64>(tile.m_len) * tile.n_len * tile.k_len;

        output_buffer_.reset();
        if (!output_buffer_.load(c_out_tile.data(), c_tile_elems)) {
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }

        // DMA store output buffer tile back to memory.
        DmaRequest req_c_out{
            DmaDirection::BUFFER_TO_DRAM,
            current_desc_.matrix_C_base() + byte_offset_c(tile.m0, tile.n0),
            0,
            tile.n_len,
            tile.m_len,
            tile.n_len,
            current_desc_.matrix_n()
        };
        if (!dma_.submit(req_c_out) || !dma_.run_sync()) {
            error_ = NpuError::BUFFER_FAIL;
            return false;
        }
        perf_.dma_write_count += static_cast<UINT64>(tile.n_len) * tile.m_len;

        return true;
    }

    UINT32 Controller::byte_offset_a(UINT32 m, UINT32 k) const {
        return (m * current_desc_.matrix_k() + k) * sizeof(INT32);
    }

    UINT32 Controller::byte_offset_b(UINT32 k, UINT32 n) const {
        return (k * current_desc_.matrix_n() + n) * sizeof(INT32);
    }

    UINT32 Controller::byte_offset_c(UINT32 m, UINT32 n) const {
        return (m * current_desc_.matrix_n() + n) * sizeof(INT32);
    }

    UINT32 Controller::word_addr(UINT32 byte_addr) const {
        return byte_addr / sizeof(INT32);
    }
}
