#include "npu/descriptor.h"

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

    Descriptor::Descriptor() {
        reset();
    }

    void Descriptor::reset() {
        matrix_m_ = 0;
        matrix_n_ = 0;
        matrix_k_ = 0;

        mode_ = dataflow::OS;

        matrix_A_base_ = 0;
        matrix_B_base_ = 0;
        matrix_C_base_ = 0;

        tile_m_ = config::TILE_SIZES;
        tile_n_ = config::TILE_SIZES;
        tile_k_ = config::TILE_SIZES;
    }

    void Descriptor::set_matrix_mnk(UINT32 matrix_m, UINT32 matrix_n, UINT32 matrix_k) {
        matrix_m_ = matrix_m;
        matrix_n_ = matrix_n;
        matrix_k_ = matrix_k;
    }

    void Descriptor::set_matrix_base(UINT32 matrix_A_base, UINT32 matrix_B_base, UINT32 matrix_C_base) {
        matrix_A_base_ = matrix_A_base;
        matrix_B_base_ = matrix_B_base;
        matrix_C_base_ = matrix_C_base;
    }

    void Descriptor::set_dataflow(dataflow mode) {
        mode_ = mode;
    }

    void Descriptor::set_tile(UINT32 tile_m, UINT32 tile_n, UINT32 tile_k) {
        tile_m_ = tile_m;
        tile_n_ = tile_n;
        tile_k_ = tile_k;
    }

    UINT32 Descriptor::matrix_m() const {
        return matrix_m_;
    }

    UINT32 Descriptor::matrix_n() const {
        return matrix_n_;
    }

    UINT32 Descriptor::matrix_k() const {
        return matrix_k_;
    }

    UINT32 Descriptor::matrix_A_base() const {
        return matrix_A_base_;
    }

    UINT32 Descriptor::matrix_B_base() const {
        return matrix_B_base_;
    }

    UINT32 Descriptor::matrix_C_base() const {
        return matrix_C_base_;
    }

    dataflow Descriptor::mode() const {
        return mode_;
    }

    UINT32 Descriptor::tile_m() const {
        return tile_m_;
    }

    UINT32 Descriptor::tile_n() const {
        return tile_n_;
    }

    UINT32 Descriptor::tile_k() const {
        return tile_k_;
    }

    bool Descriptor::validate() const {
        if (matrix_m_ == 0 || matrix_n_ == 0 || matrix_k_ == 0) {
            return false;
        }

        if (tile_m_ == 0 || tile_n_ == 0 || tile_k_ == 0) {
            return false;
        }

        switch (mode_) {
            case dataflow::OS:
                if (tile_m_ > config::ARRAY_ROWS || tile_n_ > config::ARRAY_COLS) {
                    return false;
                }
                break;
            case dataflow::WS:
                if (tile_k_ > config::ARRAY_ROWS || tile_n_ > config::ARRAY_COLS) {
                    return false;
                }
                break;
            case dataflow::IS:
                if (tile_m_ > config::ARRAY_ROWS || tile_k_ > config::ARRAY_COLS) {
                    return false;
                }
                break;
            default:
                return false;
        }

        if (matrix_A_base_ == 0 || matrix_B_base_ == 0 || matrix_C_base_ == 0) {
            return false;
        }

        if (!is_word_aligned(matrix_A_base_) ||
            !is_word_aligned(matrix_B_base_) ||
            !is_word_aligned(matrix_C_base_)) {
            return false;
        }

        return true;
    }

    bool Descriptor::load_from_memory(const Memory& memory, UINT32 base) {
        if (!is_word_aligned(base)) {
            return false;
        }

        INT32 raw[npu_descriptor::WORD_COUNT] = {};
        const UINT32 base_word = word_addr(base);
        for (UINT32 i = 0; i < npu_descriptor::WORD_COUNT; ++i) {
            if (!memory.read(base_word + i, raw[i]) || raw[i] < 0) {
                return false;
            }
        }

        reset();
        set_matrix_mnk(static_cast<UINT32>(raw[npu_descriptor::MATRIX_M]),
                       static_cast<UINT32>(raw[npu_descriptor::MATRIX_N]),
                       static_cast<UINT32>(raw[npu_descriptor::MATRIX_K]));
        set_matrix_base(static_cast<UINT32>(raw[npu_descriptor::MATRIX_A_BASE]),
                        static_cast<UINT32>(raw[npu_descriptor::MATRIX_B_BASE]),
                        static_cast<UINT32>(raw[npu_descriptor::MATRIX_C_BASE]));
        set_dataflow(static_cast<dataflow>(raw[npu_descriptor::DATAFLOW]));
        set_tile(static_cast<UINT32>(raw[npu_descriptor::TILE_M]),
                 static_cast<UINT32>(raw[npu_descriptor::TILE_N]),
                 static_cast<UINT32>(raw[npu_descriptor::TILE_K]));

        return validate();
    }

    bool Descriptor::store_to_memory(Memory& memory, UINT32 base) const {
        if (!validate() || !is_word_aligned(base)) {
            return false;
        }

        const INT32 raw[npu_descriptor::WORD_COUNT] = {
            static_cast<INT32>(matrix_m_),
            static_cast<INT32>(matrix_n_),
            static_cast<INT32>(matrix_k_),
            static_cast<INT32>(matrix_A_base_),
            static_cast<INT32>(matrix_B_base_),
            static_cast<INT32>(matrix_C_base_),
            static_cast<INT32>(mode_),
            static_cast<INT32>(tile_m_),
            static_cast<INT32>(tile_n_),
            static_cast<INT32>(tile_k_)
        };

        const UINT32 base_word = word_addr(base);
        for (UINT32 i = 0; i < npu_descriptor::WORD_COUNT; ++i) {
            if (!memory.write(base_word + i, raw[i])) {
                return false;
            }
        }
        return true;
    }
}
