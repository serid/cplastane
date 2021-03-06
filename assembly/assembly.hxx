#pragma once

#include "../strvec.hxx"
#include "../int.hxx"

namespace assembly {
    typedef i32 disp_t;
    typedef i64 imm_t;

    struct mnemo_t {
        struct arg_t {
            enum class reg_t {
                Undef,
                Al,
                Bl,
                Cl,
                Dl,
                Ah,
                Bh,
                Ch,
                Dh,
                Ax,
                Bx,
                Cx,
                Dx,
                Sp,
                Bp,
                Si,
                Di,
                Eax,
                Ebx,
                Ecx,
                Edx,
                Esp,
                Ebp,
                Esi,
                Edi,
                Rax,
                Rbx,
                Rcx,
                Rdx,
                Rsp,
                Rbp,
                Rsi,
                Rdi,
            };

            struct memory_t {
                reg_t base;
                reg_t index;
                enum class scale_t {
                    Undef,
                    S0, // If scale is S0, the index register should be Undef
                    S1,
                    S2,
                    S4,
                    S8,
                } scale;

                disp_t disp;
            };

            enum class tag_t {
                Undef,
                Immediate,
                Register,
                Memory,
            } tag;
            union {
                imm_t imm;
                reg_t reg;
                memory_t memory;
            } data;

            static arg_t imm(imm_t imm);

            static arg_t reg(reg_t reg);

            static arg_t mem(reg_t base, reg_t index, memory_t::scale_t scale, disp_t disp);

            auto print() const -> void;

            auto static print_reg(reg_t reg) -> void;
        };

        enum class tag_t {
            Undef,
            Mov,
            Add,
            Push,
            Pop,
            Ret,
        } tag;

        enum class width_t {
            Undef,
            NotSet, // Used in instructions which don't care about width like `ret`
            Byte,
            Word,
            Dword,
            Qword,
        } width;

        arg_t a1, a2;

        auto print() const -> void;

        auto check_validity() const -> void;

        [[nodiscard]] auto get_arity() const -> u8;

        auto static print_width(width_t width) -> void;
    };

    auto assemble(const vector<mnemo_t> &mnemos) -> vector<u8>;
}
