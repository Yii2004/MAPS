#include <cassert>
#include <iostream>
#include <string>

#include "common/bus.h"

using namespace maps_sim;

static UINT32 dram_addr(UINT32 byte_addr) {
    return address_map::DRAM_BASE + byte_addr;
}

static UINT32 npu_addr(UINT32 offset) {
    return address_map::NPU_BASE + offset;
}

static UINT32 console_addr(UINT32 offset) {
    return address_map::CONSOLE_BASE + offset;
}

static void test_dram_access() {
    Memory mem(16);
    Bus bus;
    bus.bind_memory(&mem);

    UINT32 value = 0;
    assert(bus.write32(3 * sizeof(UINT32), 0x12345678));
    assert(bus.read32(3 * sizeof(UINT32), value));
    assert(value == 0x12345678);

    assert(!bus.write32(3 * sizeof(UINT32) + 1, 0));
    assert(!bus.read32(16 * sizeof(UINT32), value));
}

static void test_npu_mmio_gemm() {
    Memory mem;
    NpuDevice npu;
    Bus bus;

    npu.bind_memory(&mem);
    bus.bind_memory(&mem);
    bus.bind_npu(&npu);

    const UINT32 desc_base = 64;
    const UINT32 a_base = 128;
    const UINT32 b_base = 256;
    const UINT32 c_base = 384;

    Descriptor desc;
    desc.set_matrix_mnk(2, 2, 2);
    desc.set_matrix_base(a_base, b_base, c_base);
    desc.set_dataflow(dataflow::WS);
    desc.set_tile(2, 2, 2);
    assert(desc.store_to_memory(mem, desc_base));

    const INT32 a[4] = {1, 2, 3, 4};
    const INT32 b[4] = {5, 6, 7, 8};
    assert(mem.load(a_base / sizeof(INT32), a, 4));
    assert(mem.load(b_base / sizeof(INT32), b, 4));

    assert(bus.write32(npu_addr(npu_mmio::DESC_ADDR), desc_base));
    assert(bus.write32(npu_addr(npu_mmio::CMD), npu_mmio::CMD_START));

    UINT32 value = 0;
    assert(bus.read32(npu_addr(npu_mmio::STATUS), value));
    assert(value == static_cast<UINT32>(NpuState::DONE));
    assert(bus.read32(npu_addr(npu_mmio::ERROR), value));
    assert(value == static_cast<UINT32>(NpuError::NONE));

    assert(bus.read32(dram_addr(c_base), value));
    assert(static_cast<INT32>(value) == 19);
    assert(bus.read32(dram_addr(c_base + sizeof(INT32)), value));
    assert(static_cast<INT32>(value) == 22);
    assert(bus.read32(dram_addr(c_base + 2 * sizeof(INT32)), value));
    assert(static_cast<INT32>(value) == 43);
    assert(bus.read32(dram_addr(c_base + 3 * sizeof(INT32)), value));
    assert(static_cast<INT32>(value) == 50);

    assert(bus.read32(npu_addr(npu_mmio::MAC_COUNT_LO), value));
    assert(value == 8);
}

static void test_console_mmio() {
    Bus bus;
    std::string output;
    bus.bind_console(&output);

    UINT32 status = 0;
    assert(bus.write8(console_addr(console_mmio::DATA), 'O'));
    assert(bus.write32(console_addr(console_mmio::DATA), 'K'));
    assert(bus.read32(console_addr(console_mmio::STATUS), status));
    assert(status == 1);
    assert(output == "OK");
}

static void test_unmapped_access() {
    Bus bus;
    UINT32 value = 0;

    assert(!bus.read32(dram_addr(0), value));
    assert(!bus.write32(dram_addr(0), 0));
    assert(!bus.read32(address_map::NPU_BASE + address_map::NPU_SIZE, value));
    assert(!bus.write32(address_map::NPU_BASE + address_map::NPU_SIZE, 0));
}

int main() {
    test_dram_access();
    test_npu_mmio_gemm();
    test_console_mmio();
    test_unmapped_access();

    std::cout << "test_bus_mmio_npu: PASS\n";
    return 0;
}
