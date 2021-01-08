#include "assembler.hxx"

#include <stdexcept>

namespace assembly {
    static auto register_width(mnemo_t::arg_t::reg_t reg) -> mnemo_t::width_t {
        switch (reg) {
            case mnemo_t::arg_t::reg_t::Al:
            case mnemo_t::arg_t::reg_t::Bl:
            case mnemo_t::arg_t::reg_t::Cl:
            case mnemo_t::arg_t::reg_t::Dl:
            case mnemo_t::arg_t::reg_t::Ah:
            case mnemo_t::arg_t::reg_t::Bh:
            case mnemo_t::arg_t::reg_t::Ch:
            case mnemo_t::arg_t::reg_t::Dh:
                return mnemo_t::width_t::Byte;
            case mnemo_t::arg_t::reg_t::Ax:
            case mnemo_t::arg_t::reg_t::Bx:
            case mnemo_t::arg_t::reg_t::Cx:
            case mnemo_t::arg_t::reg_t::Dx:
                return mnemo_t::width_t::Word;
            case mnemo_t::arg_t::reg_t::Eax:
            case mnemo_t::arg_t::reg_t::Ebx:
            case mnemo_t::arg_t::reg_t::Ecx:
            case mnemo_t::arg_t::reg_t::Edx:
                return mnemo_t::width_t::Dword;
            case mnemo_t::arg_t::reg_t::Rax:
            case mnemo_t::arg_t::reg_t::Rbx:
            case mnemo_t::arg_t::reg_t::Rcx:
            case mnemo_t::arg_t::reg_t::Rdx:
                return mnemo_t::width_t::Qword;
            default:
                throw std::logic_error("Unsupported register! register_width");
        }
    }

    // Converts reg_t to number usable in ModR/M and reg fields of instruction encoding
    static auto reg_to_number(mnemo_t::arg_t::reg_t reg) -> u8 {
        switch (reg) {
            case mnemo_t::arg_t::reg_t::Al:
            case mnemo_t::arg_t::reg_t::Ax:
            case mnemo_t::arg_t::reg_t::Eax:
            case mnemo_t::arg_t::reg_t::Rax:
                return 0b000;
            case mnemo_t::arg_t::reg_t::Cl:
            case mnemo_t::arg_t::reg_t::Cx:
            case mnemo_t::arg_t::reg_t::Ecx:
            case mnemo_t::arg_t::reg_t::Rcx:
                return 0b001;
            case mnemo_t::arg_t::reg_t::Dl:
            case mnemo_t::arg_t::reg_t::Dx:
            case mnemo_t::arg_t::reg_t::Edx:
            case mnemo_t::arg_t::reg_t::Rdx:
                return 0b010;
            case mnemo_t::arg_t::reg_t::Bl:
            case mnemo_t::arg_t::reg_t::Bx:
            case mnemo_t::arg_t::reg_t::Ebx:
            case mnemo_t::arg_t::reg_t::Rbx:
                return 0b011;
            case mnemo_t::arg_t::reg_t::Ah:
                return 0b100;
            case mnemo_t::arg_t::reg_t::Ch:
                return 0b101;
            case mnemo_t::arg_t::reg_t::Dh:
                return 0b110;
            case mnemo_t::arg_t::reg_t::Bh:
                return 0b111;
            default:
                throw std::logic_error("Unsupported register!");
        }
    }

    static auto mod_and_reg_and_rm_to_byte(u8 mod, u8 reg, u8 rm) -> u8 {
        return (mod << 6) | (reg << 3) | (rm << 0);
    }

