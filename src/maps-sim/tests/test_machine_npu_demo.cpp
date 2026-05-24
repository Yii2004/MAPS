#include <cassert>
#include <iostream>
#include <vector>

#include "system/loader.h"
#include "system/machine.h"

using namespace maps_sim;

static UINT32 encode_i(INT32 imm, UINT32 rs1, UINT32 funct3, UINT32 rd, UINT32 opcode) {
    return ((static_cast<UINT32>(imm) & 0xfffU) << 20) |
           (rs1 << 15) |
           (funct3 << 12) |
           (rd << 7) |
           opcode;
}

static UINT32 encode_s(INT32 imm, UINT32 rs2, UINT32 rs1, UINT32 funct3) {
    const UINT32 uimm = static_cast<UINT32>(imm) & 0xfffU;
    return ((uimm >> 5) << 25) |
           (rs2 << 20) |
           (rs1 << 15) |
           (funct3 << 12) |
           ((uimm & 0x1fU) << 7) |
           0x23U;
}

static UINT32 encode_b(INT32 imm, UINT32 rs2, UINT32 rs1, UINT32 funct3) {
    const UINT32 uimm = static_cast<UINT32>(imm) & 0x1fffU;
    return (((uimm >> 12) & 0x1U) << 31) |
           (((uimm >> 5) & 0x3fU) << 25) |
           (rs2 << 20) |
           (rs1 << 15) |
           (funct3 << 12) |
           (((uimm >> 1) & 0xfU) << 8) |
           (((uimm >> 11) & 0x1U) << 7) |
           0x63U;
}

static UINT32 encode_u(UINT32 imm, UINT32 rd, UINT32 opcode) {
    return (imm & 0xfffff000U) | (rd << 7) | opcode;
}

static void test_loader_bytes(Machine& machine) {
    const UINT8 bytes[4] = {0x78, 0x56, 0x34, 0x12};
    UINT32 value = 0;

    assert(BinaryLoader::load_bytes(machine.bus(), 512, bytes, 4));
    assert(machine.bus().read32(512, value));
    assert(value == 0x12345678U);
}

static void test_machine_cpu_npu_demo() {
    Machine machine;

    const UINT32 desc_base = 64;
    const UINT32 a_base = 128;
    const UINT32 b_base = 256;
    const UINT32 c_base = 384;

    Descriptor desc;
    desc.set_matrix_mnk(2, 2, 2);
    desc.set_matrix_base(a_base, b_base, c_base);
    desc.set_dataflow(dataflow::OS);
    desc.set_tile(2, 2, 2);
    assert(desc.store_to_memory(machine.memory(), desc_base));

    const INT32 a[4] = {1, 2, 3, 4};
    const INT32 b[4] = {5, 6, 7, 8};
    assert(machine.memory().load(a_base / sizeof(INT32), a, 4));
    assert(machine.memory().load(b_base / sizeof(INT32), b, 4));

    const std::vector<UINT32> program = {
        encode_i(desc_base, 0, 0x0, 1, 0x13),                          // addi x1, x0, desc_base
        encode_u(address_map::NPU_BASE, 10, 0x37),                     // lui x10, NPU_BASE
        encode_s(npu_mmio::DESC_ADDR, 1, 10, 0x2),                     // sw x1, DESC_ADDR(x10)
        encode_i(npu_mmio::CMD_START, 0, 0x0, 2, 0x13),                // addi x2, x0, CMD_START
        encode_s(npu_mmio::CMD, 2, 10, 0x2),                           // sw x2, CMD(x10)
        encode_i(npu_mmio::STATUS, 10, 0x2, 3, 0x03),                  // lw x3, STATUS(x10)
        encode_i(static_cast<INT32>(NpuState::DONE), 0, 0x0, 4, 0x13), // addi x4, x0, DONE
        encode_b(8, 4, 3, 0x0),                                        // beq x3, x4, +8
        encode_i(99, 0, 0x0, 5, 0x13),                                 // skipped on success
        encode_i(static_cast<INT32>(c_base), 0, 0x2, 6, 0x03), // lw x6, C[0](x0)
        0x00000073U                                                    // ecall
    };

    assert(BinaryLoader::load_words(machine.memory(), 0, program.data(),
                                    static_cast<UINT32>(program.size())));

    machine.reset(0);
    assert(machine.run(200));
    assert(machine.cpu().halted());
    assert(!machine.cpu().faulted());
    assert(machine.cpu().reg(3) == static_cast<UINT32>(NpuState::DONE));
    assert(machine.cpu().reg(5) == 0);
    assert(machine.cpu().reg(6) == 19);

    INT32 c[4] = {};
    assert(machine.memory().dump(c_base / sizeof(INT32), c, 4));
    assert(c[0] == 19);
    assert(c[1] == 22);
    assert(c[2] == 43);
    assert(c[3] == 50);
}

int main() {
    Machine machine;
    test_loader_bytes(machine);
    test_machine_cpu_npu_demo();

    std::cout << "test_machine_npu_demo: PASS\n";
    return 0;
}
