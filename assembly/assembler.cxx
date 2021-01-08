#include "assembler.hxx"

#include <iostream>

#include <sys/mman.h>
#include <cstring>

namespace assembly {
    // Converts reg_t to number usable in ModR/M and reg fields of instruction encoding
    auto reg_to_number(mnemo_t::arg_t::reg_t reg) -> u8 {
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

    auto mod_and_reg_and_rm_to_byte(u8 mod, u8 reg, u8 rm) -> u8 {
        return (mod << 6) | (reg << 3) | (rm << 0);
    }

    auto assemble_mnemo_mov(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.tag != mnemo_t::tag_t::Mov)
            throw std::logic_error("Wrong mnemo!");

        if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
            mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
            // mov r/m32, r32
            u8 opcode = 0x89;
            u8 mod = 0b11;
            u8 rm = reg_to_number(mnemo.a1.data.reg);
            u8 reg = reg_to_number(mnemo.a2.data.reg);

            // Put a REX prefix if instruction width = qword
            if (mnemo.width == mnemo_t::width_t::Qword) {
                out.push_back(0b01001000);
            }
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

            // Put a width-altering prefix if instruction width = word
            if (mnemo.width == mnemo_t::width_t::Word) {
                out.push_back(0x66);
            }

            // Put a REX prefix if instruction width = qword
            if (mnemo.width == mnemo_t::width_t::Qword) {
                out.push_back(0b01001000);
            }

            out.push_back(opcode);

            // Write imm
            if (mnemo.width == mnemo_t::width_t::Byte) {
                // Write i8
                out.push_back(mnemo.a2.data.imm);
            } else if (mnemo.width == mnemo_t::width_t::Word) {
                // Write LE i16
                i16 imm = mnemo.a2.data.imm;
                out.push_back(imm & 0xFF);
                imm >>= 8;
                out.push_back(imm & 0xFF);
            } else if (mnemo.width == mnemo_t::width_t::Dword) {
                // Write LE i32
                i32 imm = mnemo.a2.data.imm;
                out.push_back(imm & 0xFF);
                imm >>= 8;
                out.push_back(imm & 0xFF);
                imm >>= 8;
                out.push_back(imm & 0xFF);
                imm >>= 8;
                out.push_back(imm & 0xFF);
            } else if (mnemo.width == mnemo_t::width_t::Qword) {
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
            }
        } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Memory &&
                   mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
            // mov r/m32, r32
            u8 opcode = 0x89;
            // TODO: implement mov to memory and from memory
            throw std::exception();
            out.push_back(opcode);
        } else {
            throw std::logic_error("Unsupported mov shape!");
        }
    }

    auto assemble_mnemo(vector<u8> &out, const mnemo_t &mnemo) -> void {
        if (mnemo.width != mnemo_t::width_t::Byte && mnemo.width != mnemo_t::width_t::Word &&
            mnemo.width != mnemo_t::width_t::Dword && mnemo.width != mnemo_t::width_t::Qword &&
            mnemo.width != mnemo_t::width_t::NotSet)
            throw std::logic_error("Unsupported width! assemble_mnemo");
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

    // The function executes the code at pointer mc as if it was a `jit_func_t` function.
    auto eval_mc(const u8 *mc, size_t len) -> i64 {
        // This function places a `ud2` trap after the executable code to catch
        // malicous programs which overflow the buffer and continue executing.
        // Extend buffer to accomodate `ud2` trap.
        size_t buf_len = len + 2;
        void *exec_mc = mmap(nullptr, buf_len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (exec_mc == nullptr) {
            throw std::logic_error("mmap failed");
        }

        std::memcpy(exec_mc, mc, len);

        // Write the `ud2` trap.
        reinterpret_cast<char *>(exec_mc)[buf_len - 2] = 0x0F;
        reinterpret_cast<char *>(exec_mc)[buf_len - 1] = 0x0B;

        auto func = (jit_func_t) exec_mc;

        i64 execution_result = func();

        munmap(exec_mc, buf_len);

        return execution_result;
    }

    auto test_func(const string &name, const vector<mnemo_t> &mnemos, i64 expected_result) -> bool {
        std::cout << ">> Test \"" << name << "\".\n";

        vector<u8> bytes = assemble(mnemos);

        std::cout << "Compilation successful, bytes: ";

        std::cout << "[";
        for (const u8 &n:bytes) {
            std::cout << std::hex << i32(n) << std::dec << ", ";
        }
        std::cout << "]\n";

        i64 result = eval_mc(bytes.data(), bytes.size());
        if (result != expected_result) {
            std::cout << "Test failed.\nExpected result: " << expected_result << "\nActual result: " << result << "\n";
            return false;
        } else {
            std::cout << "Success. result: (" << result << ").\n";
            return true;
        }
    }

    struct test_t {
        string name;
        i64 result;
        vector<mnemo_t> mnemos;
    };

    auto test_jit() -> void {
        vector<test_t> tests = {
                // mov eax, 100
                // ret
                {.name="Simple `return 100;`", .result=100, .mnemos={
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

                /*x_mnemo = {
                        .tag = mnemo_t::tag_t::Mov,
                        .width = mnemo_t::width_t::Dword,
                        .a1 = {
                                .tag = mnemo_t::arg_t::tag_t::Memory,
                                .data = {.memory = {
                                        .base = mnemo_t::arg_t::reg_t::Ecx,
                                        .index = mnemo_t::arg_t::reg_t::Ecx,
                                        .scale = 8,
                                        .disp = 100,
                                }
                                }
                        },
                        .a2 = {
                                .tag = mnemo_t::arg_t::tag_t::Register,
                                .data = {.reg = mnemo_t::arg_t::reg_t::Ecx}
                        },
                };
                mnemos.push_back(x_mnemo);*/

                // mov ecx, 0xff00
                // mov eax, ecx
                // ret
                {.name="Reg mov test", .result=0xff00, .mnemos={
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
                {.name="64 bit reg mov test", .result=0x0f1f2f3f4f5f6f7f, .mnemos={
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
                {.name="Byte-wise mov", .result=0x0f0f0f0f0f0f0f00, .mnemos={
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
        };

        u64 success_counter = 0;

        for (const test_t &x:tests) {
            bool success = test_func(x.name, x.mnemos, x.result);
            success_counter += success;
        }

        std::cout << "\n";
        std::cout << "Testing complete [" << success_counter << " / " << tests.size() << "].\n";
    }
}