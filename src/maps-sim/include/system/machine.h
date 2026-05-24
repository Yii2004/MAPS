#ifndef MAPS_SIM_SYSTEM_MACHINE_H
#define MAPS_SIM_SYSTEM_MACHINE_H

#include <string>

#include "common/bus.h"
#include "common/memory.h"
#include "cpu/cpu.h"
#include "npu/device.h"

namespace maps_sim {

    class Machine {
        public:
            Machine();

            void reset(UINT32 pc);
            bool run(UINT32 max_steps);

            Memory& memory();
            const Memory& memory() const;
            Bus& bus();
            const Bus& bus() const;
            Cpu& cpu();
            const Cpu& cpu() const;
            NpuDevice& npu();
            const NpuDevice& npu() const;
            const std::string& console_output() const;
            INT32 exit_code() const;

        private:
            bool handle_syscall();

            Memory memory_;
            Bus bus_;
            Cpu cpu_;
            NpuDevice npu_;
            std::string console_output_;
            INT32 exit_code_;
    };
}

#endif // MAPS_SIM_SYSTEM_MACHINE_H
