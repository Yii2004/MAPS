#include "system/loader.h"

#include <fstream>
#include <vector>

namespace maps_sim {
    namespace {
        constexpr UINT32 EI_CLASS = 4;
        constexpr UINT32 EI_DATA = 5;
        constexpr UINT32 ELFCLASS32 = 1;
        constexpr UINT32 ELFDATA2LSB = 1;
        constexpr UINT32 ET_EXEC = 2;
        constexpr UINT32 EM_RISCV = 243;
        constexpr UINT32 PT_LOAD = 1;

        UINT16 read_le16(const std::vector<UINT8>& bytes, UINT32 offset) {
            return static_cast<UINT16>(bytes[offset] |
                                       (static_cast<UINT16>(bytes[offset + 1]) << 8));
        }

        UINT32 read_le32(const std::vector<UINT8>& bytes, UINT32 offset) {
            return static_cast<UINT32>(bytes[offset]) |
                   (static_cast<UINT32>(bytes[offset + 1]) << 8) |
                   (static_cast<UINT32>(bytes[offset + 2]) << 16) |
                   (static_cast<UINT32>(bytes[offset + 3]) << 24);
        }

        bool range_ok(UINT32 offset, UINT32 size, UINT32 total) {
            return offset <= total && size <= (total - offset);
        }
    }

    bool BinaryLoader::load_words(Memory& memory,
                                  UINT32 base_word,
                                  const UINT32* words,
                                  UINT32 count) {
        if (words == nullptr) {
            return false;
        }
        for (UINT32 i = 0; i < count; ++i) {
            if (!memory.write(base_word + i, static_cast<INT32>(words[i]))) {
                return false;
            }
        }
        return true;
    }

    bool BinaryLoader::load_bytes(Bus& bus,
                                  UINT32 base_addr,
                                  const UINT8* bytes,
                                  UINT32 count) {
        if (bytes == nullptr) {
            return false;
        }
        for (UINT32 i = 0; i < count; ++i) {
            if (!bus.write8(base_addr + i, bytes[i])) {
                return false;
            }
        }
        return true;
    }

    bool BinaryLoader::load_file(Bus& bus,
                                 UINT32 base_addr,
                                 const char* path) {
        if (path == nullptr) {
            return false;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }

        std::vector<UINT8> bytes((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
        if (bytes.empty()) {
            return true;
        }
        return load_bytes(bus, base_addr, bytes.data(), static_cast<UINT32>(bytes.size()));
    }

    bool ElfLoader::load(Bus& bus,
                         const char* path,
                         UINT32& entry) {
        entry = 0;
        if (path == nullptr) {
            return false;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }

        std::vector<UINT8> bytes((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
        const UINT32 size = static_cast<UINT32>(bytes.size());
        if (size < 52) {
            return false;
        }

        if (bytes[0] != 0x7f || bytes[1] != 'E' || bytes[2] != 'L' || bytes[3] != 'F') {
            return false;
        }
        if (bytes[EI_CLASS] != ELFCLASS32 || bytes[EI_DATA] != ELFDATA2LSB) {
            return false;
        }

        const UINT16 e_type = read_le16(bytes, 16);
        const UINT16 e_machine = read_le16(bytes, 18);
        if (e_type != ET_EXEC || e_machine != EM_RISCV) {
            return false;
        }

        entry = read_le32(bytes, 24);
        const UINT32 phoff = read_le32(bytes, 28);
        const UINT16 phentsize = read_le16(bytes, 42);
        const UINT16 phnum = read_le16(bytes, 44);
        if (phentsize < 32) {
            return false;
        }
        if (!range_ok(phoff, static_cast<UINT32>(phentsize) * phnum, size)) {
            return false;
        }

        for (UINT32 i = 0; i < phnum; ++i) {
            const UINT32 ph = phoff + i * phentsize;
            const UINT32 type = read_le32(bytes, ph + 0);
            if (type != PT_LOAD) {
                continue;
            }

            const UINT32 offset = read_le32(bytes, ph + 4);
            const UINT32 vaddr = read_le32(bytes, ph + 8);
            const UINT32 paddr = read_le32(bytes, ph + 12);
            const UINT32 filesz = read_le32(bytes, ph + 16);
            const UINT32 memsz = read_le32(bytes, ph + 20);
            const UINT32 load_addr = (paddr != 0) ? paddr : vaddr;

            if (filesz > memsz || !range_ok(offset, filesz, size)) {
                return false;
            }
            if (filesz != 0 && !BinaryLoader::load_bytes(bus, load_addr, bytes.data() + offset, filesz)) {
                return false;
            }
            for (UINT32 zero = filesz; zero < memsz; ++zero) {
                if (!bus.write8(load_addr + zero, 0)) {
                    return false;
                }
            }
        }

        return true;
    }
}
