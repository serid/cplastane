#pragma once

#include "../strvec.hxx"
#include "../int.hxx"

namespace assembly {
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
                    S0,
                    S1,
                    S2,
                    S4,
                    S8,
                } scale;
                i32 disp;
            };

            enum class tag_t {
                Undef,
                Immediate,
                Register,
                Memory,
            } tag;
            union {
                i64 imm;
                reg_t reg;
                memory_t memory;
            } data;

            static arg_t imm(i64 imm);

            static arg_t reg(reg_t reg);

            static arg_t mem(reg_t base, reg_t index, memory_t::scale_t scale, i32 disp);

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

        auto static print_width(width_t width) -> void;
    };

    auto assemble_mnemo(vector<u8> &out, const mnemo_t &mnemo) -> void;

    auto assemble(const vector<mnemo_t> &mnemos) -> vector<u8>;
}
