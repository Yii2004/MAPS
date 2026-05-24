#include "cpu/cpu.h"

namespace maps_sim {

    Cpu::Cpu(Bus* bus)
        : bus_(bus),
          regs_{},
          pc_(0),
          halted_(false),
          faulted_(false),
          ecall_pending_(false),
          mstatus_(0),
          mtvec_(0),
          mepc_(0),
          mcause_(0) {}

    void Cpu::bind_bus(Bus* bus) {
        bus_ = bus;
    }

    void Cpu::reset(UINT32 pc) {
        for (UINT32 i = 0; i < 32; ++i) {
            regs_[i] = 0;
        }
        pc_ = pc;
        halted_ = false;
        faulted_ = false;
        ecall_pending_ = false;
        mstatus_ = 0;
        mtvec_ = 0;
        mepc_ = 0;
        mcause_ = 0;
    }

    bool Cpu::step() {
        if (halted_ || faulted_ || ecall_pending_ || bus_ == nullptr || (pc_ % sizeof(UINT32)) != 0) {
            faulted_ = !halted_;
            return false;
        }

        UINT32 inst = 0;
        if (!bus_->read32(pc_, inst)) {
            faulted_ = true;
            return false;
        }

        UINT32 next_pc = pc_ + 4;
        if (!execute(inst, next_pc)) {
            faulted_ = true;
            return false;
        }

        regs_[0] = 0;
        pc_ = next_pc;
        return !faulted_;
    }

    bool Cpu::run(UINT32 max_steps) {
        for (UINT32 i = 0; i < max_steps && !halted_ && !faulted_; ++i) {
            if (!step()) {
                return false;
            }
            if (ecall_pending_) {
                halt();
            }
        }
        return halted_ && !faulted_;
    }

    UINT32 Cpu::pc() const {
        return pc_;
    }

    UINT32 Cpu::reg(UINT32 index) const {
        return (index < 32) ? regs_[index] : 0;
    }

    void Cpu::set_reg(UINT32 index, UINT32 value) {
        if (index != 0 && index < 32) {
            regs_[index] = value;
        }
    }

    void Cpu::halt() {
        halted_ = true;
        ecall_pending_ = false;
    }

    bool Cpu::halted() const {
        return halted_;
    }

    bool Cpu::faulted() const {
        return faulted_;
    }

    bool Cpu::ecall_pending() const {
        return ecall_pending_;
    }

    void Cpu::clear_ecall() {
        ecall_pending_ = false;
    }

    INT32 Cpu::sign_extend(UINT32 value, UINT32 bits) const {
        const UINT32 sign = 1U << (bits - 1);
        return static_cast<INT32>((value ^ sign) - sign);
    }

    UINT32 Cpu::imm_i(UINT32 inst) const {
        return static_cast<UINT32>(sign_extend(inst >> 20, 12));
    }

    UINT32 Cpu::imm_s(UINT32 inst) const {
        const UINT32 imm = ((inst >> 7) & 0x1fU) | (((inst >> 25) & 0x7fU) << 5);
        return static_cast<UINT32>(sign_extend(imm, 12));
    }

    UINT32 Cpu::imm_b(UINT32 inst) const {
        const UINT32 imm = (((inst >> 31) & 0x1U) << 12) |
                           (((inst >> 7) & 0x1U) << 11) |
                           (((inst >> 25) & 0x3fU) << 5) |
                           (((inst >> 8) & 0xfU) << 1);
        return static_cast<UINT32>(sign_extend(imm, 13));
    }

    UINT32 Cpu::imm_u(UINT32 inst) const {
        return inst & 0xfffff000U;
    }

    UINT32 Cpu::imm_j(UINT32 inst) const {
        const UINT32 imm = (((inst >> 31) & 0x1U) << 20) |
                           (((inst >> 12) & 0xffU) << 12) |
                           (((inst >> 20) & 0x1U) << 11) |
                           (((inst >> 21) & 0x3ffU) << 1);
        return static_cast<UINT32>(sign_extend(imm, 21));
    }

    bool Cpu::read_csr(UINT32 addr, UINT32& value) const {
        switch (addr) {
            case 0x300:
                value = mstatus_;
                return true;
            case 0x305:
                value = mtvec_;
                return true;
            case 0x341:
                value = mepc_;
                return true;
            case 0x342:
                value = mcause_;
                return true;
            default:
                return false;
        }
    }

