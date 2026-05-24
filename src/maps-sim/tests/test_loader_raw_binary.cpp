#include <cassert>
#include <cstdio>
#include <fstream>
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

static bool write_raw_binary(const char* path, const std::vector<UINT32>& words) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    for (UINT32 word : words) {
        const char bytes[4] = {
            static_cast<char>(word & 0xffU),
            static_cast<char>((word >> 8) & 0xffU),
            static_cast<char>((word >> 16) & 0xffU),
            static_cast<char>((word >> 24) & 0xffU)
        };
        file.write(bytes, sizeof(bytes));
        if (!file) {
            return false;
        }
    }
    return true;
}

int main() {
    const char* path = "test_loader_raw_binary_program.bin";
    const std::vector<UINT32> program = {
        encode_i(11, 0, 0x0, 1, 0x13), // addi x1, x0, 11
        encode_i(31, 0, 0x0, 2, 0x13), // addi x2, x0, 31
        encode_i(96, 0, 0x0, 3, 0x13), // addi x3, x0, 96
        encode_s(0, 2, 3, 0x2),        // sw x2, 0(x3)
        encode_i(0, 3, 0x2, 4, 0x03),  // lw x4, 0(x3)
        0x00000073U                    // ecall
    };

    assert(write_raw_binary(path, program));

    Machine machine;
    assert(BinaryLoader::load_file(machine.bus(), 0, path));
    machine.reset(0);
    assert(machine.run(100));

    assert(machine.cpu().reg(1) == 11);
    assert(machine.cpu().reg(2) == 31);
    assert(machine.cpu().reg(4) == 31);

    INT32 value = 0;
    assert(machine.memory().read(24, value));
    assert(value == 31);

    std::remove(path);
    std::cout << "test_loader_raw_binary: PASS\n";
    return 0;
}
