#include "assembly.hxx"

#include <iostream>

using namespace std;
using namespace assembly;

static auto can_be_encoded_in_32bits(i64 n) -> bool {
    // True if n is in union of sets of valid values for i32 and u32
    // return (numeric_limits<i32>::min() <= n && n <= numeric_limits<i32>::max()) ||
    //        (numeric_limits<u32>::min() <= n && n <= numeric_limits<u32>::max());
    return numeric_limits<i32>::min() <= n && n <= numeric_limits<u32>::max();
}

static auto can_be_encoded_in_8bits(i64 n) -> bool {
    // True if n is in union of sets of valid values for i8 and u8
    // return (numeric_limits<i8>::min() <= n && n <= numeric_limits<i8>::max()) ||
    //        (numeric_limits<u8>::min() <= n && n <= numeric_limits<u8>::max());
    return numeric_limits<i8>::min() <= n && n <= numeric_limits<u8>::max();
}

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
            throw logic_error("Unsupported register! register_width");
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
            throw logic_error("Unsupported register!");
    }
}

// scale_t::S0 should be handled outside this function since it is not encoded with SS with but rather with Index bits equal to 0b100
static auto scale_to_num(mnemo_t::arg_t::memory_t::scale_t scale) -> u8 {
    switch (scale) {
        case mnemo_t::arg_t::memory_t::scale_t::S0:
            throw logic_error("Unexpected scale_t::S0 @ scale_to_num");
        case mnemo_t::arg_t::memory_t::scale_t::S1:
            return 0b00;
        case mnemo_t::arg_t::memory_t::scale_t::S2:
            return 0b01;
        case mnemo_t::arg_t::memory_t::scale_t::S4:
            return 0b10;
        case mnemo_t::arg_t::memory_t::scale_t::S8:
            return 0b11;
        default:
            throw logic_error("Scale is Undef.");
    }
}

static auto mod_and_reg_and_rm_to_modrm(u8 mod, u8 reg, u8 rm) -> u8 {
    return (mod << 6) | (reg << 3) | (rm << 0);
}

static auto scale_and_index_and_base_to_sib(u8 scale, u8 index, u8 base) -> u8 {
    return (scale << 6) | (index << 3) | (base << 0);
}

// For mnemos that only allow imms up to 32 bits. 32 bit value will be sign-extended in memory to 64 bits
static auto assert_imm_not_larger_than_32_bits(mnemo_t::width_t &width, imm_t imm, const string &msg) -> void {
    if (width == mnemo_t::width_t::Qword) {
        width = mnemo_t::width_t::Dword;
        if (!can_be_encoded_in_32bits(imm))
            throw logic_error(msg);
    }
}

// Put an "address-size override" prefix if address width = dword
// In 64 bit mode switches address width from 64 bits to 32 bits
static auto push_ASOR_if_dword(vector<u8> &out, const mnemo_t::arg_t::memory_t &memory_field) -> void {
    // If index register is defined and its width does not match that of base, throw and error
    if (memory_field.index != mnemo_t::arg_t::reg_t::Undef &&
        register_width(memory_field.index) != register_width(memory_field.base)) {
        throw logic_error(
                "Assertion failed: index and base fields have differing widths @ push_ASOR_if_dword");
    }
    if (register_width(memory_field.base) == mnemo_t::width_t::Dword) {
        out.push_back(0x67);
    }
}

// Put an "operand-size override" prefix if operand width = word
// In 64 bit mode switches operand width from 32 bits to 16 bits
static auto push_OSOR_if_word(vector<u8> &out, mnemo_t::width_t width) -> void {
    if (width == mnemo_t::width_t::Word) {
        out.push_back(0x66);
    }
}

// Put a REX prefix if operand width = qword
static auto push_rex_if_qword(vector<u8> &out, mnemo_t::width_t width) -> void {
    if (width == mnemo_t::width_t::Qword) {
        out.push_back(0b01001000);
    }
}

