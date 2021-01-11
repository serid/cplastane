#include "assembly_test.hxx"

#include <iostream>

#include "../assembly/assembler.hxx"
#include "../jit/jit.hxx"

using assembly::mnemo_t;

namespace test {
    /// Test options. Combine with bitwise-or
    enum test_opt_t {
        NONE = 0,
        CHECK_BYTECODE = 0b1, // Check assembly result against expected_bytecode.
        RUN = 0b10, // Run assembly result.
        CHECK_EXEC_RESULT = 0b100, // Check result of running against expected_exec_result. Implies RUN.
    };

    struct test_t {
        string name;
        u8 test_opt;
        i64 expected_exec_result;
        vector<u8> expected_bytecode;
        vector<mnemo_t> mnemos;
    };

    auto u64_to_i64(u64 n) -> i64 {
        union U {
            u64 u;
            i64 i;
        };

        U u = { .u=n };
        return u.i;
    }

    static auto test_func(const test_t &x_test) -> bool {
        std::cout << ">> Test \"" << x_test.name << "\".\n";

        vector<u8> bytecode = assemble(x_test.mnemos);

        if ((x_test.test_opt & CHECK_BYTECODE) != 0) {
            if (bytecode == x_test.expected_bytecode) {
                std::cout << "[+] Bytecode check successful.\n";
                std::cout << "Bytecode: [";
                for (const u8 &n:bytecode) {
                    std::cout << std::hex << i32(n >> 4) << i32(n & 0x0f) << std::dec << ", ";
                }
                std::cout << "].\n";
            } else {
                std::cout << "[/] Bytecode check failed.\n";
                std::cout << "Expected bytecode: [";
                for (const u8 &n:x_test.expected_bytecode) {
                    std::cout << std::hex << i32(n >> 4) << i32(n & 0x0f) << std::dec << ", ";
                }
                std::cout << "].\n";

                std::cout << "Actual bytecode: [";
                for (const u8 &n:bytecode) {
                    std::cout << std::hex << i32(n >> 4) << i32(n & 0x0f) << std::dec << ", ";
                }
                std::cout << "].\n";

                return false;
            }
        }

        if ((x_test.test_opt & RUN) != 0) {
            i64 result = jit::eval_mc(bytecode.data(), bytecode.size());

            if ((x_test.test_opt & CHECK_EXEC_RESULT) != 0) {
                if (result == x_test.expected_exec_result) {
                    std::cout << "[+] Result check successful.\n";
                    std::cout << "Result: (" << std::hex << result << std::dec << ").\n";
                    return true;
                } else {
                    std::cout << "[/] Result check failed.\n";
                    std::cout << "Expected result: (" << std::hex << x_test.expected_exec_result << std::dec << ").\n"
                              << "Actual result: (" << std::hex << result << std::dec << ").\n";
                    return false;
                }
            }
        }
        return true;
    }

