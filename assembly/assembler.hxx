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
                Eax,
                Ebx,
                Ecx,
                Edx,
                Rax,
                Rbx,
                Rcx,
                Rdx,
            };

            enum class tag_t {
                Undef,
                Immediate,
                Register,
                Memory,
            } tag;
            union data_t {
                i64 imm;
                reg_t reg;
                struct memory_t {
                    reg_t base;
                    reg_t index;
                    u8 scale;
                    i32 disp;
                } memory;
            } data;
        };

        enum class tag_t {
            Undef,
            Mov,
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
    };

    auto assemble_mnemo(vector<u8> &out, const mnemo_t &mnemo) -> void;

    auto assemble(const vector<mnemo_t> &mnemos) -> vector<u8>;
}