static auto append_disp(vector<u8> &out, disp_t a_disp) -> void {
    // Append the disp
    if (a_disp == 0) {
        // no disp
    } else if (-128 <= a_disp && a_disp <= 127) {
        // disp8
        out.push_back(a_disp);
    } else {
        // disp32
        i32 disp = a_disp;
        out.push_back(disp & 0xff);
        disp >>= 8;
        out.push_back(disp & 0xff);
        disp >>= 8;
        out.push_back(disp & 0xff);
        disp >>= 8;
        out.push_back(disp & 0xff);
    }
}

static auto append_imm_upto_64(vector<u8> &out, mnemo_t::width_t width, imm_t a_imm) -> void {
    switch (width) {
        case mnemo_t::width_t::Byte: {
            // Write i8
            out.push_back(a_imm);
            break;
        }
        case mnemo_t::width_t::Word: {
            // Write LE i16
            i16 imm = a_imm;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            break;
        }
        case mnemo_t::width_t::Dword: {
            // Write LE i32
            i32 imm = a_imm;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            break;
        }
        case mnemo_t::width_t::Qword: {
            // Write LE i64
            i64 imm = a_imm;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            imm >>= 8;
            out.push_back(imm & 0xff);
            break;
        }
        default:
            throw logic_error("Unsupported width! @ append_imm_upto_64");
    }
}

// Pushes opcode1 if mnemo.width == byte, else opcode2
static auto
push_operand_width_prefixes_and_opcode(vector<u8> &out, mnemo_t::width_t width, u8 opcode1, u8 opcode2) -> void {
    push_OSOR_if_word(out, width);
    push_rex_if_qword(out, width);
    switch (width) {
        case mnemo_t::width_t::Byte:
            out.push_back(opcode1);
            break;
        case mnemo_t::width_t::Word:
        case mnemo_t::width_t::Dword:
        case mnemo_t::width_t::Qword:
            out.push_back(opcode2);
            break;
        default:
            throw logic_error("Unsupported width!");
    }
}

struct assemble_memory_mnemo_result {
    u8 mod;
    u8 rm;
    u8 sib;
    bool sib_eh; // Indicates whether sib was returned. SIB is not returned for short form memory adressing and returned for long form.
};

static auto assemble_memory_mnemo(const mnemo_t::arg_t::memory_t &memory) -> assemble_memory_mnemo_result {
    u8 mod;
    u8 rm;

    u8 scale = 0xFF;
    u8 index = 0xFF;
    u8 base = 0xFF;

    // Fill in "mod" and "rm"

    // Choose disp size
    if (memory.disp == 0) {
        // no disp
        mod = 0b00;
    } else if (-128 <= memory.disp && memory.disp <= 127) {
        // disp8
        mod = 0b01;
    } else {
        // disp32
        mod = 0b10;
    }

    // Short address form does not allow using "index"/"scale" or using "esp"/"rsp" as a base register.
    // It also disallows using "ebp" base without displacement.
    bool is_short = !(memory.scale != mnemo_t::arg_t::memory_t::scale_t::S0 ||
                      memory.base == mnemo_t::arg_t::reg_t::Esp ||
                      memory.base == mnemo_t::arg_t::reg_t::Rsp ||
                      (memory.disp == 0b00 && memory.base == mnemo_t::arg_t::reg_t::Rbp));

    if (is_short) {
        // Encode addressing without SIB byte

        rm = reg_to_number(memory.base);
    } else {
        // Encode addressing using SIB byte

        // Use SIB
        rm = 0b100;

        // Fill in SIB

        // NOTE: Put simply, SIB adressing does not allow to adress [scaled index] + [EBP].
        // Instead that bit combination means no base ([scaled index] + disp32).
        // (00 xxx 100) (xx xxx 101)
        // mod reg rm    ss index base
        //
        // NOTE: Also, SIB adressing does not allow to use ESP as index.
        // Instead that bit combination means no index ([base] + dispxx). scale has no effect in this case
        // (xx xxx 100) (nn 100 xxx)
        // mod reg rm    ss index base
        if ((mod == 0b00 && memory.base == mnemo_t::arg_t::reg_t::Ebp) ||
            memory.index == mnemo_t::arg_t::reg_t::Esp) {
            throw logic_error("Unsupported operation.");
            // TO DO: handle corner cases
        }

        // Handle scale_t::S0 edge case
        if (memory.scale == mnemo_t::arg_t::memory_t::scale_t::S0) {
            scale = 0b00; // In fact it can have any value
            index = 0b100;
        } else {
            scale = scale_to_num(memory.scale);
            index = reg_to_number(memory.index);
        }
        base = reg_to_number(memory.base);
    }

    assemble_memory_mnemo_result result{};
    result.mod = mod;
    result.rm = rm;

    if (is_short) {
        result.sib = 0b11111111; // Should not be used
        result.sib_eh = false;
    } else {
        result.sib = scale_and_index_and_base_to_sib(scale, index, base);
        result.sib_eh = true;
    }

    return result;
}

