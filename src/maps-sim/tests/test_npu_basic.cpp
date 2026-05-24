#include <cassert>
#include <iostream>
#include <vector>

#include "npu/array.h"
#include "npu/pe.h"

using namespace maps_sim;

static std::vector<INT32> gemm_golden(const std::vector<INT32>& a,
                                      const std::vector<INT32>& b,
                                      UINT32 m,
                                      UINT32 n,
                                      UINT32 k) {
    std::vector<INT32> c(m * n, 0);
    for (UINT32 i = 0; i < m; ++i) {
        for (UINT32 j = 0; j < n; ++j) {
            for (UINT32 p = 0; p < k; ++p) {
                c[i * n + j] += a[i * k + p] * b[p * n + j];
            }
        }
    }
    return c;
}

static void assert_eq(const std::vector<INT32>& actual,
                      const std::vector<INT32>& expected) {
    assert(actual.size() == expected.size());
    for (UINT32 i = 0; i < actual.size(); ++i) {
        assert(actual[i] == expected[i]);
    }
}

static void test_pe_modes() {
    PE pe;

    pe.set_mode(dataflow::OS);
    pe.load_psum(5);
    pe.set_inputs(2, 3);
    pe.tick();
    assert(pe.output0() == 2);
    assert(pe.output1() == 3);
    assert(pe.psum() == 11);

    pe.reset();
    pe.set_mode(dataflow::WS);
    pe.load_weight(3);
    pe.set_inputs(4, 5);
    pe.tick();
    assert(pe.output0() == 4);
    assert(pe.output1() == 17);
    assert(pe.psum() == 17);

    pe.reset();
    pe.set_mode(dataflow::IS);
    pe.load_input(3);
    pe.set_inputs(5, 4);
    pe.tick();
    assert(pe.output0() == 17);
    assert(pe.output1() == 4);
    assert(pe.psum() == 17);
}

static void test_array_2x2(dataflow mode) {
    Array array;
    const std::vector<INT32> a = {1, 2,
                                  3, 4};
    const std::vector<INT32> b = {5, 6,
                                  7, 8};
    std::vector<INT32> out(4, 0);
    UINT32 cycles = 0;

    assert(array.run_gemm_tile(mode, a.data(), b.data(), nullptr, 2, 2, 2,
                               out.data(), &cycles));
    assert(cycles == 4);
    assert_eq(out, std::vector<INT32>({19, 22, 43, 50}));
}

static void test_array_split_k(dataflow mode) {
    Array array;
    const std::vector<INT32> a0 = {1, 3};
    const std::vector<INT32> b0 = {5, 6};
    std::vector<INT32> partial(4, 0);
    UINT32 cycles = 0;

    assert(array.run_gemm_tile(mode, a0.data(), b0.data(), nullptr, 2, 2, 1,
                               partial.data(), &cycles));
    assert_eq(partial, std::vector<INT32>({5, 6, 15, 18}));

    const std::vector<INT32> a1 = {2, 4};
    const std::vector<INT32> b1 = {7, 8};
    std::vector<INT32> out(4, 0);
    assert(array.run_gemm_tile(mode, a1.data(), b1.data(), partial.data(), 2, 2, 1,
                               out.data(), &cycles));
    assert_eq(out, std::vector<INT32>({19, 22, 43, 50}));
}

static void test_array_tail_tile(dataflow mode) {
    Array array;
    const UINT32 m = 3;
    const UINT32 n = 2;
    const UINT32 k = 3;
    const std::vector<INT32> a = {1, 2, 3,
                                  4, 5, 6,
                                  7, 8, 9};
    const std::vector<INT32> b = {1, 2,
                                  3, 4,
                                  5, 6};
    std::vector<INT32> out(m * n, 0);

    assert(array.run_gemm_tile(mode, a.data(), b.data(), nullptr, m, n, k,
                               out.data(), nullptr));
    assert_eq(out, gemm_golden(a, b, m, n, k));
}

int main() {
    test_pe_modes();

    test_array_2x2(dataflow::OS);
    test_array_2x2(dataflow::WS);
    test_array_2x2(dataflow::IS);

    test_array_split_k(dataflow::OS);
    test_array_split_k(dataflow::WS);
    test_array_split_k(dataflow::IS);

    test_array_tail_tile(dataflow::OS);
    test_array_tail_tile(dataflow::WS);
    test_array_tail_tile(dataflow::IS);

    std::cout << "test_npu_basic: PASS\n";
    return 0;
}
