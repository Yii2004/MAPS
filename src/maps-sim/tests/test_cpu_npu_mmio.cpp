#include <cassert>
#include <iostream>
#include <vector>

#include "cpu/cpu.h"
#include "npu/device.h"

using namespace maps_sim;

static UINT32 encode_i(INT32 imm, UINT32 rs1, UINT32 funct3, UINT32 rd, UINT32 opcode) {
    return ((static_cast<UINT32>(imm) & 0xfffU) << 20) | (rs1 << 15) |
           (funct3 << 12) | (rd << 7) | opcode;
}

static UINT32 encode_s(INT32 imm, UINT32 rs2, UINT32 rs1, UINT32 funct3) {
    const UINT32 uimm = static_cast<UINT32>(imm) & 0xfffU;
    return ((uimm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) |
           (funct3 << 12) | ((uimm & 0x1fU) << 7) | 0x23U;
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

static void load_program(Memory& memory, const std::vector<UINT32>& program) {
    for (UINT32 i = 0; i < program.size(); ++i) {
        assert(memory.write(i, static_cast<INT32>(program[i])));
    }
}

int main() {
    Memory mem;
    NpuDevice npu;
    Bus bus;
    Cpu cpu(&bus);

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
    desc.set_dataflow(dataflow::IS);
    desc.set_tile(2, 2, 2);
    assert(desc.store_to_memory(mem, desc_base));

    const INT32 a[4] = {1, 2, 3, 4};
    const INT32 b[4] = {5, 6, 7, 8};
    assert(mem.load(a_base / sizeof(INT32), a, 4));
    assert(mem.load(b_base / sizeof(INT32), b, 4));

    const std::vector<UINT32> program = {
        encode_i(desc_base, 0, 0x0, 1, 0x13),                         // addi x1, x0, desc_base
        encode_u(address_map::NPU_BASE, 10, 0x37),                    // lui x10, NPU_BASE
        encode_s(npu_mmio::DESC_ADDR, 1, 10, 0x2),                    // sw x1, DESC_ADDR(x10)
        encode_i(npu_mmio::CMD_START, 0, 0x0, 2, 0x13),               // addi x2, x0, CMD_START
        encode_s(npu_mmio::CMD, 2, 10, 0x2),                          // sw x2, CMD(x10)
        encode_i(npu_mmio::STATUS, 10, 0x2, 3, 0x03),                 // lw x3, STATUS(x10)
        encode_i(static_cast<INT32>(NpuState::DONE), 0, 0x0, 4, 0x13),// addi x4, x0, DONE
        encode_b(8, 4, 3, 0x0),                                       // beq x3, x4, +8
        encode_i(99, 0, 0x0, 5, 0x13),                                // skipped on success
        encode_i(static_cast<INT32>(c_base), 0, 0x2, 6, 0x03), // lw x6, C[0](x0)
        0x00000073U                                                   // ecall
    };
    load_program(mem, program);

    cpu.reset(0);
    assert(cpu.run(200));
    assert(cpu.reg(3) == static_cast<UINT32>(NpuState::DONE));
    assert(cpu.reg(5) == 0);
    assert(cpu.reg(6) == 19);

    INT32 c[4] = {};
    assert(mem.dump(c_base / sizeof(INT32), c, 4));
    assert(c[0] == 19);
    assert(c[1] == 22);
    assert(c[2] == 43);
    assert(c[3] == 50);

    std::cout << "test_cpu_npu_mmio: PASS\n";
    return 0;
}