// A template for a mnemo which operates on memory and a register.
// These mnemos are encoded in a similar way and only differ in opcodes used.
//
// Operand Encoding variants are: MR and RM
// MR means first operand is memory, second is register
// RM means first operand is register, second is memory
// This function will deduce variant from `mnemo` parameter a1 and a2 fields
// For example:
// mov r/m32 r32 ; MR
// mov r32 r/m32 ; RM
static auto
assemble_memory_register_mnemos_template(vector<u8> &out, const mnemo_t &mnemo, u8 opcode1, u8 opcode2) -> void {
    const mnemo_t::arg_t *memory_arg;
    const mnemo_t::arg_t *register_arg;
    if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory && mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
        // MR
        memory_arg = &mnemo.a1;
        register_arg = &mnemo.a2;
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register && mnemo.a2.tag == mnemo_t::arg_t::tag_t::Memory) {
        // RM
        memory_arg = &mnemo.a2;
        register_arg = &mnemo.a1;
    } else {
        throw logic_error("Unexpected mnemo shape @ assemble_memory_register_mnemos_template");
    }

    u8 reg = reg_to_number(register_arg->data.reg);

    assemble_memory_mnemo_result result = assemble_memory_mnemo(memory_arg->data.memory);

    push_ASOR_if_dword(out, memory_arg->data.memory);
    push_operand_width_prefixes_and_opcode(out, mnemo.width, opcode1, opcode2);
    out.push_back(mod_and_reg_and_rm_to_modrm(result.mod, reg, result.rm));
    if (result.sib_eh) {
        out.push_back(result.sib);
    }

    append_disp(out, memory_arg->data.memory.disp);
}

static auto assemble_mnemo_mov(vector<u8> &out, const mnemo_t &mnemo) -> void {
    if (mnemo.tag != mnemo_t::tag_t::Mov)
        throw logic_error("Wrong mnemo!");

    if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
        mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
        u8 mod = 0b11;
        u8 rm = reg_to_number(mnemo.a1.data.reg);
        u8 reg = reg_to_number(mnemo.a2.data.reg);

        push_operand_width_prefixes_and_opcode(out, mnemo.width, 0x88, 0x89);
        out.push_back(mod_and_reg_and_rm_to_modrm(mod, reg, rm));
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Immediate) {
        u8 opcode_shift = reg_to_number(mnemo.a1.data.reg);

        push_operand_width_prefixes_and_opcode(out, mnemo.width, 0xb0 + opcode_shift, 0xb8 + opcode_shift);

        // Write imm
        append_imm_upto_64(out, mnemo.width, mnemo.a2.data.imm);
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
        assemble_memory_register_mnemos_template(out, mnemo, 0x88, 0x89);
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Memory) {
        assemble_memory_register_mnemos_template(out, mnemo, 0x8a, 0x8b);
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Immediate) {
        u8 reg = 0;  // Exactly 0 (/digit value)

        assemble_memory_mnemo_result result = assemble_memory_mnemo(mnemo.a1.data.memory);

        push_ASOR_if_dword(out, mnemo.a1.data.memory);
        push_operand_width_prefixes_and_opcode(out, mnemo.width, 0xc6, 0xc7);
        out.push_back(mod_and_reg_and_rm_to_modrm(result.mod, reg, result.rm));
        if (result.sib_eh) {
            out.push_back(result.sib);
        }

        append_disp(out, mnemo.a1.data.memory.disp);

        mnemo_t::width_t width = mnemo.width;
        assert_imm_not_larger_than_32_bits(width, mnemo.a2.data.imm,
                                           "Attempted to move immediate 64 bit value to memory using MOV @ assemble_mnemo_mov");
        append_imm_upto_64(out, width, mnemo.a2.data.imm);
    } else {
        throw logic_error("Unsupported mov shape!");
    }
}

