#include <cassert>
#include <iostream>
#include <vector>

#include "common/memory.h"
#include "npu/controller.h"

using namespace maps_sim;

static std::vector<INT32> gemm_golden(
    const std::vector<INT32>& a,
    const std::vector<INT32>& b,
    UINT32 m,
    UINT32 n,
    UINT32 k) {
    std::vector<INT32> c(m * n, 0);
    for (UINT32 i = 0; i < m; ++i) {
        for (UINT32 j = 0; j < n; ++j) {
            INT32 acc = 0;
            for (UINT32 p = 0; p < k; ++p) {
                acc += a[i * k + p] * b[p * n + j];
            }
            c[i * n + j] = acc;
        }
    }
    return c;
}

static void run_case(UINT32 m, UINT32 n, UINT32 k, UINT32 tm, UINT32 tn, UINT32 tk, dataflow mode) {
    Controller ctrl;
    Memory mem;
    ctrl.bind_memory(&mem);
    Descriptor desc;
    desc.set_matrix_mnk(m, n, k);
    desc.set_matrix_base(1024, 2048, 3072);
    desc.set_dataflow(mode);
    desc.set_tile(tm, tn, tk);

    assert(ctrl.submit(desc));

    std::vector<INT32> a(m * k, 0);
    std::vector<INT32> b(k * n, 0);

    // deterministic pattern values, avoid all-zero trivial case
    for (UINT32 i = 0; i < m * k; ++i) {
        a[i] = static_cast<INT32>((i % 7) - 3);
    }
    for (UINT32 i = 0; i < k * n; ++i) {
        b[i] = static_cast<INT32>((i % 5) - 2);
    }

    assert(ctrl.load_input_data(a.data(), static_cast<UINT32>(a.size())));
    assert(ctrl.load_weight_data(b.data(), static_cast<UINT32>(b.size())));
    assert(ctrl.run());
    assert(ctrl.state() == NpuState::DONE);
    assert(ctrl.error() == NpuError::NONE);

    std::vector<INT32> out(m * n, 0);
    assert(ctrl.read_output_data(out.data(), static_cast<UINT32>(out.size())));

    const std::vector<INT32> golden = gemm_golden(a, b, m, n, k);
    assert(out.size() == golden.size());
    for (UINT32 i = 0; i < out.size(); ++i) {
        assert(out[i] == golden[i]);
    }
}

int main() {
    // Even tiles
    run_case(4, 4, 4, 2, 2, 2, dataflow::WS);
    run_case(4, 4, 4, 2, 2, 2, dataflow::OS);
    run_case(4, 4, 4, 2, 2, 2, dataflow::IS);

    // Tail tiles (non-divisible)
    run_case(5, 3, 7, 2, 2, 3, dataflow::WS);
    run_case(5, 3, 7, 2, 2, 3, dataflow::OS);
    run_case(5, 3, 7, 2, 2, 3, dataflow::IS);

    std::cout << "test_end2end_npu: PASS\n";
    return 0;
}
