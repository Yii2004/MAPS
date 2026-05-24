#include <cassert>
#include <iostream>

#include "common/memory.h"
#include "npu/device.h"

using namespace maps_sim;

static void test_mmio_gemm_start() {
    Memory mem;
    NpuDevice npu;
    npu.bind_memory(&mem);

    const UINT32 desc_base = 64;
    const UINT32 a_base = 128;
    const UINT32 b_base = 256;
    const UINT32 c_base = 384;

    Descriptor desc;
    desc.set_matrix_mnk(2, 2, 2);
    desc.set_matrix_base(a_base, b_base, c_base);
    desc.set_dataflow(dataflow::OS);
    desc.set_tile(2, 2, 2);
    assert(desc.store_to_memory(mem, desc_base));

    const INT32 a[4] = {1, 2, 3, 4};
    const INT32 b[4] = {5, 6, 7, 8};
    assert(mem.load(a_base / sizeof(INT32), a, 4));
    assert(mem.load(b_base / sizeof(INT32), b, 4));

    assert(npu.write32(npu_mmio::DESC_ADDR, desc_base));
    assert(npu.write32(npu_mmio::CMD, npu_mmio::CMD_START));

    UINT32 value = 0;
    assert(npu.read32(npu_mmio::STATUS, value));
    assert(value == static_cast<UINT32>(NpuState::DONE));
    assert(npu.read32(npu_mmio::ERROR, value));
    assert(value == static_cast<UINT32>(NpuError::NONE));

    INT32 c[4] = {};
    assert(mem.dump(c_base / sizeof(INT32), c, 4));
    assert(c[0] == 19);
    assert(c[1] == 22);
    assert(c[2] == 43);
    assert(c[3] == 50);

    assert(npu.read32(npu_mmio::MAC_COUNT_LO, value));
    assert(value == 8);
}

static void test_mmio_invalid_descriptor() {
    Memory mem;
    NpuDevice npu;
    npu.bind_memory(&mem);

    const UINT32 desc_base = 32;
    Descriptor desc;
    desc.set_matrix_mnk(2, 2, 2);
    desc.set_matrix_base(128, 256, 384);
    desc.set_dataflow(dataflow::OS);
    desc.set_tile(config::ARRAY_ROWS + 1, 2, 2);

    const INT32 raw[npu_descriptor::WORD_COUNT] = {
        2, 2, 2, 128, 256, 384, static_cast<INT32>(dataflow::OS),
        static_cast<INT32>(config::ARRAY_ROWS + 1), 2, 2
    };
    assert(mem.load(desc_base / sizeof(INT32), raw, npu_descriptor::WORD_COUNT));

    assert(npu.write32(npu_mmio::DESC_ADDR, desc_base));
    assert(!npu.write32(npu_mmio::CMD, npu_mmio::CMD_START));

    UINT32 value = 0;
    assert(npu.read32(npu_mmio::STATUS, value));
    assert(value == static_cast<UINT32>(NpuState::ERROR));
    assert(npu.read32(npu_mmio::ERROR, value));
    assert(value == static_cast<UINT32>(NpuError::INVALID_DESC));
}

int main() {
    test_mmio_gemm_start();
    test_mmio_invalid_descriptor();

    std::cout << "test_npu_device_mmio: PASS\n";
    return 0;
}