static auto assemble_mnemo_add(vector<u8> &out, const mnemo_t &mnemo) -> void {
    if (mnemo.tag != mnemo_t::tag_t::Add)
        throw logic_error("Wrong mnemo!");

    if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
        mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
        u8 mod = 0b11;
        u8 rm = reg_to_number(mnemo.a1.data.reg);
        u8 reg = reg_to_number(mnemo.a2.data.reg);

        push_operand_width_prefixes_and_opcode(out, mnemo.width, 0x00, 0x01);
        out.push_back(mod_and_reg_and_rm_to_modrm(mod, reg, rm));
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Immediate) {
        // Add has three encoding variantions:
        // 1. Special short encod for al/ax/eax/rax
        // 2. Special short encod for imm8 operand and reg of any width
        // 3. Normal
        if (can_be_encoded_in_8bits(mnemo.a2.data.imm) && mnemo.width != mnemo_t::width_t::Byte) {
            // Will add a sign-extended imm8 to a register
            u8 mod = 0b11;
            u8 rm = reg_to_number(mnemo.a1.data.reg);
            u8 reg = 0; // Exactly 0 (/digit value)

            push_operand_width_prefixes_and_opcode(out, mnemo.width, 0xee, 0x83);
            out.push_back(mod_and_reg_and_rm_to_modrm(mod, reg, rm));

            append_imm_upto_64(out, mnemo_t::width_t::Byte, mnemo.a2.data.imm);
        } else if (mnemo.a1.data.reg == mnemo_t::arg_t::reg_t::Al ||
                   mnemo.a1.data.reg == mnemo_t::arg_t::reg_t::Ax ||
                   mnemo.a1.data.reg == mnemo_t::arg_t::reg_t::Eax ||
                   mnemo.a1.data.reg == mnemo_t::arg_t::reg_t::Rax) {
            push_operand_width_prefixes_and_opcode(out, mnemo.width, 0x04, 0x05);

            mnemo_t::width_t width = mnemo.width;
            assert_imm_not_larger_than_32_bits(width, mnemo.a2.data.imm,
                                               "Attempted to add immediate 64 bit value to memory using MOV @ assemble_mnemo_add");
            append_imm_upto_64(out, width, mnemo.a2.data.imm);
        } else {
            u8 mod = 0b11;
            u8 rm = reg_to_number(mnemo.a1.data.reg);
            u8 reg = 0; // Exactly 0 (/digit value)

            push_operand_width_prefixes_and_opcode(out, mnemo.width, 0x80, 0x81);
            out.push_back(mod_and_reg_and_rm_to_modrm(mod, reg, rm));

            mnemo_t::width_t width = mnemo.width;
            assert_imm_not_larger_than_32_bits(width, mnemo.a2.data.imm,
                                               "Attempted to add immediate 64 bit value to memory using MOV @ assemble_mnemo_add");
            append_imm_upto_64(out, width, mnemo.a2.data.imm);
        }
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
        assemble_memory_register_mnemos_template(out, mnemo, 0x00, 0x01);
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Memory) {
        assemble_memory_register_mnemos_template(out, mnemo, 0x02, 0x03);
    } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory &&
               mnemo.a2.tag == mnemo_t::arg_t::tag_t::Immediate) {
        u8 reg = 0; // Exactly 0 (/digit value)

        assemble_memory_mnemo_result result = assemble_memory_mnemo(mnemo.a1.data.memory);

        push_ASOR_if_dword(out, mnemo.a1.data.memory);
        if (can_be_encoded_in_8bits(mnemo.a2.data.imm) && mnemo.width != mnemo_t::width_t::Byte) {
            // Will add a sign-extended imm8 to a memory
            push_operand_width_prefixes_and_opcode(out, mnemo.width, 0xee, 0x83);
        } else {
            // Will add a full imm to a memory
            push_operand_width_prefixes_and_opcode(out, mnemo.width, 0x80, 0x81);
        }
        out.push_back(mod_and_reg_and_rm_to_modrm(result.mod, reg, result.rm));
        if (result.sib_eh) {
            out.push_back(result.sib);
        }
        append_disp(out, mnemo.a1.data.memory.disp);

        mnemo_t::width_t width = mnemo.width;
        assert_imm_not_larger_than_32_bits(width, mnemo.a2.data.imm,
                                           "Attempted to add immediate 64 bit value to memory using MOV @ assemble_mnemo_add");
        append_imm_upto_64(out, width, mnemo.a2.data.imm);
    } else {
        throw logic_error("Unsupported add shape!");
    }
}

