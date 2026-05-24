#include "system/machine.h"

#include "runtime/syscall.h"

namespace maps_sim {
    namespace {
        constexpr UINT32 REG_A0 = 10;
        constexpr UINT32 REG_A1 = 11;
        constexpr UINT32 REG_A7 = 17;
    }

    Machine::Machine()
        : memory_(),
          bus_(),
          cpu_(&bus_),
          npu_(),
          console_output_(),
          exit_code_(0) {
        npu_.bind_memory(&memory_);
        bus_.bind_memory(&memory_);
        bus_.bind_npu(&npu_);
        bus_.bind_console(&console_output_);
    }

    void Machine::reset(UINT32 pc) {
        npu_.reset();
        npu_.bind_memory(&memory_);
        bus_.bind_memory(&memory_);
        bus_.bind_npu(&npu_);
        bus_.bind_console(&console_output_);
        cpu_.bind_bus(&bus_);
        cpu_.reset(pc);
        console_output_.clear();
        exit_code_ = 0;
    }

    bool Machine::run(UINT32 max_steps) {
        for (UINT32 i = 0; i < max_steps && !cpu_.halted() && !cpu_.faulted(); ++i) {
            if (!cpu_.step()) {
                return false;
            }
            if (cpu_.ecall_pending() && !handle_syscall()) {
                return false;
            }
        }
        return cpu_.halted() && !cpu_.faulted();
    }

    Memory& Machine::memory() {
        return memory_;
    }

    const Memory& Machine::memory() const {
        return memory_;
    }

    Bus& Machine::bus() {
        return bus_;
    }

    const Bus& Machine::bus() const {
        return bus_;
    }

    Cpu& Machine::cpu() {
        return cpu_;
    }

    const Cpu& Machine::cpu() const {
        return cpu_;
    }

    NpuDevice& Machine::npu() {
        return npu_;
    }

    const NpuDevice& Machine::npu() const {
        return npu_;
    }

    const std::string& Machine::console_output() const {
        return console_output_;
    }

    INT32 Machine::exit_code() const {
        return exit_code_;
    }

    bool Machine::handle_syscall() {
        const UINT32 number = cpu_.reg(REG_A7);
        const UINT32 arg0 = cpu_.reg(REG_A0);
        const UINT32 arg1 = cpu_.reg(REG_A1);

        switch (number) {
            case MAPS_SYSCALL_EXIT:
                exit_code_ = static_cast<INT32>(arg0);
                cpu_.halt();
                return true;

            case MAPS_SYSCALL_PUTCHAR:
                console_output_.push_back(static_cast<char>(arg0 & 0xffU));
                cpu_.set_reg(REG_A0, 1);
                cpu_.clear_ecall();
                return true;

            case MAPS_SYSCALL_WRITE:
                for (UINT32 i = 0; i < arg1; ++i) {
                    UINT8 ch = 0;
                    if (!bus_.read8(arg0 + i, ch)) {
                        cpu_.set_reg(REG_A0, 0xffffffffU);
                        cpu_.clear_ecall();
                        return true;
                    }
                    console_output_.push_back(static_cast<char>(ch));
                }
                cpu_.set_reg(REG_A0, arg1);
                cpu_.clear_ecall();
                return true;

            default:
                cpu_.set_reg(REG_A0, 0xffffffffU);
                cpu_.clear_ecall();
                return true;
        }
    }
}
