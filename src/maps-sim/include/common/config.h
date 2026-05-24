#ifndef MAPS_SIM_COMMON_CONFIG_H
#define MAPS_SIM_COMMON_CONFIG_H

#include "common/types.h"

namespace maps_sim {

    enum class dataflow {
        WS, // Weight Stationary
        OS, // Output Stationary
        IS  // Input Stationary
    };

    namespace config {
        inline constexpr UINT32 ARRAY_ROWS = 16;
        inline constexpr UINT32 ARRAY_COLS = 16;

        inline constexpr UINT32 MATRIX_ROWS = 16;
        inline constexpr UINT32 MATRIX_COLS = 16;

        inline constexpr UINT32 BUFFER_SIZES = 2048;
        // Number of INT32 elements for a 4MB DRAM model.
        inline constexpr UINT32 DRAM_SIZES = (4 * 1024 * 1024) / sizeof(INT32);

        inline constexpr UINT32 TILE_SIZES = 16; 
    }

} // namespace maps_sim


#endif // MAPS_SIM_COMMON_CONFIG_H
