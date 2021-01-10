#include "assembly_test.hxx"

#include <iostream>

#include "../assembly/assembler.hxx"
#include "../jit/jit.hxx"

using assembly::mnemo_t;

namespace test {
    struct test_t {
        string name;
        i64 expected_result;
        vector<mnemo_t> mnemos;
    };

    static auto test_func(const test_t &x_test) -> bool {
        std::cout << ">> Test \"" << x_test.name << "\".\n";

        vector<u8> bytes = assemble(x_test.mnemos);

        std::cout << "Compilation successful, bytes: ";

        std::cout << "[";
        for (const u8 &n:bytes) {
            std::cout << std::hex << i32(n) << std::dec << ", ";
        }
        std::cout << "]\n";

        i64 result = jit::eval_mc(bytes.data(), bytes.size());
        if (result != x_test.expected_result) {
            std::cout << "Test failed.\nExpected result: " << x_test.expected_result
                      << "\nActual result: " << result << "\n";
            return false;
        } else {
            std::cout << "Success. Result: (" << result << ").\n";
            return true;
        }
    }

    auto test_jit() -> void {
        vector<test_t> tests = {
                // mov eax, 100
                // ret
                {.name="Simple `return 100;`", .expected_result=100, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Eax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 100}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }
                }},


                // mov ecx, 0xff00
                // mov eax, ecx
                // ret
                {.name="Reg mov test", .expected_result=0xff00, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Ecx}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0xff00}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Eax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Ecx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }

                }},


                // mov rcx, 0x0f1f2f3f4f5f6f7f
                // mov rax, rcx
                // ret
                {.name="64 bit reg mov test", .expected_result=0x0f1f2f3f4f5f6f7f, .mnemos={
                        {
                                {
                                        .tag = mnemo_t::tag_t::Mov,
                                        .width = mnemo_t::width_t::Qword,
                                        .a1 = {
                                                .tag = mnemo_t::arg_t::tag_t::Register,
                                                .data = {.reg = mnemo_t::arg_t::reg_t::Rcx}
                                        },
                                        .a2 = {
                                                .tag = mnemo_t::arg_t::tag_t::Immediate,
                                                .data = {.imm = 0x0f1f2f3f4f5f6f7f}
                                        },
                                },
                                {
                                        .tag = mnemo_t::tag_t::Mov,
                                        .width = mnemo_t::width_t::Qword,
                                        .a1 = {
                                                .tag = mnemo_t::arg_t::tag_t::Register,
                                                .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                        },
                                        .a2 = {
                                                .tag = mnemo_t::arg_t::tag_t::Register,
                                                .data = {.reg = mnemo_t::arg_t::reg_t::Rcx}
                                        },
                                },
                                {
                                        .tag = mnemo_t::tag_t::Ret,
                                        .width = mnemo_t::width_t::NotSet,
                                }
                        }
                }},

                // Simple mov test. Second mov superimposes value on a previous value.
                // TODO: expand test to cover every register and register width
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov al, 0
                // ret
                {.name="Byte-wise mov", .expected_result=0x0f0f0f0f0f0f0f00, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x0f0f0f0f0f0f0f0f}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Al}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }
                }},

                // Simple `mov reg, reg` test.
                // TODO: expand test to cover every register and register width
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov cx, 0x0a0a
                // mov dx, cx
                // mov ax, dx
                // ret
                {.name="Byte-wise `mov reg, reg`", .expected_result=0x0f0f0f0f0f0f0a0a, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x0f0f0f0f0f0f0f0f}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Word,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Cx}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x0a0a}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Word,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Dx}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Cx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Word,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Ax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Dx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }
                }},


                // Simple `mov mem reg` test without SIB. Second mov superimposes value on a previous value.
                // Saving rbp is neccesary to use rbp as a stack pointer register which is needed because
                // rsp can not be encoded without SIB.
                // TODO: expand test to cover every register and register width
                // mov rcx, rbp ; save rbp
                // mov rbp, rsp ; enter
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov [rbp - 8], rax
                // mov al, 0
                // mov [rbp - 8], al
                // mov rax, [rbp - 8]
                // mov rbp, rcx ; restore rbp
                // ret
                {.name="`mov mem reg` without SIB", .expected_result=0x0f0f0f0f0f0f0f00, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rcx}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rbp}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rbp}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rsp}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x0f0f0f0f0f0f0f0f}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Rbp, .index=mnemo_t::arg_t::reg_t::Rcx, .scale=mnemo_t::arg_t::memory_t::scale_t::S0, .disp=-8}}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Al}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Rbp, .index=mnemo_t::arg_t::reg_t::Rcx, .scale=mnemo_t::arg_t::memory_t::scale_t::S0, .disp=-8}}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Al}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Rbp, .index=mnemo_t::arg_t::reg_t::Rcx, .scale=mnemo_t::arg_t::memory_t::scale_t::S0, .disp=-8}}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rbp}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rcx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }
                }},


                // Simple `mov mem reg` test with SIB. Second mov superimposes value on a previous value.
                // TODO: expand test to cover every register and register width
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov [rsp - 8], rax
                // mov al, 0
                // mov [rsp - 8], al
                // mov rax, [rsp - 8]
                // ret
                {.name="`mov mem reg` with SIB", .expected_result=0x0f0f0f0f0f0f0f00, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x0f0f0f0f0f0f0f0f}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Rsp, .index=mnemo_t::arg_t::reg_t::Rcx, .scale=mnemo_t::arg_t::memory_t::scale_t::S0, .disp=-8}}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Al}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Rsp, .index=mnemo_t::arg_t::reg_t::Rcx, .scale=mnemo_t::arg_t::memory_t::scale_t::S0, .disp=-8}}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Al}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Rsp, .index=mnemo_t::arg_t::reg_t::Rcx, .scale=mnemo_t::arg_t::memory_t::scale_t::S0, .disp=-8}}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }
                }},


                // `push/pop` test.
                // TODO: expand test to cover every register and register width
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // push rax
                // pop rax
                // ret
                {.name="`push/pop`", .expected_result=0x0f0f0f0f0f0f0f00, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rcx}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x0f0f0f0f0f0f0f00}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Push,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rcx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Pop,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rax}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }
                }},
        };

        u64 success_counter = 0;

        for (const test_t &x:tests) {
            bool success = test_func(x);
            success_counter += success;
        }

        std::cout << "\n";
        std::cout << "Testing complete [" << success_counter << " / " << tests.size() << "].\n";
    }
}