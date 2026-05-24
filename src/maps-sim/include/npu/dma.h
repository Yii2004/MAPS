#ifndef MAPS_SIM_NPU_DMA_H
#define MAPS_SIM_NPU_DMA_H

#include "common/types.h"
#include "common/memory.h"
#include "npu/buffer.h"

namespace maps_sim {
    enum class DmaState {
        IDLE,
        BUSY,
        DONE,
        ERROR
    };


    enum class DmaError {
        NONE,
        INVALID_PARAM,
        OUT_OF_RANGE,
        NULL_PTR,
        NOT_READY
    };

    enum class DmaDirection {
        DRAM_TO_BUFFER,
        BUFFER_TO_DRAM
    };

    struct DmaStats {
        UINT64 bytes_moved = 0;
        UINT64 transfer_count = 0;
        UINT64 cycle_count = 0;
    };

    struct DmaRequest {
        DmaDirection direction;
        UINT32 dram_base;   // byte address, must be INT32-aligned
        UINT32 buffer_base;
        UINT32 length;      // elements per row
        UINT32 rows;        // number of rows
        UINT32 src_stride;  // source row stride (elements)
        UINT32 dst_stride;  // destination row stride (elements)
    };

    class DmaEngine {

        public:
            DmaEngine();

            void bind_memory(Memory* mem);
            void bind_buffer(Buffer* buf);

            bool submit(const DmaRequest& req);

            bool run_sync();

            DmaState state() const ;
            DmaError error() const ;
            DmaStats stats() const ;
            bool busy() const ;
            bool done() const ;

            void reset() ;

        private:
            Buffer* buffer_;
            Memory* memory_;
            UINT32 dram_size_;

            DmaRequest current_req_;
            bool has_request_;

            DmaState state_;
            DmaError error_;
            DmaStats stats_;

    };




}


#endif
