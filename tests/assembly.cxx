#include "assembly.hxx"

#include <iostream>

#include "../assembly/assembly.hxx"
#include "../jit/jit.hxx"
#include "../test/test.hxx"

using namespace std;

using assembly::mnemo_t;

namespace tests {
    static auto u64_to_i64(u64 n) -> i64 {
        union U {
            u64 u;
            i64 i;
        };

        U u = {.u=n};
        return u.i;
    }

    // Test group of tests that output bytecode
    static auto run_bytecode_tests() -> test::TestGroupResult {
        auto process = [](vector<mnemo_t> mnemos) -> vector<u8> {
            return assembly::assemble(mnemos);
        };
        auto output_printer = [](const vector<u8> &vec) -> void {
            cout << "[";
            for (const u8 &n : vec) {
                cout << hex << i32(n >> 4) << i32(n & 0x0f) << dec << ", ";
            }
            cout << "]";
        };

        using bytecode_test = test::Test<vector<mnemo_t>, vector<u8>>;

        test::TestGroup tests = {
                // bytecode test
                // mov rcx, 0x0f0f0f0f0f0f0f0f
                // mov ecx, 0x0a0a
                // mov cx, dx
                // mov cl, dl
                // mov qword [rax + rax * 1], 1
                new bytecode_test("`mov` bytecode test", {
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                                                  .a2 = mnemo_t::arg_t::imm(0x0f0f0f0f0f0f0f0f),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ecx),
                                                  .a2 = mnemo_t::arg_t::imm(0x0a0a),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Word,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Cx),
                                                  .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Dx),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Byte,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Cl),
                                                  .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Dl),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Rax,
                                                                            mnemo_t::arg_t::reg_t::Rax,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S1, 0),
                                                  .a2 = mnemo_t::arg_t::imm(1),
                                          },
                                  },
                                  {0x48, 0xb9, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xb9, 0x0a, 0x0a, 0x00,
                                   0x00,
                                   0x66, 0x89, 0xd1, 0x88, 0xd1, 0x48, 0xc7, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00},
                                  process, output_printer
                ),

                // bytecode test
                // add ebx, ecx
                // add eax, 100
                // add ebx, 100
                // add ebx, 10000
                // add ebx, [rsi]
                // add [rsi], ebx
                // add [rsi], 100
                // add [rsi], 10000
                new bytecode_test("`add` bytecode test", {
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ebx),
                                                  .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ecx),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Eax),
                                                  .a2 = mnemo_t::arg_t::imm(100),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ebx),
                                                  .a2 = mnemo_t::arg_t::imm(100),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ebx),
                                                  .a2 = mnemo_t::arg_t::imm(10000),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ebx),
                                                  .a2 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Rsi,
                                                                            mnemo_t::arg_t::reg_t::Rcx,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S0, 0),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Rsi,
                                                                            mnemo_t::arg_t::reg_t::Rcx,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S0, 0),
                                                  .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ebx),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Byte,
                                                  .a1 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Rsi,
                                                                            mnemo_t::arg_t::reg_t::Rcx,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S0, 0),
                                                  .a2 = mnemo_t::arg_t::imm(100),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Add,
                                                  .width = mnemo_t::width_t::Word,
                                                  .a1 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Rsi,
                                                                            mnemo_t::arg_t::reg_t::Rcx,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S0, 0),
                                                  .a2 = mnemo_t::arg_t::imm(10000),
                                          },
                                  },
                                  {0x01, 0xcb, 0x83, 0xc0, 0x64, 0x83, 0xc3, 0x64, 0x81, 0xc3, 0x10, 0x27, 0x00, 0x00,
                                   0x03,
                                   0x1e, 0x01, 0x1e, 0x80, 0x06, 0x64, 0x66, 0x81, 0x06, 0x10, 0x27},
                                  process, output_printer
                ),

                // bytecode test
                // mov rcx, 0x0f0f0f0f0f0f0f0f
                // mov ecx, 0x0a0a
                // mov cx, dx
                // mov cl, dl
                // mov qword [rax + rax * 1], 1
                new bytecode_test("`mov` bytecode test", {
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                                                  .a2 = mnemo_t::arg_t::imm(0x0f0f0f0f0f0f0f0f),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ecx),
                                                  .a2 = mnemo_t::arg_t::imm(0x0a0a),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Word,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Cx),
                                                  .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Dx),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Byte,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Cl),
                                                  .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Dl),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Mov,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Rax,
                                                                            mnemo_t::arg_t::reg_t::Rax,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S1, 0),
                                                  .a2 = mnemo_t::arg_t::imm(1),
                                          },
                                  }, {0x48, 0xb9, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xb9, 0x0a, 0x0a, 0x00, 0x00, 0x66,
                                      0x89, 0xd1, 0x88, 0xd1, 0x48, 0xc7, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00},
                                  process, output_printer
                ),

                // bytecode test
                // push rbx
                // push 100
                // push 1000
                // push [esi+eax*4-10]
                // pop rbx
                // pop [esi+eax*4-10]
                new bytecode_test("`push/pop` bytecode multitest", {
                                          {
                                                  .tag = mnemo_t::tag_t::Push,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rbx),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Push,
                                                  .width = mnemo_t::width_t::Byte,
                                                  .a1 = mnemo_t::arg_t::imm(100),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Push,
                                                  .width = mnemo_t::width_t::Dword,
                                                  .a1 = mnemo_t::arg_t::imm(1000),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Push,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Esi,
                                                                            mnemo_t::arg_t::reg_t::Eax,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S4, -10),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Pop,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rbx),
                                          },
                                          {
                                                  .tag = mnemo_t::tag_t::Pop,
                                                  .width = mnemo_t::width_t::Qword,
                                                  .a1 = mnemo_t::arg_t::mem(mnemo_t::arg_t::reg_t::Esi,
                                                                            mnemo_t::arg_t::reg_t::Eax,
                                                                            mnemo_t::arg_t::memory_t::scale_t::S4, -10),
                                          },
                                  }, {0x53, 0x6a, 0x64, 0x68, 0xe8, 0x03, 0x00, 0x00, 0x67, 0xff, 0x74, 0x86, 0xf6, 0x5b, 0x67, 0x8f,
                                      0x44, 0x86, 0xf6},
                                  process, output_printer
                ),
        };

        return test::run_test_group(tests);
    }

    // Test group of tests that execute bytecode
    static auto run_exec_tests() -> test::TestGroupResult {
        auto process = [](vector<mnemo_t> mnemos) -> i64 {
            vector<u8> bytecode = assembly::assemble(mnemos);
            return jit::eval_mc(bytecode.data(), bytecode.size());
        };
        auto output_printer = [](const i64 &num1) -> void {
            cout << "(" << num1 << ")";
        };

        using exec_test = test::Test<vector<mnemo_t>, i64>;

        test::TestGroup tests = {
                // mov eax, 100
                // ret
                new exec_test("Simple `return 100;`", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Eax),
                                .a2 = mnemo_t::arg_t::imm(100),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        },
                }, 100, process, output_printer),

                // mov ecx, 0xff00
                // mov eax, ecx
                // ret
                new exec_test("`mov reg, reg`", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ecx),
                                .a2 = mnemo_t::arg_t::imm(0xff00),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Dword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Eax),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Ecx),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        },
                }, 0xff00, process, output_printer),

                // mov rcx, 0x0f1f2f3f4f5f6f7f
                // mov rax, rcx
                // ret
                new exec_test("`mov qword reg, reg`", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                                .a2 = mnemo_t::arg_t::imm(0x0f1f2f3f4f5f6f7f),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        },
                }, 0x0f1f2f3f4f5f6f7f, process, output_printer),

                // Simple mov test. Second mov superimposes value on a previous value.
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov al, 0
                // ret
                new exec_test("`mov byte reg, imm", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                                .a2 = mnemo_t::arg_t::imm(0x0f0f0f0f0f0f0f0f),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Al),
                                .a2 = mnemo_t::arg_t::imm(0),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        }
                }, 0x0f0f0f0f0f0f0f00, process, output_printer),

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
                new exec_test("`mov mem reg` without SIB", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rbp),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rbp),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rsp),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                                .a2 = mnemo_t::arg_t::imm(0x0f0f0f0f0f0f0f0f),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rbp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Al),
                                .a2 = mnemo_t::arg_t::imm(0),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rbp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Al),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                                .a2 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rbp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rbp),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        },
                }, 0x0f0f0f0f0f0f0f00, process, output_printer),

                // Simple `mov mem reg` test with SIB. Second mov superimposes value on a previous value.
                //
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov [rsp - 8], rax
                // mov al, 0
                // mov [rsp - 8], al
                // mov rax, [rsp - 8]
                // ret
                new exec_test("`mov mem reg` with SIB", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                                .a2 = mnemo_t::arg_t::imm(0x0f0f0f0f0f0f0f0f),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rsp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Al),
                                .a2 = mnemo_t::arg_t::imm(0),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rsp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                                .a2 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Al),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                                .a2 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rsp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        },
                }, 0x0f0f0f0f0f0f0f00, process, output_printer),

                // `push/pop` test.
                //
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // push rax
                // pop rax
                // ret
                new exec_test("`push/pop`", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                                .a2 = mnemo_t::arg_t::imm(0x0f0f0f0f0f0f0f00),
                        },
                        {
                                .tag = mnemo_t::tag_t::Push,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rcx),
                        },
                        {
                                .tag = mnemo_t::tag_t::Pop,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        },
                }, 0x0f0f0f0f0f0f0f00, process, output_printer),

                // mov QWORD [rsp - 8], 0x8f8f8f8f ; will be sign-extended to 64 bits
                // mov BYTE [rsp - 8], 0
                // mov rax, [rsp - 8]
                // ret
                new exec_test("`mov imm`", {
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rsp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                                .a2 = mnemo_t::arg_t::imm(0x8f8f8f8f),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Byte,
                                .a1 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rsp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                                .a2 = mnemo_t::arg_t::imm(0),
                        },
                        {
                                .tag = mnemo_t::tag_t::Mov,
                                .width = mnemo_t::width_t::Qword,
                                .a1 = mnemo_t::arg_t::reg(mnemo_t::arg_t::reg_t::Rax),
                                .a2 = mnemo_t::arg_t::mem(
                                        mnemo_t::arg_t::reg_t::Rsp,
                                        mnemo_t::arg_t::reg_t::Rcx,
                                        mnemo_t::arg_t::memory_t::scale_t::S0,
                                        -8
                                ),
                        },
                        {
                                .tag = mnemo_t::tag_t::Ret,
                                .width = mnemo_t::width_t::NotSet,
                        },
                }, u64_to_i64(0xffffffff8f8f8f00), process, output_printer),
        };

        return test::run_test_group(tests);
    }

    auto test_assembly() -> void {
        test::log_combine_test_groups_results<2>({run_bytecode_tests(), run_exec_tests()});
    }
}