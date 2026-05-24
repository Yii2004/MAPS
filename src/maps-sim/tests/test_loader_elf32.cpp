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

static void put16(std::vector<UINT8>& bytes, UINT32 offset, UINT16 value) {
    bytes[offset] = static_cast<UINT8>(value & 0xffU);
    bytes[offset + 1] = static_cast<UINT8>((value >> 8) & 0xffU);
}

static void put32(std::vector<UINT8>& bytes, UINT32 offset, UINT32 value) {
    bytes[offset] = static_cast<UINT8>(value & 0xffU);
    bytes[offset + 1] = static_cast<UINT8>((value >> 8) & 0xffU);
    bytes[offset + 2] = static_cast<UINT8>((value >> 16) & 0xffU);
    bytes[offset + 3] = static_cast<UINT8>((value >> 24) & 0xffU);
}

static bool write_elf(const char* path, const std::vector<UINT32>& program) {
    const UINT32 ehdr_size = 52;
    const UINT32 phdr_size = 32;
    const UINT32 segment_offset = 0x100;
    const UINT32 load_addr = 0x40;
    const UINT32 filesz = static_cast<UINT32>(program.size() * sizeof(UINT32));
    const UINT32 memsz = filesz + 4;

    std::vector<UINT8> elf(segment_offset + filesz, 0);
    elf[0] = 0x7f;
    elf[1] = 'E';
    elf[2] = 'L';
    elf[3] = 'F';
    elf[4] = 1; // ELFCLASS32
    elf[5] = 1; // little endian
    elf[6] = 1; // current ELF version

    put16(elf, 16, 2);       // ET_EXEC
    put16(elf, 18, 243);     // EM_RISCV
    put32(elf, 20, 1);       // EV_CURRENT
    put32(elf, 24, load_addr);
    put32(elf, 28, ehdr_size);
    put32(elf, 32, 0);
    put32(elf, 36, 0);
    put16(elf, 40, ehdr_size);
    put16(elf, 42, phdr_size);
    put16(elf, 44, 1);

    put32(elf, ehdr_size + 0, 1);              // PT_LOAD
    put32(elf, ehdr_size + 4, segment_offset);
    put32(elf, ehdr_size + 8, load_addr);
    put32(elf, ehdr_size + 12, load_addr);
    put32(elf, ehdr_size + 16, filesz);
    put32(elf, ehdr_size + 20, memsz);
    put32(elf, ehdr_size + 24, 0x7);           // R/W/X
    put32(elf, ehdr_size + 28, 4);

    for (UINT32 i = 0; i < program.size(); ++i) {
        put32(elf, segment_offset + i * sizeof(UINT32), program[i]);
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(reinterpret_cast<const char*>(elf.data()), static_cast<std::streamsize>(elf.size()));
    return static_cast<bool>(file);
}

int main() {
    const char* path = "test_loader_elf32_program.elf";
    const std::vector<UINT32> program = {
        encode_i(17, 0, 0x0, 1, 0x13), // addi x1, x0, 17
        encode_i(25, 0, 0x0, 2, 0x13), // addi x2, x0, 25
        encode_i(96, 0, 0x0, 3, 0x13), // addi x3, x0, 96
        encode_s(0, 2, 3, 0x2),        // sw x2, 0(x3)
        encode_i(0, 3, 0x2, 4, 0x03),  // lw x4, 0(x3)
        0x00000073U                    // ecall
    };
    assert(write_elf(path, program));

    Machine machine;
    UINT32 entry = 0;
    assert(ElfLoader::load(machine.bus(), path, entry));
    assert(entry == 0x40);

    machine.reset(entry);
    assert(machine.run(100));
    assert(machine.cpu().reg(1) == 17);
    assert(machine.cpu().reg(2) == 25);
    assert(machine.cpu().reg(4) == 25);

    INT32 value = 0;
    assert(machine.memory().read(24, value));
    assert(value == 25);
    assert(machine.memory().read(0x40 / sizeof(UINT32) + static_cast<UINT32>(program.size()), value));
    assert(value == 0);

    std::remove(path);
    std::cout << "test_loader_elf32: PASS\n";
    return 0;
}