// `pop` operates similarly to `push` save for different opcodes and inability to accept immediate arguments.
// Based on this, I can unify two functions under a template, where argument chooses what operation to encode.
template<bool is_push>
static auto assemble_mnemo_push_pop_template(vector<u8> &out, const mnemo_t &mnemo) -> void {
    if (is_push) {
        if (mnemo.tag != mnemo_t::tag_t::Push)
            throw logic_error("Wrong mnemo!");
    } else {
        if (mnemo.tag != mnemo_t::tag_t::Pop)
            throw logic_error("Wrong mnemo!");
    }

    switch (mnemo.a1.tag) {
        case mnemo_t::arg_t::tag_t::Register: {
            // In 64-bit mode, `push` and `pop` only accept 16-bit or 64-bit registers.
            if (mnemo.width == mnemo_t::width_t::Byte || mnemo.width == mnemo_t::width_t::Dword) {
                throw logic_error(
                        string("Unsupported register @ assemble_mnemo_") + (is_push ? "push" : "pop"));
            }
            u8 opcode = is_push ? 0x50 : 0x58;
            opcode += reg_to_number(mnemo.a1.data.reg);

            push_OSOR_if_word(out, mnemo.width);
            out.push_back(opcode);
            break;
        }
        case mnemo_t::arg_t::tag_t::Memory: {
            // In 64-bit mode, `push` and `pop` only accept 16-bit or 64-bit registers.
            if (mnemo.width == mnemo_t::width_t::Byte || mnemo.width == mnemo_t::width_t::Dword) {
                throw logic_error(
                        string("Unsupported register @ assemble_mnemo_") + (is_push ? "push" : "pop"));
            }
            u8 opcode = is_push ? 0xff : 0x8f;
            u8 reg = is_push ? 6 : 0;

            assemble_memory_mnemo_result result = assemble_memory_mnemo(mnemo.a1.data.memory);

            push_ASOR_if_dword(out, mnemo.a1.data.memory);
            push_OSOR_if_word(out, mnemo.width);
            out.push_back(opcode);
            out.push_back(mod_and_reg_and_rm_to_modrm(result.mod, reg, result.rm));
            if (result.sib_eh) {
                out.push_back(result.sib);
            }
            append_disp(out, mnemo.a1.data.memory.disp);
            break;
        }
        case mnemo_t::arg_t::tag_t::Immediate: {
            if (!is_push) {
                throw logic_error("Cannot `pop` into an immediate value.");
            }

            if (mnemo.width == mnemo_t::width_t::Qword) {
                throw logic_error("Unsupported width! @ assemble_mnemo_push");
            }

            // Encode `push`
            push_operand_width_prefixes_and_opcode(out, mnemo.width, 0x6a, 0x68);
            append_imm_upto_64(out, mnemo.width, mnemo.a1.data.imm);
            break;
        }
        default:
            throw logic_error(
                    string("Unsupported mnemo shape @ assemble_mnemo_") + (is_push ? "push" : "pop"));
    }
}

