#ifndef MAPS_SIM_CPU_CPU_H
#define MAPS_SIM_CPU_CPU_H

#include "common/bus.h"

namespace maps_sim {

    class Cpu {
        public:
            explicit Cpu(Bus* bus = nullptr);

            void bind_bus(Bus* bus);
            void reset(UINT32 pc);

            bool step();
            bool run(UINT32 max_steps);

            UINT32 pc() const;
            UINT32 reg(UINT32 index) const;
            void set_reg(UINT32 index, UINT32 value);
            void halt();
            bool halted() const;
            bool faulted() const;
            bool ecall_pending() const;
            void clear_ecall();

        private:
            INT32 sign_extend(UINT32 value, UINT32 bits) const;

            UINT32 imm_i(UINT32 inst) const;
            UINT32 imm_s(UINT32 inst) const;
            UINT32 imm_b(UINT32 inst) const;
            UINT32 imm_u(UINT32 inst) const;
            UINT32 imm_j(UINT32 inst) const;

            bool read_csr(UINT32 addr, UINT32& value) const;
            bool write_csr(UINT32 addr, UINT32 value);
            bool load(UINT32 funct3, UINT32 rd, UINT32 addr);
            bool store(UINT32 funct3, UINT32 addr, UINT32 value);
            bool execute(UINT32 inst, UINT32& next_pc);

            Bus* bus_;
            UINT32 regs_[32];
            UINT32 pc_;
            bool halted_;
            bool faulted_;
            bool ecall_pending_;
            UINT32 mstatus_;
            UINT32 mtvec_;
            UINT32 mepc_;
            UINT32 mcause_;
    };
}

#endif // MAPS_SIM_CPU_CPU_H
