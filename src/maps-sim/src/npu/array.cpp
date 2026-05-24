#include "npu/array.h"

#include <algorithm>

namespace maps_sim {

    Array::Array() {
        reset();
    }

    void Array::reset() {
        for (UINT32 i = 0; i < config::ARRAY_ROWS; ++i) {
            for (UINT32 j = 0; j < config::ARRAY_COLS; ++j) {
                pe_array_[i][j].reset();
            }
        }
    }

    bool Array::run_gemm_tile(dataflow mode,
                              const INT32* a_tile,
                              const INT32* b_tile,
                              const INT32* c_in_tile,
                              UINT32 m_len,
                              UINT32 n_len,
                              UINT32 k_len,
                              INT32* c_out_tile,
                              UINT32* cycles_out) {
        if (cycles_out != nullptr) {
            *cycles_out = 0;
        }
        if (a_tile == nullptr || b_tile == nullptr || c_out_tile == nullptr ||
            m_len == 0 || n_len == 0 || k_len == 0 ||
            !dims_fit(mode, m_len, n_len, k_len)) {
            return false;
        }

        switch (mode) {
            case dataflow::OS:
                return run_os_tile(a_tile, b_tile, c_in_tile, m_len, n_len, k_len,
                                   c_out_tile, cycles_out);
            case dataflow::WS:
                return run_ws_tile(a_tile, b_tile, c_in_tile, m_len, n_len, k_len,
                                   c_out_tile, cycles_out);
            case dataflow::IS:
                return run_is_tile(a_tile, b_tile, c_in_tile, m_len, n_len, k_len,
                                   c_out_tile, cycles_out);
            default:
                return false;
        }
    }

    bool Array::dims_fit(dataflow mode, UINT32 m_len, UINT32 n_len, UINT32 k_len) const {
        switch (mode) {
            case dataflow::OS:
                return m_len <= config::ARRAY_ROWS && n_len <= config::ARRAY_COLS;
            case dataflow::WS:
                return k_len <= config::ARRAY_ROWS && n_len <= config::ARRAY_COLS;
            case dataflow::IS:
                return m_len <= config::ARRAY_ROWS && k_len <= config::ARRAY_COLS;
            default:
                return false;
        }
    }

    void Array::set_array_mode(dataflow mode) {
        for (UINT32 i = 0; i < config::ARRAY_ROWS; ++i) {
            for (UINT32 j = 0; j < config::ARRAY_COLS; ++j) {
                pe_array_[i][j].set_mode(mode);
            }
        }
    }

    void Array::tick_grid(const INT32* left_boundary, const INT32* top_boundary) {
        INT32 lhs_inputs[config::ARRAY_ROWS][config::ARRAY_COLS];
        INT32 rhs_inputs[config::ARRAY_ROWS][config::ARRAY_COLS];

        for (UINT32 i = 0; i < config::ARRAY_ROWS; ++i) {
            for (UINT32 j = 0; j < config::ARRAY_COLS; ++j) {
                lhs_inputs[i][j] = (j == 0) ? left_boundary[i] : pe_array_[i][j - 1].output0();
                rhs_inputs[i][j] = (i == 0) ? top_boundary[j] : pe_array_[i - 1][j].output1();
            }
        }

        for (UINT32 i = 0; i < config::ARRAY_ROWS; ++i) {
            for (UINT32 j = 0; j < config::ARRAY_COLS; ++j) {
                pe_array_[i][j].set_inputs(lhs_inputs[i][j], rhs_inputs[i][j]);
            }
        }

        for (UINT32 i = 0; i < config::ARRAY_ROWS; ++i) {
            for (UINT32 j = 0; j < config::ARRAY_COLS; ++j) {
                pe_array_[i][j].tick();
            }
        }
    }

    INT32 Array::c_in_at(const INT32* c_in_tile, UINT32 row, UINT32 col, UINT32 n_len) const {
        return (c_in_tile == nullptr) ? 0 : c_in_tile[row * n_len + col];
    }

