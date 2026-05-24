#include <cassert>
#include <cstddef>
#include <iostream>

#include "npu/descriptor.h"
#include "npu/device.h"
#include "runtime/npu_driver.h"

using namespace maps_sim;

static void test_descriptor_layout() {
    static_assert(sizeof(maps_npu_desc_t) == MAPS_NPU_DESC_WORDS * sizeof(uint32_t),
                  "runtime descriptor must stay 10 words");
    static_assert(offsetof(maps_npu_desc_t, matrix_m) == npu_descriptor::MATRIX_M * sizeof(uint32_t),
                  "matrix_m offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, matrix_n) == npu_descriptor::MATRIX_N * sizeof(uint32_t),
                  "matrix_n offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, matrix_k) == npu_descriptor::MATRIX_K * sizeof(uint32_t),
                  "matrix_k offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, matrix_a_base) == npu_descriptor::MATRIX_A_BASE * sizeof(uint32_t),
                  "matrix_a_base offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, matrix_b_base) == npu_descriptor::MATRIX_B_BASE * sizeof(uint32_t),
                  "matrix_b_base offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, matrix_c_base) == npu_descriptor::MATRIX_C_BASE * sizeof(uint32_t),
                  "matrix_c_base offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, dataflow) == npu_descriptor::DATAFLOW * sizeof(uint32_t),
                  "dataflow offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, tile_m) == npu_descriptor::TILE_M * sizeof(uint32_t),
                  "tile_m offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, tile_n) == npu_descriptor::TILE_N * sizeof(uint32_t),
                  "tile_n offset mismatch");
    static_assert(offsetof(maps_npu_desc_t, tile_k) == npu_descriptor::TILE_K * sizeof(uint32_t),
                  "tile_k offset mismatch");

    assert(MAPS_NPU_DATAFLOW_WS == static_cast<uint32_t>(dataflow::WS));
    assert(MAPS_NPU_DATAFLOW_OS == static_cast<uint32_t>(dataflow::OS));
    assert(MAPS_NPU_DATAFLOW_IS == static_cast<uint32_t>(dataflow::IS));
    assert(MAPS_NPU_STATE_DONE == static_cast<uint32_t>(NpuState::DONE));
    assert(MAPS_NPU_ERROR_INVALID_DESC == static_cast<uint32_t>(NpuError::INVALID_DESC));
}

static void test_descriptor_init_and_valid() {
    maps_npu_desc_t desc = {};
    maps_npu_desc_init(&desc, 128, 256, 384, 2, 2, 2, MAPS_NPU_DATAFLOW_IS, 2, 2, 2);

    assert(desc.matrix_m == 2);
    assert(desc.matrix_n == 2);
    assert(desc.matrix_k == 2);
    assert(desc.matrix_a_base == 128);
    assert(desc.matrix_b_base == 256);
    assert(desc.matrix_c_base == 384);
    assert(desc.dataflow == MAPS_NPU_DATAFLOW_IS);
    assert(desc.tile_m == 2);
    assert(desc.tile_n == 2);
    assert(desc.tile_k == 2);
    assert(maps_npu_desc_valid(&desc));

    desc.matrix_a_base = 0;
    assert(!maps_npu_desc_valid(&desc));
}

int main() {
    test_descriptor_layout();
    test_descriptor_init_and_valid();

    std::cout << "test_runtime_npu_driver: PASS\n";
    return 0;
}