    auto test_jit() -> void {
        vector<test_t> tests = {
                // mov eax, 100
                // ret
                {.name="Simple `return 100;`", .test_opt=RUN | CHECK_EXEC_RESULT, .expected_exec_result=100, .mnemos={
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
                {.name="`mov reg, reg`", .test_opt=RUN | CHECK_EXEC_RESULT, .expected_exec_result=0xff00, .mnemos={
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
                {.name="`mov qword reg, reg`", .test_opt=RUN |
                                                         CHECK_EXEC_RESULT, .expected_exec_result=0x0f1f2f3f4f5f6f7f, .mnemos={
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
                {.name="`mov byte reg, imm", .test_opt=RUN |
                                                       CHECK_EXEC_RESULT, .expected_exec_result=0x0f0f0f0f0f0f0f00, .mnemos={
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

                // bytecode test
                // mov rcx, 0x0f0f0f0f0f0f0f0f
                // mov ecx, 0x0a0a
                // mov cx, dx
                // mov cl, dl
                {.name="`mov` bytecode test", .test_opt=CHECK_BYTECODE, .expected_bytecode={0x48, 0xb9, 0x0f, 0x0f,
                                                                                            0x0f,
                                                                                            0x0f, 0x0f, 0x0f, 0x0f,
                                                                                            0x0f,
                                                                                            0xb9, 0x0a, 0x0a, 0x00,
                                                                                            0x00,
                                                                                            0x66, 0x89, 0xd1, 0x88,
                                                                                            0xd1}, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rcx}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x0f0f0f0f0f0f0f0f}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Ecx}
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
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Cx}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Dx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Cl}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Dl}
                                },
                        },
                }},

                // Simple `mov mem reg` test without SIB. Second mov superimposes value on a previous value.
                // Saving rbp is neccesary to use rbp as a stack pointer register which is needed because
                // rsp can not be encoded without SIB.
                //
                // mov rcx, rbp ; save rbp
                // mov rbp, rsp ; enter
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov [rbp - 8], rax
                // mov al, 0
                // mov [rbp - 8], al
                // mov rax, [rbp - 8]
                // mov rbp, rcx ; restore rbp
                // ret
                {.name="`mov mem reg` without SIB", .test_opt=RUN |
                                                              CHECK_EXEC_RESULT, .expected_exec_result=0x0f0f0f0f0f0f0f00, .mnemos={
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
                //
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov [rsp - 8], rax
                // mov al, 0
                // mov [rsp - 8], al
                // mov rax, [rsp - 8]
                // ret
                {.name="`mov mem reg` with SIB", .test_opt=RUN |
                                                           CHECK_EXEC_RESULT, .expected_exec_result=0x0f0f0f0f0f0f0f00, .mnemos={
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
                //
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // push rax
                // pop rax
                // ret
                {.name="`push/pop`", .test_opt=RUN |
                                               CHECK_EXEC_RESULT, .expected_exec_result=0x0f0f0f0f0f0f0f00, .mnemos={
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

                // bytecode test
                // push rbx
                // push 100
                // push 1000
                // push [esi+eax*4-10]
                // pop rbx
                // pop [esi+eax*4-10]
                {.name="`push/pop` bytecode multitest", .test_opt=CHECK_BYTECODE, .expected_bytecode={0x53, 0x6a, 0x64,
                                                                                                      0x68,
                                                                                                      0xe8, 0x03, 0x00,
                                                                                                      0x00,
                                                                                                      0x67, 0xff, 0x74,
                                                                                                      0x86,
                                                                                                      0xf6, 0x5b, 0x67,
                                                                                                      0x8f,
                                                                                                      0x44, 0x86,
                                                                                                      0xf6}, .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Push,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rbx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Push,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 100}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Push,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 1000}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Push,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Esi, .index=mnemo_t::arg_t::reg_t::Eax, .scale=mnemo_t::arg_t::memory_t::scale_t::S4, .disp=-10}}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Pop,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Register,
                                        .data = {.reg = mnemo_t::arg_t::reg_t::Rbx}
                                },
                        },
                        {
                                .tag = mnemo_t::tag_t::Pop,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Esi, .index=mnemo_t::arg_t::reg_t::Eax, .scale=mnemo_t::arg_t::memory_t::scale_t::S4, .disp=-10}}
                                },
                        },
                }},

                // mov QWORD [rsp - 8], 0x8f8f8f8f ; will be sign-extended to 64 bits
                // mov BYTE [rsp - 8], 0
                // mov rax, [rsp - 8]
                // ret
                {.name="`mov imm`", .test_opt=RUN |
                                              CHECK_EXEC_RESULT, .expected_exec_result=u64_to_i64(0xffffffff8f8f8f00), .mnemos={
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = {
                                        .tag = mnemo_t::arg_t::tag_t::Memory,
                                        .data = {.memory = {.base=mnemo_t::arg_t::reg_t::Rsp, .index=mnemo_t::arg_t::reg_t::Rcx, .scale=mnemo_t::arg_t::memory_t::scale_t::S0, .disp=-8}}
                                },
                                .a2 = {
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0x8f8f8f8f}
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
                                        .tag = mnemo_t::arg_t::tag_t::Immediate,
                                        .data = {.imm = 0}
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