    bool Cpu::write_csr(UINT32 addr, UINT32 value) {
        switch (addr) {
            case 0x300:
                mstatus_ = value;
                return true;
            case 0x305:
                mtvec_ = value & ~0x3U;
                return true;
            case 0x341:
                mepc_ = value & ~0x3U;
                return true;
            case 0x342:
                mcause_ = value;
                return true;
            default:
                return false;
        }
    }

    bool Cpu::load(UINT32 funct3, UINT32 rd, UINT32 addr) {
        switch (funct3) {
            case 0x0: {
                UINT8 value = 0;
                if (!bus_->read8(addr, value)) {
                    return false;
                }
                regs_[rd] = static_cast<UINT32>(sign_extend(value, 8));
                return true;
            }
            case 0x1: {
                UINT16 value = 0;
                if (!bus_->read16(addr, value)) {
                    return false;
                }
                regs_[rd] = static_cast<UINT32>(sign_extend(value, 16));
                return true;
            }
            case 0x2: {
                UINT32 value = 0;
                if (!bus_->read32(addr, value)) {
                    return false;
                }
                regs_[rd] = value;
                return true;
            }
            case 0x4: {
                UINT8 value = 0;
                if (!bus_->read8(addr, value)) {
                    return false;
                }
                regs_[rd] = value;
                return true;
            }
            case 0x5: {
                UINT16 value = 0;
                if (!bus_->read16(addr, value)) {
                    return false;
                }
                regs_[rd] = value;
                return true;
            }
            default:
                return false;
        }
    }

    bool Cpu::store(UINT32 funct3, UINT32 addr, UINT32 value) {
        switch (funct3) {
            case 0x0:
                return bus_->write8(addr, static_cast<UINT8>(value & 0xffU));
            case 0x1:
                return bus_->write16(addr, static_cast<UINT16>(value & 0xffffU));
            case 0x2:
                return bus_->write32(addr, value);
            default:
                return false;
        }
    }

