#ifndef MAPS_SIM_NPU_DESCRIPTOR_H
#define MAPS_SIM_NPU_DESCRIPTOR_H

#include "common/memory.h"
#include "common/types.h"
#include "common/config.h"

namespace maps_sim {

    namespace npu_descriptor {
        inline constexpr UINT32 WORD_COUNT = 10;

        inline constexpr UINT32 MATRIX_M = 0;
        inline constexpr UINT32 MATRIX_N = 1;
        inline constexpr UINT32 MATRIX_K = 2;
        inline constexpr UINT32 MATRIX_A_BASE = 3;
        inline constexpr UINT32 MATRIX_B_BASE = 4;
        inline constexpr UINT32 MATRIX_C_BASE = 5;
        inline constexpr UINT32 DATAFLOW = 6;
        inline constexpr UINT32 TILE_M = 7;
        inline constexpr UINT32 TILE_N = 8;
        inline constexpr UINT32 TILE_K = 9;
    }

    class Descriptor {
        public:
            Descriptor();

            void reset();

            void set_matrix_mnk(UINT32 matrix_m, UINT32 matrix_n, UINT32 matrix_k);
            void set_matrix_base(UINT32 matrix_A_base, UINT32 matrix_B_base, UINT32 matrix_C_base);
            void set_dataflow(dataflow mode);
            void set_tile(UINT32 tile_m, UINT32 tile_n, UINT32 tile_k);

            UINT32 matrix_m() const;
            UINT32 matrix_n() const;
            UINT32 matrix_k() const;

            UINT32 matrix_A_base() const;
            UINT32 matrix_B_base() const;
            UINT32 matrix_C_base() const;

            dataflow mode() const;

            UINT32 tile_m() const;
            UINT32 tile_n() const;
            UINT32 tile_k() const;

            bool validate() const;
            bool load_from_memory(const Memory& memory, UINT32 base);
            bool store_to_memory(Memory& memory, UINT32 base) const;

        private:
            UINT32 matrix_m_;
            UINT32 matrix_n_;
            UINT32 matrix_k_;

            dataflow mode_;

            UINT32 matrix_A_base_;
            UINT32 matrix_B_base_;
            UINT32 matrix_C_base_;

            UINT32 tile_m_;
            UINT32 tile_n_;
            UINT32 tile_k_;
    };
}

#endif // MAPS_SIM_NPU_DESCRIPTOR_H