static auto assemble_mnemo(vector<u8> &out, const mnemo_t &mnemo) -> void {
    mnemo.check_validity();

    switch (mnemo.tag) {
        case mnemo_t::tag_t::Mov: {
            assemble_mnemo_mov(out, mnemo);
            break;
        }
        case mnemo_t::tag_t::Add: {
            assemble_mnemo_add(out, mnemo);
            break;
        }
        case mnemo_t::tag_t::Push: {
            assemble_mnemo_push_pop_template<true>(out, mnemo);
            break;
        }
        case mnemo_t::tag_t::Pop: {
            assemble_mnemo_push_pop_template<false>(out, mnemo);
            break;
        }
        case mnemo_t::tag_t::Ret: {
            out.push_back(0xc3);
            break;
        }
        default:
            throw logic_error("Unimplemented mnemo.tag!");
    }
}

namespace assembly {
    auto assemble(const vector<mnemo_t> &mnemos) -> vector<u8> {
        vector<u8> result{};
        for (const mnemo_t &mnemo:mnemos) {
            assemble_mnemo(result, mnemo);
        }
        return result;
    }

    mnemo_t::arg_t mnemo_t::arg_t::imm(imm_t imm) {
        return {
                .tag=tag_t::Immediate,
                .data {.imm = imm},
        };
    }