    // Put a operand-size override prefix if instruction width = word
    static auto push_OSOR_if_word(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.width == mnemo_t::width_t::Word) {
            out.push_back(0x66);
        }
    }

    // Put a REX prefix if instruction width = qword
    static auto push_rex_if_qword(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.width == mnemo_t::width_t::Qword) {
            out.push_back(0b01001000);
        }
    }

    static auto assemble_mnemo_mov(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.tag != mnemo_t::tag_t::Mov)
            throw std::logic_error("Wrong mnemo!");

        if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
            mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
            u8 opcode;
            switch (mnemo.width) {
                case mnemo_t::width_t::Byte:
                    // mov r/m8, r8
                    opcode = 0x88;
                    break;
                case mnemo_t::width_t::Word:
                case mnemo_t::width_t::Dword:
                case mnemo_t::width_t::Qword:
                    // mov r/m32, r32
                    opcode = 0x89;
                    break;
                default:
                    throw std::logic_error("Unsupported width!");
            }
            u8 mod = 0b11;
            u8 rm = reg_to_number(mnemo.a1.data.reg);
            u8 reg = reg_to_number(mnemo.a2.data.reg);

            push_OSOR_if_word(out, mnemo);
            push_rex_if_qword(out, mnemo);
            out.push_back(opcode);
            out.push_back(mod_and_reg_and_rm_to_byte(mod, reg, rm));
        } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
                   mnemo.a2.tag == mnemo_t::arg_t::tag_t::Immediate) {
            u8 opcode;
            switch (mnemo.width) {
                case mnemo_t::width_t::Byte:
                    // mov r8, imm8
                    opcode = 0xb0;
                    break;
                case mnemo_t::width_t::Word:
                case mnemo_t::width_t::Dword:
                case mnemo_t::width_t::Qword:
                    // mov r32, imm32
                    opcode = 0xb8;
                    break;
                default:
                    throw std::logic_error("Unsupported width!");
            }
            opcode += reg_to_number(mnemo.a1.data.reg);

            push_OSOR_if_word(out, mnemo);
            push_rex_if_qword(out, mnemo);
            out.push_back(opcode);

            // Write imm
            switch (mnemo.width) {
                case mnemo_t::width_t::Byte: {
                    // Write i8
                    out.push_back(mnemo.a2.data.imm);
                    break;
                }
                case mnemo_t::width_t::Word: {
                    // Write LE i16
                    i16 imm = mnemo.a2.data.imm;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    break;
                }
                case mnemo_t::width_t::Dword: {
                    // Write LE i32
                    i32 imm = mnemo.a2.data.imm;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    break;
                }
                case mnemo_t::width_t::Qword: {
                    // Write LE i64
                    i64 imm = mnemo.a2.data.imm;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    break;
                }
                default:
                    throw std::logic_error("Unsupported width! 1");
            }
        } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory &&
                   mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
            // TODO: implement mov to memory and from memory
            throw std::exception();
        } else {
            throw std::logic_error("Unsupported mov shape!");
        }
    }

    auto assemble_mnemo(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.width != mnemo_t::width_t::Byte && mnemo.width != mnemo_t::width_t::Word &&
            mnemo.width != mnemo_t::width_t::Dword && mnemo.width != mnemo_t::width_t::Qword &&
            mnemo.width != mnemo_t::width_t::NotSet)
            throw std::logic_error("Unsupported width! assemble_mnemo");
        if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register && register_width(mnemo.a1.data.reg) != mnemo.width) {
            throw std::logic_error("arg1 register width does not match instruction width!");
        }
        if (mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register && register_width(mnemo.a2.data.reg) != mnemo.width) {
            throw std::logic_error("arg2 register width does not match instruction width!");
        }

        switch (mnemo.tag) {
            case mnemo_t::tag_t::Mov: {
                assemble_mnemo_mov(out, mnemo);
                break;
            }
            case mnemo_t::tag_t::Ret: {
                out.push_back(0xc3);
                break;
            }
            default:
                throw std::logic_error("Unimplemented mnemo.tag!");
        }
    }

    auto assemble(const vector<mnemo_t> &mnemos) -> vector<u8> {
        vector<u8> result{};
        for (const mnemo_t &mnemo:mnemos) {
            assemble_mnemo(result, mnemo);
        }
        return result;
    }
}