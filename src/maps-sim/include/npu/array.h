#ifndef MAPS_SIM_NPU_ARRAY_H
#define MAPS_SIM_NPU_ARRAY_H

#include "common/config.h"
#include "npu/pe.h"

namespace maps_sim {

    class Array {
        public:
            Array();

            void reset();

            bool run_gemm_tile(dataflow mode,
                               const INT32* a_tile,
                               const INT32* b_tile,
                               const INT32* c_in_tile,
                               UINT32 m_len,
                               UINT32 n_len,
                               UINT32 k_len,
                               INT32* c_out_tile,
                               UINT32* cycles_out);

        private:
            bool dims_fit(dataflow mode, UINT32 m_len, UINT32 n_len, UINT32 k_len) const;
            void set_array_mode(dataflow mode);
            void tick_grid(const INT32* left_boundary, const INT32* top_boundary);
            INT32 c_in_at(const INT32* c_in_tile, UINT32 row, UINT32 col, UINT32 n_len) const;

            bool run_os_tile(const INT32* a_tile, const INT32* b_tile,
                             const INT32* c_in_tile, UINT32 m_len, UINT32 n_len,
                             UINT32 k_len, INT32* c_out_tile, UINT32* cycles_out);
            bool run_ws_tile(const INT32* a_tile, const INT32* b_tile,
                             const INT32* c_in_tile, UINT32 m_len, UINT32 n_len,
                             UINT32 k_len, INT32* c_out_tile, UINT32* cycles_out);
            bool run_is_tile(const INT32* a_tile, const INT32* b_tile,
                             const INT32* c_in_tile, UINT32 m_len, UINT32 n_len,
                             UINT32 k_len, INT32* c_out_tile, UINT32* cycles_out);

            PE pe_array_[config::ARRAY_ROWS][config::ARRAY_COLS];
    };
}

#endif // MAPS_SIM_NPU_ARRAY_H
