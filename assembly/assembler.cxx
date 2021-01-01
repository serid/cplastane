#include "assembler.hxx"

#include <iostream>

#include <sys/mman.h>
#include <cstring>

namespace assembly {
    // Converts reg_t to number usable in ModR/M and reg fields of instruction encoding
    auto reg_to_number(mnemo_t::arg_t::reg_t reg) -> u8 {
        switch (reg) {
            case mnemo_t::arg_t::reg_t::Eax:
                return 0b000;
            case mnemo_t::arg_t::reg_t::Ecx:
                return 0b001;
            case mnemo_t::arg_t::reg_t::Edx:
                return 0b010;
            case mnemo_t::arg_t::reg_t::Ebx:
                return 0b011;
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

        switch (mnemo.width) {
            case mnemo_t::width_t::Dword: {
                if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
                    mnemo.a2.tag == mnemo_t::arg_t::tag_t::Register) {
                    // mov r/m32, r32
                    u8 opcode = 0x89;
                    u8 mod = 0b11;
                    u8 rm = reg_to_number(mnemo.a1.data.reg);
                    u8 reg = reg_to_number(mnemo.a2.data.reg);
                    out.push_back(opcode);
                    out.push_back(mod_and_reg_and_rm_to_byte(mod, reg, rm));
                } else if (mnemo.a1.tag == mnemo_t::arg_t::tag_t::Register &&
                           mnemo.a2.tag == mnemo_t::arg_t::tag_t::Immediate) {
                    // mov r32, imm32
                    u8 opcode = 0xB8;
                    opcode += reg_to_number(mnemo.a1.data.reg);
                    out.push_back(opcode);

                    // Write LE i32
                    i32 imm = mnemo.a2.data.imm;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
                    imm >>= 8;
                    out.push_back(imm & 0xFF);
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
                break;
            }
            default:
                throw std::logic_error("Unsupported mov width!");
        }
    }

    auto assemble_mnemo(vector<u8> &out, const mnemo_t &mnemo) -> void {
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
    auto eval_mc(const u8 *mc, size_t len) -> void {
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

        std::cout << func();

        munmap(exec_mc, buf_len);
    }

    void test_jit() {
        vector<mnemo_t> mnemos{};

        mnemo_t x_mnemo{};
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
        x_mnemo = {
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
        };
        mnemos.push_back(x_mnemo);
        x_mnemo = {
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
        };
        mnemos.push_back(x_mnemo);
        x_mnemo = {
                .tag = mnemo_t::tag_t::Ret,
        };
        mnemos.push_back(x_mnemo);

        vector<u8> bytes = assemble(mnemos);

        std::cout << "[";
        for (const u8 &n:bytes) {
            std::cout << std::hex << i32(n) << std::dec << ", ";
        }
        std::cout << "]\n";

        eval_mc(bytes.data(), bytes.size());
    }
}