    mnemo_t::arg_t mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t reg) {
        arg_t o{};
        o.tag = tag_t::Register;
        o.data.reg = reg;
        return o;
    }

    mnemo_t::arg_t mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t base, mnemo_t::arg_t::reg_t index,
                                       mnemo_t::arg_t::memory_t::scale_t scale, disp_t disp) {
        // If scale is S0, the index register should be Undef
        if (scale == mnemo_t::arg_t::memory_t::scale_t::S0 && index != mnemo_t::arg_t::reg_t::Undef)
            throw logic_error("if scale is S0, the index register should be Undef");

        arg_t o{};
        o.tag = tag_t::Memory;
        o.data.memory = {
                .base=base,
                .index=index,
                .scale=scale,
                .disp=disp,
        };
        return o;
    }

    auto mnemo_t::arg_t::print() const -> void {
        switch (this->tag) {
            case tag_t::Immediate:
                cout << this->data.imm;
                break;
            case tag_t::Register:
                print_reg(this->data.reg);
                break;
            case tag_t::Memory:
                cout << "[";

                print_reg(this->data.memory.base);

                // Print index with scale if there is one
                if (this->data.memory.scale != memory_t::scale_t::S0) {
                    cout << " + ";

                    print_reg(this->data.memory.index);

                    // Print scale if it is not S1
                    if (this->data.memory.scale != memory_t::scale_t::S1) {
                        cout << " * ";

                        switch (this->data.memory.scale) {
                            case memory_t::scale_t::S1:
                                cout << "1";
                                break;
                            case memory_t::scale_t::S2:
                                cout << "2";
                                break;
                            case memory_t::scale_t::S4:
                                cout << "4";
                                break;
                            case memory_t::scale_t::S8:
                                cout << "8";
                                break;
                            default:
                                throw logic_error("unreachable");
                        }
                    }
                }

                // Print disp if it is not zero
                if (this->data.memory.disp != 0) {
                    if (this->data.memory.disp > 0)
                        cout << " + ";
                    else
                        cout << " - ";
                    cout << abs(this->data.memory.disp);
                }
                cout << "]";
                break;
            default:
                throw logic_error("unreachable");
        }
    }

    auto mnemo_t::arg_t::print_reg(reg_t reg) -> void {
        switch (reg) {
            case reg_t::Al:
                cout << "al";
                break;
            case reg_t::Bl:
                cout << "bl";
                break;
            case reg_t::Cl:
                cout << "cl";
                break;
            case reg_t::Dl:
                cout << "dl";
                break;
            case reg_t::Ah:
                cout << "ah";
                break;
            case reg_t::Bh:
                cout << "bh";
                break;
            case reg_t::Ch:
                cout << "ch";
                break;
            case reg_t::Dh:
                cout << "dh";
                break;
            case reg_t::Ax:
                cout << "ax";
                break;
            case reg_t::Bx:
                cout << "bx";
                break;
            case reg_t::Cx:
                cout << "cx";
                break;
            case reg_t::Dx:
                cout << "dx";
                break;
            case reg_t::Eax:
                cout << "eax";
                break;
            case reg_t::Ebx:
                cout << "ebx";
                break;
            case reg_t::Ecx:
                cout << "ecx";
                break;
            case reg_t::Edx:
                cout << "edx";
                break;
            case reg_t::Esp:
                cout << "esp";
                break;
            case reg_t::Ebp:
                cout << "ebp";
                break;
            case reg_t::Esi:
                cout << "esi";
                break;
            case reg_t::Edi:
                cout << "edi";
                break;
            case reg_t::Rax:
                cout << "rax";
                break;
            case reg_t::Rbx:
                cout << "rbx";
                break;
            case reg_t::Rcx:
                cout << "rcx";
                break;
            case reg_t::Rdx:
                cout << "rdx";
                break;
            case reg_t::Rsp:
                cout << "rsp";
                break;
            case reg_t::Rbp:
                cout << "rbp";
                break;
            case reg_t::Rsi:
                cout << "rsi";
                break;
            case reg_t::Rdi:
                cout << "rdi";
                break;
            default:
                throw logic_error("unsupported register. print_reg");
        }
    }

    auto mnemo_t::print() const -> void {
        cout << "mnemo_t(";
        switch (this->tag) {
            case tag_t::Mov:
                cout << "mov";
                break;
            case tag_t::Add:
                cout << "add";
                break;
            case tag_t::Push:
                cout << "push";
                break;
            case tag_t::Pop:
                cout << "pop";
                break;
            case tag_t::Ret:
                cout << "ret";
                break;
            default:
                throw logic_error("unimplemented mnemo.tag");
        }

        if (this->get_arity() >= 1) {
            std::cout << ' ';

            print_width(this->width);

            std::cout << ' ';

            this->a1.print();

            if (this->get_arity() >= 2) {
                std::cout << ' ';

                this->a2.print();
            }
        }

        cout << ")";
    }

    auto mnemo_t::print_width(width_t width) -> void {
        switch (width) {
            case width_t::Byte:
                cout << "BYTE";
                break;
            case width_t::Word:
                cout << "WORD";
                break;
            case width_t::Dword:
                cout << "DWORD";
                break;
            case width_t::Qword:
                cout << "QWORD";
                break;
            case width_t::NotSet:
                cout << "NotSet";
                break;
            default:
                throw logic_error("unimplemented width_t. print_width");
        }
    }

    // Perform basic validity checks for a mnemonic
    auto mnemo_t::check_validity() const -> void {
        if (this->tag == mnemo_t::tag_t::Undef)
            throw logic_error("mnemo has Undef tag. assemble_mnemo");
        if (this->width == mnemo_t::width_t::Undef)
            throw logic_error("mnemo has Undef width. assemble_mnemo");
        if (this->a1.tag == mnemo_t::arg_t::tag_t::Register && register_width(this->a1.data.reg) != this->width) {
            throw logic_error("arg1 register width does not match instruction width");
        }
        if (this->a2.tag == mnemo_t::arg_t::tag_t::Register && register_width(this->a2.data.reg) != this->width) {
            throw logic_error("arg2 register width does not match instruction width");
        }
    }

    // Returns arity of a mnemo (how many arguments it takes)
    auto mnemo_t::get_arity() const -> u8 {
        switch (this->tag) {
            case tag_t::Mov:
            case tag_t::Add:
                return 2;
            case tag_t::Push:
            case tag_t::Pop:
                return 1;
            case tag_t::Ret:
                return 0;
            default:
                throw logic_error("unhandled mnemo.tag @ get_arity");
        }
    }
}