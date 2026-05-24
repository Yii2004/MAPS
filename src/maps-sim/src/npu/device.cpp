#include "npu/device.h"

namespace maps_sim {

    NpuDevice::NpuDevice()
        : controller_(),
          memory_(nullptr),
          desc_addr_(0) {}

    void NpuDevice::bind_memory(Memory* memory) {
        memory_ = memory;
        controller_.bind_memory(memory);
    }

    void NpuDevice::reset() {
        desc_addr_ = 0;
        controller_.reset();
        if (memory_ != nullptr) {
            controller_.bind_memory(memory_);
        }
    }

    bool NpuDevice::write32(UINT32 offset, UINT32 value) {
        switch (offset) {
            case npu_mmio::DESC_ADDR:
                desc_addr_ = value;
                return true;
            case npu_mmio::CMD:
                if (value == npu_mmio::CMD_START) {
                    return start();
                }
                if (value == npu_mmio::CMD_RESET) {
                    reset();
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    bool NpuDevice::read32(UINT32 offset, UINT32& value) const {
        const PerfCounters perf = controller_.perf();

        switch (offset) {
            case npu_mmio::DESC_ADDR:
                value = desc_addr_;
                return true;
            case npu_mmio::STATUS:
                value = static_cast<UINT32>(controller_.state());
                return true;
            case npu_mmio::ERROR:
                value = static_cast<UINT32>(controller_.error());
                return true;
            case npu_mmio::CYCLE_COUNT_LO:
                value = perf_lo(perf.cycle_count);
                return true;
            case npu_mmio::CYCLE_COUNT_HI:
                value = perf_hi(perf.cycle_count);
                return true;
            case npu_mmio::DMA_READ_COUNT_LO:
                value = perf_lo(perf.dma_read_count);
                return true;
            case npu_mmio::DMA_READ_COUNT_HI:
                value = perf_hi(perf.dma_read_count);
                return true;
            case npu_mmio::DMA_WRITE_COUNT_LO:
                value = perf_lo(perf.dma_write_count);
                return true;
            case npu_mmio::DMA_WRITE_COUNT_HI:
                value = perf_hi(perf.dma_write_count);
                return true;
            case npu_mmio::MAC_COUNT_LO:
                value = perf_lo(perf.mac_count);
                return true;
            case npu_mmio::MAC_COUNT_HI:
                value = perf_hi(perf.mac_count);
                return true;
            default:
                return false;
        }
    }

    Controller& NpuDevice::controller() {
        return controller_;
    }

    const Controller& NpuDevice::controller() const {
        return controller_;
    }

    bool NpuDevice::start() {
        if (memory_ == nullptr) {
            return false;
        }

        Descriptor desc;
        if (!desc.load_from_memory(*memory_, desc_addr_)) {
            return controller_.submit(desc);
        }
        return controller_.submit(desc) && controller_.run();
    }

    UINT32 NpuDevice::perf_lo(UINT64 value) const {
        return static_cast<UINT32>(value & 0xffffffffULL);
    }

    UINT32 NpuDevice::perf_hi(UINT64 value) const {
        return static_cast<UINT32>(value >> 32);
    }
}
