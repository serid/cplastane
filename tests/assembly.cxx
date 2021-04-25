#include "assembly.hxx"

#include <iostream>

#include "../assembly/assembly.hxx"
#include "../jit/jit.hxx"
#include "../test/test.hxx"
#include "../assembly/parse/parse.hxx"

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
        auto process = [](const vector<mnemo_t> &mnemos) -> vector<u8> {
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
                new bytecode_test("`mov` bytecode test",
                                  move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                          "mov QWORD rcx, 0x0F0F0F0F0F0F0F0F\n"
                                          "mov DWORD ecx, 0x0A0A\n"
                                          "mov WORD cx, dx\n"
                                          "mov BYTE cl, dl\n"
                                          "mov QWORD [rax + rax * 1], 1\n")).data),
                                  {0x48, 0xb9, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xb9, 0x0a, 0x0a, 0x00,
                                   0x00, 0x66, 0x89, 0xd1, 0x88, 0xd1, 0x48, 0xc7, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00},
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
                new bytecode_test("`add` bytecode test",
                                  move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                          "add DWORD ebx, ecx\n"
                                          "add DWORD eax, 100\n"
                                          "add DWORD ebx, 100\n"
                                          "add DWORD ebx, 10000\n"
                                          "add DWORD ebx, [rsi]\n"
                                          "add DWORD [rsi], ebx\n"
                                          "add BYTE [rsi], 100\n"
                                          "add WORD [rsi], 10000\n")).data),
                                  {0x01, 0xcb, 0x83, 0xc0, 0x64, 0x83, 0xc3, 0x64, 0x81, 0xc3, 0x10, 0x27, 0x00, 0x00,
                                   0x03, 0x1e, 0x01, 0x1e, 0x80, 0x06, 0x64, 0x66, 0x81, 0x06, 0x10, 0x27},
                                  process, output_printer
                ),

                // bytecode test
                // mov rcx, 0x0f0f0f0f0f0f0f0f
                // mov ecx, 0x0a0a
                // mov cx, dx
                // mov cl, dl
                // mov qword [rax + rax * 1], 1
                new bytecode_test("`mov` bytecode test",
                                  move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                          "mov QWORD rcx, 0x0F0F0F0F0F0F0F0F\n"
                                          "mov DWORD ecx, 0x0A0A\n"
                                          "mov WORD cx, dx\n"
                                          "mov BYTE cl, dl\n"
                                          "mov QWORD [rax + rax * 1], 1\n")).data),
                                  {0x48, 0xb9, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xb9, 0x0a, 0x0a, 0x00,
                                   0x00, 0x66, 0x89, 0xd1, 0x88, 0xd1, 0x48, 0xc7, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00},
                                  process, output_printer
                ),

                // bytecode test
                // push rbx
                // push 100
                // push 1000
                // push [esi+eax*4-10]
                // pop rbx
                // pop [esi+eax*4-10]
                new bytecode_test("`push/pop` bytecode multitest",
                                  move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                          "push QWORD rbx\n"
                                          "push BYTE 100\n"
                                          "push DWORD 1000\n"
                                          "push QWORD [esi + eax * 4 + -10]\n"
                                          "pop QWORD rbx\n"
                                          "pop QWORD [esi + eax * 4 + -10]\n")).data),
                                  {0x53, 0x6a, 0x64, 0x68, 0xe8, 0x03, 0x00, 0x00, 0x67, 0xff, 0x74, 0x86, 0xf6, 0x5b,
                                   0x67, 0x8f, 0x44, 0x86, 0xf6},
                                  process, output_printer
                ),
        };

        return test::run_test_group(tests);
    }

    // Test group of tests that execute bytecode
    static auto run_exec_tests() -> test::TestGroupResult {
        auto process = [](const vector<mnemo_t> &mnemos) -> i64 {
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
                new exec_test("Simple `return 100;`",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov DWORD eax, 100\n"
                                      "ret\n")).data), 100, process, output_printer),

                // mov ecx, 0xff00
                // mov eax, ecx
                // ret
                new exec_test("`mov reg, reg`",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov DWORD ecx, 0xFF00\n"
                                      "mov DWORD eax, ecx\n"
                                      "ret\n")).data), 0xff00, process, output_printer),

                // mov rcx, 0x0f1f2f3f4f5f6f7f
                // mov rax, rcx
                // ret
                new exec_test("`mov qword reg, reg`",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov QWORD rcx, 0x0F1F2F3F4F5F6F7F\n"
                                      "mov QWORD rax, rcx\n"
                                      "ret\n")).data), 0x0f1f2f3f4f5f6f7f, process, output_printer),

                // Simple mov test. Second mov superimposes value on a previous value.
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov al, 0
                // ret
                new exec_test("`mov byte reg, imm",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov QWORD rax, 0x0F0F0F0F0F0F0F0F\n"
                                      "mov BYTE al, 0\n"
                                      "ret\n")).data), 0x0f0f0f0f0f0f0f00, process, output_printer),

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
                new exec_test("`mov mem reg` without SIB",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov QWORD rcx, rbp\n"
                                      "mov QWORD rbp, rsp\n"
                                      "mov QWORD rax, 0x0F0F0F0F0F0F0F0F\n"
                                      "mov QWORD [rbp + -8], rax\n"
                                      "mov BYTE al, 0\n"
                                      "mov BYTE [rbp + -8], al\n"
                                      "mov QWORD rax, [rbp + -8]\n"
                                      "mov QWORD rbp, rcx\n"
                                      "ret\n")).data), 0x0f0f0f0f0f0f0f00, process, output_printer),

                // Simple `mov mem reg` test with SIB. Second mov superimposes value on a previous value.
                //
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // mov [rsp - 8], rax
                // mov al, 0
                // mov [rsp - 8], al
                // mov rax, [rsp - 8]
                // ret
                new exec_test("`mov mem reg` with SIB",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov QWORD rax, 0x0F0F0F0F0F0F0F0F\n"
                                      "mov QWORD [rsp + -8], rax\n"
                                      "mov BYTE al, 0\n"
                                      "mov BYTE [rsp + -8], al\n"
                                      "mov QWORD rax, [rsp + -8]\n"
                                      "ret\n")).data), 0x0f0f0f0f0f0f0f00, process, output_printer),

                // `push/pop` test.
                //
                // mov rax, 0x0f0f0f0f0f0f0f0f
                // push rax
                // mov rax, -1
                // pop rax
                // ret
                new exec_test("`push/pop`",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov QWORD rax, 0x0F0F0F0F0F0F0F0F\n"
                                      "push QWORD rax\n"
                                      "mov QWORD rax, -1\n"
                                      "pop QWORD rax\n"
                                      "ret\n")).data), 0x0f0f0f0f0f0f0f0f, process, output_printer),

                // mov QWORD [rsp - 8], 0x8f8f8f8f ; will be sign-extended to 64 bits
                // mov BYTE [rsp - 8], 0
                // mov rax, [rsp - 8]
                // ret
                new exec_test("`mov imm`",
                              move(assembly::parse::unwrap_or_log_error(assembly::parse::parse(
                                      "mov QWORD [rsp + -8], 0x8F8F8F8F\n"
                                      "mov BYTE [rsp + -8], 0\n"
                                      "mov QWORD rax, [rsp + -8]\n"
                                      "ret\n")).data), u64_to_i64(0xffffffff8f8f8f00), process, output_printer),
        };

        auto results = test::run_test_group(tests);

        for (auto &test : tests) {
            delete test;
        }

        return results;
    }

    auto test_assembly() -> void {
        test::log_combine_test_groups_results<2>({run_bytecode_tests(), run_exec_tests()});
    }
}