    bool Cpu::execute(UINT32 inst, UINT32& next_pc) {
        const UINT32 opcode = inst & 0x7fU;
        const UINT32 rd = (inst >> 7) & 0x1fU;
        const UINT32 funct3 = (inst >> 12) & 0x7U;
        const UINT32 rs1 = (inst >> 15) & 0x1fU;
        const UINT32 rs2 = (inst >> 20) & 0x1fU;
        const UINT32 funct7 = (inst >> 25) & 0x7fU;

        switch (opcode) {
            case 0x37:
                regs_[rd] = imm_u(inst);
                return true;

            case 0x17:
                regs_[rd] = pc_ + imm_u(inst);
                return true;

            case 0x6f:
                regs_[rd] = pc_ + 4;
                next_pc = pc_ + imm_j(inst);
                return (next_pc % sizeof(UINT32)) == 0;

            case 0x67:
                if (funct3 != 0) {
                    return false;
                }
                next_pc = (regs_[rs1] + imm_i(inst)) & ~1U;
                regs_[rd] = pc_ + 4;
                return (next_pc % sizeof(UINT32)) == 0;

            case 0x63: {
                bool taken = false;
                switch (funct3) {
                    case 0x0:
                        taken = regs_[rs1] == regs_[rs2];
                        break;
                    case 0x1:
                        taken = regs_[rs1] != regs_[rs2];
                        break;
                    case 0x4:
                        taken = static_cast<INT32>(regs_[rs1]) < static_cast<INT32>(regs_[rs2]);
                        break;
                    case 0x5:
                        taken = static_cast<INT32>(regs_[rs1]) >= static_cast<INT32>(regs_[rs2]);
                        break;
                    case 0x6:
                        taken = regs_[rs1] < regs_[rs2];
                        break;
                    case 0x7:
                        taken = regs_[rs1] >= regs_[rs2];
                        break;
                    default:
                        return false;
                }
                if (taken) {
                    next_pc = pc_ + imm_b(inst);
                    return (next_pc % sizeof(UINT32)) == 0;
                }
                return true;
            }

            case 0x03:
                return load(funct3, rd, regs_[rs1] + imm_i(inst));

            case 0x23:
                return store(funct3, regs_[rs1] + imm_s(inst), regs_[rs2]);

            case 0x13:
                switch (funct3) {
                    case 0x0:
                        regs_[rd] = regs_[rs1] + imm_i(inst);
                        return true;
                    case 0x2:
                        regs_[rd] = (static_cast<INT32>(regs_[rs1]) < static_cast<INT32>(imm_i(inst))) ? 1 : 0;
                        return true;
                    case 0x3:
                        regs_[rd] = (regs_[rs1] < imm_i(inst)) ? 1 : 0;
                        return true;
                    case 0x4:
                        regs_[rd] = regs_[rs1] ^ imm_i(inst);
                        return true;
                    case 0x6:
                        regs_[rd] = regs_[rs1] | imm_i(inst);
                        return true;
                    case 0x7:
                        regs_[rd] = regs_[rs1] & imm_i(inst);
                        return true;
                    case 0x1:
                        if (funct7 != 0x00) {
                            return false;
                        }
                        regs_[rd] = regs_[rs1] << rs2;
                        return true;
                    case 0x5:
                        if (funct7 == 0x00) {
                            regs_[rd] = regs_[rs1] >> rs2;
                            return true;
                        }
                        if (funct7 == 0x20) {
                            regs_[rd] = static_cast<UINT32>(static_cast<INT32>(regs_[rs1]) >> rs2);
                            return true;
                        }
                        return false;
                    default:
                        return false;
                }

            case 0x33:
                switch (funct3) {
                    case 0x0:
                        if (funct7 == 0x00) {
                            regs_[rd] = regs_[rs1] + regs_[rs2];
                            return true;
                        }
                        if (funct7 == 0x20) {
                            regs_[rd] = regs_[rs1] - regs_[rs2];
                            return true;
                        }
                        return false;
                    case 0x1:
                        if (funct7 != 0x00) {
                            return false;
                        }
                        regs_[rd] = regs_[rs1] << (regs_[rs2] & 0x1fU);
                        return true;
                    case 0x2:
                        if (funct7 != 0x00) {
                            return false;
                        }
                        regs_[rd] = (static_cast<INT32>(regs_[rs1]) < static_cast<INT32>(regs_[rs2])) ? 1 : 0;
                        return true;
                    case 0x3:
                        if (funct7 != 0x00) {
                            return false;
                        }
                        regs_[rd] = (regs_[rs1] < regs_[rs2]) ? 1 : 0;
                        return true;
                    case 0x4:
                        if (funct7 != 0x00) {
                            return false;
                        }
                        regs_[rd] = regs_[rs1] ^ regs_[rs2];
                        return true;
                    case 0x5:
                        if (funct7 == 0x00) {
                            regs_[rd] = regs_[rs1] >> (regs_[rs2] & 0x1fU);
                            return true;
                        }
                        if (funct7 == 0x20) {
                            regs_[rd] = static_cast<UINT32>(static_cast<INT32>(regs_[rs1]) >> (regs_[rs2] & 0x1fU));
                            return true;
                        }
                        return false;
                    case 0x6:
                        if (funct7 != 0x00) {
                            return false;
                        }
                        regs_[rd] = regs_[rs1] | regs_[rs2];
                        return true;
                    case 0x7:
                        if (funct7 != 0x00) {
                            return false;
                        }
                        regs_[rd] = regs_[rs1] & regs_[rs2];
                        return true;
                    default:
                        return false;
                }

            case 0x73:
                if (inst == 0x00000073U) {
                    if (mtvec_ != 0) {
                        mepc_ = pc_ + 4;
                        mcause_ = 11;
                        next_pc = mtvec_;
                        return true;
                    }
                    ecall_pending_ = true;
                    return true;
                }
                if (inst == 0x30200073U) {
                    next_pc = mepc_;
                    return (next_pc % sizeof(UINT32)) == 0;
                }
                if (funct3 >= 1 && funct3 <= 7) {
                    const UINT32 csr_addr = inst >> 20;
                    const UINT32 csr_operand = (funct3 >= 5) ? rs1 : regs_[rs1];
                    UINT32 old_value = 0;
                    if (!read_csr(csr_addr, old_value)) {
                        return false;
                    }

                    UINT32 new_value = old_value;
                    switch (funct3) {
                        case 0x1:
                        case 0x5:
                            new_value = csr_operand;
                            break;
                        case 0x2:
                        case 0x6:
                            new_value = old_value | csr_operand;
                            break;
                        case 0x3:
                        case 0x7:
                            new_value = old_value & ~csr_operand;
                            break;
                        default:
                            return false;
                    }
                    if (rd != 0) {
                        regs_[rd] = old_value;
                    }
                    if (((funct3 == 0x2 || funct3 == 0x3) && rs1 == 0) ||
                        ((funct3 == 0x6 || funct3 == 0x7) && rs1 == 0)) {
                        return true;
                    }
                    return write_csr(csr_addr, new_value);
                }
                return false;

            default:
                return false;
        }
    }
}
