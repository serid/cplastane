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
            case mnemo_t::arg_t::reg_t::Sp:
            case mnemo_t::arg_t::reg_t::Bp:
            case mnemo_t::arg_t::reg_t::Si:
            case mnemo_t::arg_t::reg_t::Di:
                return mnemo_t::width_t::Word;
            case mnemo_t::arg_t::reg_t::Eax:
            case mnemo_t::arg_t::reg_t::Ebx:
            case mnemo_t::arg_t::reg_t::Ecx:
            case mnemo_t::arg_t::reg_t::Edx:
            case mnemo_t::arg_t::reg_t::Esp:
            case mnemo_t::arg_t::reg_t::Ebp:
            case mnemo_t::arg_t::reg_t::Esi:
            case mnemo_t::arg_t::reg_t::Edi:
                return mnemo_t::width_t::Dword;
            case mnemo_t::arg_t::reg_t::Rax:
            case mnemo_t::arg_t::reg_t::Rbx:
            case mnemo_t::arg_t::reg_t::Rcx:
            case mnemo_t::arg_t::reg_t::Rdx:
            case mnemo_t::arg_t::reg_t::Rsp:
            case mnemo_t::arg_t::reg_t::Rbp:
            case mnemo_t::arg_t::reg_t::Rsi:
            case mnemo_t::arg_t::reg_t::Rdi:
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
            case mnemo_t::arg_t::reg_t::Sp:
            case mnemo_t::arg_t::reg_t::Esp:
            case mnemo_t::arg_t::reg_t::Rsp:
                return 0b100;
            case mnemo_t::arg_t::reg_t::Ch:
            case mnemo_t::arg_t::reg_t::Bp:
            case mnemo_t::arg_t::reg_t::Ebp:
            case mnemo_t::arg_t::reg_t::Rbp:
                return 0b101;
            case mnemo_t::arg_t::reg_t::Dh:
            case mnemo_t::arg_t::reg_t::Si:
            case mnemo_t::arg_t::reg_t::Esi:
            case mnemo_t::arg_t::reg_t::Rsi:
                return 0b110;
            case mnemo_t::arg_t::reg_t::Bh:
            case mnemo_t::arg_t::reg_t::Di:
            case mnemo_t::arg_t::reg_t::Edi:
            case mnemo_t::arg_t::reg_t::Rdi:
                return 0b111;
            default:
                throw std::logic_error("Unsupported register!");
        }
    }

    // scale_t::S0 should be handled outside this function since it is not encoded with SS with but rather with Index bits equal to 0b100
    static auto scale_to_byte(mnemo_t::arg_t::memory_t::scale_t scale) -> u8 {
        switch (scale) {
            case mnemo_t::arg_t::memory_t::scale_t::S0:
                throw std::logic_error("Unexpected scale_t::S0 @ scale_to_byte");
            case mnemo_t::arg_t::memory_t::scale_t::S1:
                return 0b00;
            case mnemo_t::arg_t::memory_t::scale_t::S2:
                return 0b01;
            case mnemo_t::arg_t::memory_t::scale_t::S4:
                return 0b10;
            case mnemo_t::arg_t::memory_t::scale_t::S8:
                return 0b11;
            default:
                throw std::logic_error("Scale is Undef.");
        }
    }

    static auto mod_and_reg_and_rm_to_modrm(u8 mod, u8 reg, u8 rm) -> u8 {
        return (mod << 6) | (reg << 3) | (rm << 0);
    }

    static auto scale_and_index_and_base_to_sib(u8 scale, u8 index, u8 base) -> u8 {
        return (scale << 6) | (index << 3) | (base << 0);
    }

    // Put an "address-size override" prefix if address width = dword
    // In 64 bit mode switches address width from 64 bits to 32 bits
    static auto push_ASOR_if_dword(vector<u8> &out, const mnemo_t::arg_t::memory_t &memory_field) -> void {
        if (register_width(memory_field.index) != register_width(memory_field.base)) {
            throw std::logic_error(
                    "Assertion failed: index and base fields have differing widths @ push_ASOR_if_dword");
        }
        if (register_width(memory_field.base) == mnemo_t::width_t::Dword) {
            out.push_back(0x67);
        }
    }

    // Put an "operand-size override" prefix if operand width = word
    // In 64 bit mode switches operand width from 32 bits to 16 bits
    static auto push_OSOR_if_word(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.width == mnemo_t::width_t::Word) {
            out.push_back(0x66);
        }
    }

    // Put a REX prefix if operand width = qword
    static auto push_rex_if_qword(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.width == mnemo_t::width_t::Qword) {
            out.push_back(0b01001000);
        }
    }

    // Returns opcode1 if mnemo.width == byte, else returns opcode2
    static auto pick_opcode_byte_or_else(const mnemo_t &mnemo, u8 opcode1, u8 opcode2) -> u8 {
        switch (mnemo.width) {
            case mnemo_t::width_t::Byte:
                return opcode1;
            case mnemo_t::width_t::Word:
            case mnemo_t::width_t::Dword:
            case mnemo_t::width_t::Qword:
                return opcode2;
            default:
                throw std::logic_error("Unsupported width!");
        }
    }

    // A template for a mnemo which operates on memory.
    // These mnemos are encoded in the same way and the only difference is opcodes.
    // opcode1 -- opcode for a byte-wise operation
    // opcode2 -- opcode for operations with other widths
    //
    // Example:
    // mov mem, reg
    // is
    // assemble_memory_mnemo(..., 0x88, 0x89)
    //
    // mov reg, mem
    // is
    // assemble_memory_mnemo(..., 0x8a, 0x8b)
    //
    // add mem, reg
    // is
    // assemble_memory_mnemo(..., 0x00, 0x01)
    //
    // add reg, mem
    // is
    // assemble_memory_mnemo(..., 0x02, 0x03)
    //
    // cool isn't it question_mark
    //
    // NOTE: Operand Encoding variants are: MR and RM
    // MR means first operand is memory, second is register
    // RM means first operand is register, second is memory
    // This function will deduce variant from `mnemo` parameter a1 and a2 fields
    // For example:
    // mov r/m32 r32 ; MR
    // mov r32 r/m32 ; RM
    static auto assemble_memory_mnemo(vector<u8> &out, const mnemo_t &mnemo, u8 opcode1, u8 opcode2) -> void {
        const mnemo_t::arg_t *memory_arg;
        const mnemo_t::arg_t *register_arg;
        if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory && mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
            // MR
            memory_arg = &mnemo.a1;
            register_arg = &mnemo.a2;
        } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register && mnemo.a2.tag == mnemo_t::arg_t::tag_t::Memory) {
            // MR
            memory_arg = &mnemo.a2;
            register_arg = &mnemo.a1;
        } else {
            throw std::logic_error("Unexpected mnemo shape @ assemble_memory_mnemo");
        }

        push_ASOR_if_dword(out, memory_arg->data.memory);

        u8 opcode = pick_opcode_byte_or_else(mnemo, opcode1, opcode2);
        u8 reg = reg_to_number(register_arg->data.reg);
        u8 mod;
        u8 rm;

        // Fill in "mod" and "rm"
        {
            // This algorithm will always encodes the instruction using SIB byte. This is suboptimal since
            // mov [eax], eax
            // can be encoded without the SIB byte, using simple modrm form.
            // TODO: implement simple modrm adressing

            u8 scale;
            u8 index;
            u8 base;

            // Choose disp size
            if (memory_arg->data.memory.disp == 0) {
                // no disp
                mod = 0b00;
            } else if (-128 <= memory_arg->data.memory.disp && memory_arg->data.memory.disp <= 127) {
                // disp8
                mod = 0b01;
            } else {
                // disp32
                mod = 0b10;
            }

            // Use SIB
            rm = 0b100;

            // Fill in SIB

            // NOTE: Put simply, SIB adressing does not allow to adress [scaled index] + [EBP].
            // Instead that bit combination means no base ([scaled index] + disp32).
            // (00 xxx 100) (xx xxx 101)
            // mod reg rm    ss index base

            // NOTE: Also, SIB adressing does not allow to use ESP as index.
            // Instead that bit combination means no index ([base] + dispxx). scale has no effect in this case
            // (xx xxx 100) (nn 100 xxx)
            // mod reg rm    ss index base

            if ((mod == 0b00 && memory_arg->data.memory.base == mnemo_t::arg_t::reg_t::Ebp) ||
                memory_arg->data.memory.index == mnemo_t::arg_t::reg_t::Esp) {
                throw std::logic_error("Bruh");
                // TODO: handle corner cases
            }

            // Handle scale_t::S0 edge case
            if (memory_arg->data.memory.scale == mnemo_t::arg_t::memory_t::scale_t::S0) {
                scale = 0b00; // In fact it can have any value
                index = 0b100;
            } else {
                scale = scale_to_byte(memory_arg->data.memory.scale);
                index = reg_to_number(memory_arg->data.memory.index);
            }
            base = reg_to_number(memory_arg->data.memory.base);

            push_OSOR_if_word(out, mnemo);
            push_rex_if_qword(out, mnemo);
            out.push_back(opcode);
            out.push_back(mod_and_reg_and_rm_to_modrm(mod, reg, rm));
            out.push_back(scale_and_index_and_base_to_sib(scale, index, base));

            // Append the disp
            if (memory_arg->data.memory.disp == 0) {
                // no disp
            } else if (-128 <= memory_arg->data.memory.disp && memory_arg->data.memory.disp <= 127) {
                // disp8
                out.push_back(memory_arg->data.memory.disp);
            } else {
                i32 disp = memory_arg->data.memory.disp;
                out.push_back(disp & 0xFF);
                disp >>= 8;
                out.push_back(disp & 0xFF);
                disp >>= 8;
                out.push_back(disp & 0xFF);
                disp >>= 8;
                out.push_back(disp & 0xFF);
            }
        }
    }

    static auto assemble_mnemo_mov(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.tag != mnemo_t::tag_t::Mov)
            throw std::logic_error("Wrong mnemo!");

        if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
            mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
            u8 opcode = pick_opcode_byte_or_else(mnemo, 0x88, 0x89);
            u8 mod = 0b11;
            u8 rm = reg_to_number(mnemo.a1.data.reg);
            u8 reg = reg_to_number(mnemo.a2.data.reg);

            push_OSOR_if_word(out, mnemo);
            push_rex_if_qword(out, mnemo);
            out.push_back(opcode);
            out.push_back(mod_and_reg_and_rm_to_modrm(mod, reg, rm));
        } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
                   mnemo.a2.tag == mnemo_t::arg_t::tag_t::Immediate) {
            u8 opcode = pick_opcode_byte_or_else(mnemo, 0xb0, 0xb8);
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
            assemble_memory_mnemo(out, mnemo, 0x88, 0x89);
        } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
                   mnemo.a2.tag == mnemo_t::arg_t::tag_t::Memory) {
            assemble_memory_mnemo(out, mnemo, 0x8a, 0x8b);
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