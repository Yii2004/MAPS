#include <cassert>
#include <iostream>
#include <vector>

#include "cpu/cpu.h"

using namespace maps_sim;

static UINT32 encode_r(UINT32 funct7, UINT32 rs2, UINT32 rs1, UINT32 funct3, UINT32 rd, UINT32 opcode) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

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

static UINT32 encode_j(INT32 imm, UINT32 rd) {
    const UINT32 uimm = static_cast<UINT32>(imm) & 0x1fffffU;
    return (((uimm >> 20) & 0x1U) << 31) |
           (((uimm >> 1) & 0x3ffU) << 21) |
           (((uimm >> 11) & 0x1U) << 20) |
           (((uimm >> 12) & 0xffU) << 12) |
           (rd << 7) |
           0x6fU;
}

static UINT32 encode_csr(UINT32 csr, UINT32 rs1, UINT32 funct3, UINT32 rd) {
    return (csr << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x73U;
}

static void load_program(Memory& memory, const std::vector<UINT32>& program) {
    for (UINT32 i = 0; i < program.size(); ++i) {
        assert(memory.write(i, static_cast<INT32>(program[i])));
    }
}

static void test_integer_branch_and_memory() {
    Memory mem(256);
    Bus bus;
    Cpu cpu(&bus);
    bus.bind_memory(&mem);

    const std::vector<UINT32> program = {
        encode_i(5, 0, 0x0, 1, 0x13),       // addi x1, x0, 5
        encode_i(7, 0, 0x0, 2, 0x13),       // addi x2, x0, 7
        encode_r(0, 2, 1, 0x0, 3, 0x33),    // add x3, x1, x2
        encode_i(64, 0, 0x0, 4, 0x13),      // addi x4, x0, 64
        encode_s(0, 3, 4, 0x2),             // sw x3, 0(x4)
        encode_i(0, 4, 0x2, 5, 0x03),       // lw x5, 0(x4)
        encode_b(8, 5, 3, 0x0),             // beq x3, x5, +8
        encode_i(1, 0, 0x0, 6, 0x13),       // skipped
        encode_i(1, 0, 0x0, 6, 0x13),       // addi x6, x0, 1
        0x00000073U                         // ecall
    };
    load_program(mem, program);

    cpu.reset(0);
    assert(cpu.run(100));
    assert(cpu.halted());
    assert(!cpu.faulted());
    assert(cpu.reg(3) == 12);
    assert(cpu.reg(5) == 12);
    assert(cpu.reg(6) == 1);

    INT32 value = 0;
    assert(mem.read(16, value));
    assert(value == 12);
}

static void test_byte_halfword_and_jumps() {
    Memory mem(256);
    Bus bus;
    Cpu cpu(&bus);
    bus.bind_memory(&mem);

    const std::vector<UINT32> program = {
        encode_i(80, 0, 0x0, 1, 0x13),      // addi x1, x0, 80
        encode_i(-1, 0, 0x0, 2, 0x13),      // addi x2, x0, -1
        encode_s(0, 2, 1, 0x0),             // sb x2, 0(x1)
        encode_i(0, 1, 0x0, 3, 0x03),       // lb x3, 0(x1)
        encode_i(0, 1, 0x4, 4, 0x03),       // lbu x4, 0(x1)
        encode_i(0x234, 0, 0x0, 5, 0x13),   // addi x5, x0, 0x234
        encode_s(2, 5, 1, 0x1),             // sh x5, 2(x1)
        encode_i(2, 1, 0x1, 6, 0x03),       // lh x6, 2(x1)
        encode_j(8, 7),                     // jal x7, +8
        encode_i(1, 0, 0x0, 8, 0x13),       // skipped
        encode_i(1, 0, 0x0, 8, 0x13),       // addi x8, x0, 1
        0x00000073U                         // ecall
    };
    load_program(mem, program);

    cpu.reset(0);
    assert(cpu.run(100));
    assert(cpu.reg(3) == 0xffffffffU);
    assert(cpu.reg(4) == 0xffU);
    assert(cpu.reg(6) == 0x234U);
    assert(cpu.reg(7) == 36);
    assert(cpu.reg(8) == 1);
}

static void test_fault_on_unknown_instruction() {
    Memory mem(16);
    Bus bus;
    Cpu cpu(&bus);
    bus.bind_memory(&mem);
    assert(mem.write(0, 0));

    cpu.reset(0);
    assert(!cpu.step());
    assert(cpu.faulted());
}

static void test_ecall_trap_and_mret() {
    Memory mem(64);
    Bus bus;
    Cpu cpu(&bus);
    bus.bind_memory(&mem);

    std::vector<UINT32> program(20, encode_i(0, 0, 0x0, 0, 0x13));
    program[0] = encode_i(64, 0, 0x0, 1, 0x13);       // addi x1, x0, trap_vector
    program[1] = encode_csr(0x305, 1, 0x1, 0);        // csrw mtvec, x1
    program[2] = encode_i(7, 0, 0x0, 10, 0x13);       // addi a0, x0, 7
    program[3] = 0x00000073U;                         // ecall -> mtvec
    program[4] = encode_i(1, 0, 0x0, 11, 0x13);       // addi a1, x0, 1
    program[5] = encode_csr(0x305, 0, 0x1, 0);        // csrw mtvec, x0
    program[6] = 0x00000073U;                         // host halt
    program[16] = encode_i(1, 10, 0x0, 10, 0x13);     // addi a0, a0, 1
    program[17] = 0x30200073U;                        // mret

    load_program(mem, program);
    cpu.reset(0);
    assert(cpu.run(100));
    assert(cpu.halted());
    assert(!cpu.faulted());
    assert(cpu.reg(10) == 8);
    assert(cpu.reg(11) == 1);
}

int main() {
    test_integer_branch_and_memory();
    test_byte_halfword_and_jumps();
    test_fault_on_unknown_instruction();
    test_ecall_trap_and_mret();

    std::cout << "test_cpu_basic: PASS\n";
    return 0;
}