    bool Array::run_os_tile(const INT32* a_tile, const INT32* b_tile,
                            const INT32* c_in_tile, UINT32 m_len, UINT32 n_len,
                            UINT32 k_len, INT32* c_out_tile, UINT32* cycles_out) {
        reset();
        set_array_mode(dataflow::OS);

        for (UINT32 i = 0; i < m_len; ++i) {
            for (UINT32 j = 0; j < n_len; ++j) {
                pe_array_[i][j].load_psum(c_in_at(c_in_tile, i, j, n_len));
            }
        }

        const UINT32 cycles = m_len + n_len + k_len - 2;
        for (UINT32 cyc = 0; cyc < cycles; ++cyc) {
            INT32 left[config::ARRAY_ROWS] = {};
            INT32 top[config::ARRAY_COLS] = {};

            for (UINT32 i = 0; i < m_len; ++i) {
                if (cyc >= i) {
                    const UINT32 k = cyc - i;
                    if (k < k_len) {
                        left[i] = a_tile[i * k_len + k];
                    }
                }
            }
            for (UINT32 j = 0; j < n_len; ++j) {
                if (cyc >= j) {
                    const UINT32 k = cyc - j;
                    if (k < k_len) {
                        top[j] = b_tile[k * n_len + j];
                    }
                }
            }

            tick_grid(left, top);
        }

        for (UINT32 i = 0; i < m_len; ++i) {
            for (UINT32 j = 0; j < n_len; ++j) {
                c_out_tile[i * n_len + j] = pe_array_[i][j].psum();
            }
        }

        if (cycles_out != nullptr) {
            *cycles_out = cycles;
        }
        return true;
    }

    bool Array::run_ws_tile(const INT32* a_tile, const INT32* b_tile,
                            const INT32* c_in_tile, UINT32 m_len, UINT32 n_len,
                            UINT32 k_len, INT32* c_out_tile, UINT32* cycles_out) {
        reset();
        set_array_mode(dataflow::WS);
        std::fill(c_out_tile, c_out_tile + (m_len * n_len), 0);

        for (UINT32 k = 0; k < k_len; ++k) {
            for (UINT32 j = 0; j < n_len; ++j) {
                pe_array_[k][j].load_weight(b_tile[k * n_len + j]);
            }
        }

        const UINT32 cycles = m_len + n_len + k_len - 2;
        for (UINT32 cyc = 0; cyc < cycles; ++cyc) {
            INT32 left[config::ARRAY_ROWS] = {};
            INT32 top[config::ARRAY_COLS] = {};

            for (UINT32 k = 0; k < k_len; ++k) {
                if (cyc >= k) {
                    const UINT32 m = cyc - k;
                    if (m < m_len) {
                        left[k] = a_tile[m * k_len + k];
                    }
                }
            }
            for (UINT32 j = 0; j < n_len; ++j) {
                if (cyc >= j) {
                    const UINT32 m = cyc - j;
                    if (m < m_len) {
                        top[j] = c_in_at(c_in_tile, m, j, n_len);
                    }
                }
            }

            tick_grid(left, top);

            for (UINT32 j = 0; j < n_len; ++j) {
                const UINT32 ready_cycle = j + k_len - 1;
                if (cyc >= ready_cycle) {
                    const UINT32 m = cyc - ready_cycle;
                    if (m < m_len) {
                        c_out_tile[m * n_len + j] = pe_array_[k_len - 1][j].output1();
                    }
                }
            }
        }

        if (cycles_out != nullptr) {
            *cycles_out = cycles;
        }
        return true;
    }

    bool Array::run_is_tile(const INT32* a_tile, const INT32* b_tile,
                            const INT32* c_in_tile, UINT32 m_len, UINT32 n_len,
                            UINT32 k_len, INT32* c_out_tile, UINT32* cycles_out) {
        reset();
        set_array_mode(dataflow::IS);
        std::fill(c_out_tile, c_out_tile + (m_len * n_len), 0);

        for (UINT32 i = 0; i < m_len; ++i) {
            for (UINT32 k = 0; k < k_len; ++k) {
                pe_array_[i][k].load_input(a_tile[i * k_len + k]);
            }
        }

        const UINT32 cycles = m_len + n_len + k_len - 2;
        for (UINT32 cyc = 0; cyc < cycles; ++cyc) {
            INT32 left[config::ARRAY_ROWS] = {};
            INT32 top[config::ARRAY_COLS] = {};

            for (UINT32 i = 0; i < m_len; ++i) {
                if (cyc >= i) {
                    const UINT32 n = cyc - i;
                    if (n < n_len) {
                        left[i] = c_in_at(c_in_tile, i, n, n_len);
                    }
                }
            }
            for (UINT32 k = 0; k < k_len; ++k) {
                if (cyc >= k) {
                    const UINT32 n = cyc - k;
                    if (n < n_len) {
                        top[k] = b_tile[k * n_len + n];
                    }
                }
            }

            tick_grid(left, top);

            for (UINT32 i = 0; i < m_len; ++i) {
                const UINT32 ready_cycle = i + k_len - 1;
                if (cyc >= ready_cycle) {
                    const UINT32 n = cyc - ready_cycle;
                    if (n < n_len) {
                        c_out_tile[i * n_len + n] = pe_array_[i][k_len - 1].output0();
                    }
                }
            }
        }

        if (cycles_out != nullptr) {
            *cycles_out = cycles;
        }
        return true;
    }

}
