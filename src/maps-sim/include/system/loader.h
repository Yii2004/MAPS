#ifndef MAPS_SIM_SYSTEM_LOADER_H
#define MAPS_SIM_SYSTEM_LOADER_H

#include "common/bus.h"
#include "common/memory.h"

namespace maps_sim {

    class BinaryLoader {
        public:
            static bool load_words(Memory& memory,
                                   UINT32 base_word,
                                   const UINT32* words,
                                   UINT32 count);

            static bool load_bytes(Bus& bus,
                                   UINT32 base_addr,
                                   const UINT8* bytes,
                                   UINT32 count);

            static bool load_file(Bus& bus,
                                  UINT32 base_addr,
                                  const char* path);
    };

    class ElfLoader {
        public:
            static bool load(Bus& bus,
                             const char* path,
                             UINT32& entry);
    };
}

#endif // MAPS_SIM_SYSTEM_LOADER_H
