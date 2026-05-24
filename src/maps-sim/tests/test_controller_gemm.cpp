#include <cassert>
#include <iostream>

#include "common/memory.h"
#include "npu/controller.h"

using namespace maps_sim;

static void run_mode_case(dataflow mode) {
    Controller ctrl;
    Memory mem;
    ctrl.bind_memory(&mem);

    Descriptor desc;
    desc.set_matrix_mnk(2, 2, 2);
    desc.set_matrix_base(16, 128, 256);
    desc.set_dataflow(mode);
    desc.set_tile(2, 2, 2);

    assert(ctrl.submit(desc));

    // A = [1 2; 3 4] (row-major)
    INT32 a_data[4] = {1, 2, 3, 4};
    // B = [5 6; 7 8] (row-major)
    INT32 b_data[4] = {5, 6, 7, 8};

    assert(ctrl.load_input_data(a_data, 4));
    assert(ctrl.load_weight_data(b_data, 4));

    assert(ctrl.run());
    assert(ctrl.state() == NpuState::DONE);
    assert(ctrl.error() == NpuError::NONE);

    INT32 out[4] = {0, 0, 0, 0};
    assert(ctrl.read_output_data(out, 4));

    // C = A * B = [19 22; 43 50]
    assert(out[0] == 19);
    assert(out[1] == 22);
    assert(out[2] == 43);
    assert(out[3] == 50);
}

static void test_controller_gemm_2x2_all_modes() {
    run_mode_case(dataflow::WS);
    run_mode_case(dataflow::OS);
    run_mode_case(dataflow::IS);
}

static void test_controller_invalid_desc() {
    Controller ctrl;
    Descriptor bad_desc;
    // keep M/N/K = 0, base = 0 by default, should fail validation

    assert(!ctrl.submit(bad_desc));
    assert(ctrl.state() == NpuState::ERROR);
    assert(ctrl.error() == NpuError::INVALID_DESC);
}

static void test_controller_invalid_tile_shape() {
    Controller ctrl;
    Descriptor bad_desc;
    bad_desc.set_matrix_mnk(2, 2, 2);
    bad_desc.set_matrix_base(16, 128, 256);
    bad_desc.set_dataflow(dataflow::OS);
    bad_desc.set_tile(config::ARRAY_ROWS + 1, 2, 2);

    assert(!ctrl.submit(bad_desc));
    assert(ctrl.state() == NpuState::ERROR);
    assert(ctrl.error() == NpuError::INVALID_DESC);
}

int main() {
    test_controller_gemm_2x2_all_modes();
    test_controller_invalid_desc();
    test_controller_invalid_tile_shape();

    std::cout << "test_controller_gemm: PASS\n";
    return 0;
}
