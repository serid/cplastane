#include "parse.hxx"

#include <exception>
#include <iostream>

#include "../../parsec/parsec.hxx"

// Parse assembly text using parser combinators

using namespace std;
using namespace assembly;
using namespace parsec;

using arg_t = mnemo_t::arg_t;
using reg_t = arg_t::reg_t;

static auto parse_register(strive tail) -> OptionParserResult<reg_t> {
    return OptionParserResult<reg_t>().choice([=]() {
        return consume_prefix_str(tail, "al", reg_t::Al);
    }).choice([=]() {
        return consume_prefix_str(tail, "bl", reg_t::Bl);
    }).choice([=]() {
        return consume_prefix_str(tail, "cl", reg_t::Cl);
    }).choice([=]() {
        return consume_prefix_str(tail, "dl", reg_t::Dl);
    }).choice([=]() {
        return consume_prefix_str(tail, "ah", reg_t::Ah);
    }).choice([=]() {
        return consume_prefix_str(tail, "bh", reg_t::Bh);
    }).choice([=]() {
        return consume_prefix_str(tail, "ch", reg_t::Ch);
    }).choice([=]() {
        return consume_prefix_str(tail, "dh", reg_t::Dh);
    }).choice([=]() {
        return consume_prefix_str(tail, "ax", reg_t::Ax);
    }).choice([=]() {
        return consume_prefix_str(tail, "bx", reg_t::Bx);
    }).choice([=]() {
        return consume_prefix_str(tail, "cx", reg_t::Cx);
    }).choice([=]() {
        return consume_prefix_str(tail, "dx", reg_t::Dx);
    }).choice([=]() {
        return consume_prefix_str(tail, "sp", reg_t::Sp);
    }).choice([=]() {
        return consume_prefix_str(tail, "bp", reg_t::Bp);
    }).choice([=]() {
        return consume_prefix_str(tail, "si", reg_t::Si);
    }).choice([=]() {
        return consume_prefix_str(tail, "di", reg_t::Di);
    }).choice([=]() {
        return consume_prefix_str(tail, "eax", reg_t::Eax);
    }).choice([=]() {
        return consume_prefix_str(tail, "ebx", reg_t::Ebx);
    }).choice([=]() {
        return consume_prefix_str(tail, "ecx", reg_t::Ecx);
    }).choice([=]() {
        return consume_prefix_str(tail, "edx", reg_t::Edx);
    }).choice([=]() {
        return consume_prefix_str(tail, "rax", reg_t::Rax);
    }).choice([=]() {
        return consume_prefix_str(tail, "rbx", reg_t::Rbx);
    }).choice([=]() {
        return consume_prefix_str(tail, "rcx", reg_t::Rcx);
    }).choice([=]() {
        return consume_prefix_str(tail, "rdx", reg_t::Rdx);
    }).choice([=]() {
        return consume_prefix_str(tail, "rsp", reg_t::Rsp);
    }).choice([=]() {
        return consume_prefix_str(tail, "rbp", reg_t::Rbp);
    }).choice([=]() {
        return consume_prefix_str(tail, "rsi", reg_t::Rsi);
    }).choice([=]() {
        return consume_prefix_str(tail, "rdi", reg_t::Rdi);
    });
}

static auto parse_mnemo_name(strive tail) -> OptionParserResult<mnemo_t::tag_t> {
    return consume_prefix_str(tail, "mov", mnemo_t::tag_t::Mov).choice([=]() {
        return consume_prefix_str(tail, "add", mnemo_t::tag_t::Add);
    }).choice([=]() {
        return consume_prefix_str(tail, "push", mnemo_t::tag_t::Push);
    }).choice([=]() {
        return consume_prefix_str(tail, "pop", mnemo_t::tag_t::Pop);
    }).choice([=]() {
        return consume_prefix_str(tail, "ret", mnemo_t::tag_t::Ret);
    });
}

static auto parse_mnemo_width(strive tail) -> OptionParserResult<mnemo_t::width_t> {
    return consume_prefix_str(tail, "BYTE", mnemo_t::width_t::Byte).choice([=]() {
        return consume_prefix_str(tail, "WORD", mnemo_t::width_t::Word);
    }).choice([=]() {
        return consume_prefix_str(tail, "DWORD", mnemo_t::width_t::Dword);
    }).choice([=]() {
        return consume_prefix_str(tail, "QWORD", mnemo_t::width_t::Qword);
    }).choice([=]() {
        return consume_prefix_str(tail, "NotSet", mnemo_t::width_t::NotSet);
    });
}

static auto parse_arg(strive tail) -> OptionParserResult<arg_t> {
    // Parses an arg from text assembly like
    // 100
    // eax
    // [eax * 2 + 100]

    OptionParserResult<arg_t> result =
            parse_i64(tail).bind<ParserResult<arg_t>>([](ParserResult<i64> result1) {
                strive tail1 = result1.tail;
                arg_t result = arg_t::imm(result1.data);
                return make_option(ParserResult(tail1, result));
            }).choice([tail]() {
                return parse_register(tail).bind<ParserResult<arg_t>>([](ParserResult<reg_t> result1) {
                    strive tail1 = result1.tail;
                    arg_t result = arg_t::reg(result1.data);
                    return make_option(ParserResult(tail1, result));
                });
            }).choice([tail]() {
                /* // A (failed) attempt to write parsing code using proper bindings
                return consume_prefix_char<monostate>(tail, '[', monostate()).bind<ParserResult<arg_t>>(
                        [](ParserResult<monostate> result1) {
                            strive tail1 = result1.tail;
                            return parse_register(tail1).bind<ParserResult<arg_t>>([](ParserResult<reg_t> result2) {
                                strive tail2 = result2.tail;
                                reg_t base = result2.data;
                                reg_t index = result2.data;

                                arg_t result = {
                                        .tag = arg_t::tag_t::Register,
                                        .data = {.reg = result2.data}
                                };
                                return make_option(ParserResult(tail2, result));
                            });
                        });
                */

                // Parse a memory operand
                if (OptionParserResult<monostate> a = consume_prefix_char(tail, '[', monostate())) {
                    if (OptionParserResult<reg_t> b = parse_register(a.value().tail)) {
                        reg_t base = b.value().data;

                        // Maybe parse an index with scale
                        reg_t index = reg_t::Undef;
                        arg_t::memory_t::scale_t scale = arg_t::memory_t::scale_t::S0;
                        if (OptionParserResult<monostate> c = consume_prefix_str(b.value().tail, " + ", monostate())) {
                            if (OptionParserResult<reg_t> d = parse_register(c.value().tail)) {
                                index = d.value().data;
                                b.value().tail = d.value().tail;

                                if (OptionParserResult<monostate> e = consume_prefix_str(d.value().tail, " * ",
                                                                                         monostate())) {
                                    if (OptionParserResult<i64> f = parse_i64(e.value().tail)) {
                                        switch (f.value().data) {
                                            case 0:
                                                scale = arg_t::memory_t::scale_t::S0;
                                                break;
                                            case 1:
                                                scale = arg_t::memory_t::scale_t::S1;
                                                break;
                                            case 2:
                                                scale = arg_t::memory_t::scale_t::S2;
                                                break;
                                            case 4:
                                                scale = arg_t::memory_t::scale_t::S4;
                                                break;
                                            case 8:
                                                scale = arg_t::memory_t::scale_t::S8;
                                                break;
                                        }

                                        b.value().tail = f.value().tail;
                                    }
                                }
                            }
                        }

                        i32 disp = 0;
                        if (OptionParserResult<monostate> c = consume_prefix_str(b.value().tail, " + ", monostate())) {
                            if (OptionParserResult<i64> d = parse_i64(c.value().tail)) {
                                disp = i32(d.value().data);
                                b.value().tail = d.value().tail;
                            }
                        }

                        if (OptionParserResult<monostate> c = consume_prefix_char(b.value().tail, ']', monostate())) {
                            arg_t result = arg_t::mem(base, index, scale, disp);
                            return make_option(ParserResult(c.value().tail, result));
                        } else {
                            return OptionParserResult<arg_t>();
                        }
                    } else {
                        return OptionParserResult<arg_t>();
                    }
                } else {
                    return OptionParserResult<arg_t>();
                }
            });

    return result;
}

static auto parse_line(strive tail) -> OptionParserResult<mnemo_t> {
    // Parses a line from text assembly like
    // mov eax, 100

    // Parse mnemo tag
    if (OptionParserResult<mnemo_t::tag_t> a = parse_mnemo_name(tail)) {
        mnemo_t::tag_t tag = a.value().data;

        // Skip spaces
        ParserResult<monostate> b = skip_while_char(a.value().tail, [](char c) { return c == ' '; });

        // Parse mnemo width
        if (OptionParserResult<mnemo_t::width_t> c = parse_mnemo_width(b.tail)) {
            mnemo_t::width_t width = c.value().data;

            // Skip spaces
            ParserResult<monostate> d = skip_while_char(c.value().tail, [](char c) { return c == ' '; });

            arg_t arg1{};
            // Maybe parse arg1
            if (OptionParserResult<mnemo_t::arg_t> e = parse_arg(d.tail)) {
                arg1 = e.value().data;

                // Skip ", "
                if (OptionParserResult<monostate> f = consume_prefix_str(e.value().tail, ", ", monostate())) {
                    arg_t arg2{};
                    // Maybe parse arg2
                    if (OptionParserResult<mnemo_t::arg_t> g = parse_arg(f.value().tail)) {
                        arg2 = g.value().data;

                        // todo: consume \n

                        mnemo_t mnemo = {
                                .tag = tag,
                                .width = width,
                                .a1 = arg1,
                                .a2 = arg2,
                        };

                        return make_option(ParserResult(g.value().tail, mnemo));
                    } else {
                        return OptionParserResult<mnemo_t>();
                    }
                } else {
                    return OptionParserResult<mnemo_t>();
                }
            } else {
                return OptionParserResult<mnemo_t>();
            }
        } else {
            return OptionParserResult<mnemo_t>();
        }
    } else {
        return OptionParserResult<mnemo_t>();
    }
}

namespace assembly {
    namespace parse {
        auto parse(strive tail) -> vector<mnemo_t> {
            // Parses multiline assembly text

            vector<mnemo_t> result{};

            for (;;) {
                if (OptionParserResult<mnemo_t> result1 = parse_line(tail)) {
                    // If result is available, continue iteration
                    tail = result1.value().tail;
                    result.push_back(result1.value().data);

                    // Consume a newline
                    if (OptionParserResult<monostate> result2 = consume_prefix_char(tail, '\n', monostate())) {
                        // If result is available, continue iteration
                        tail = result2.value().tail;
                    } else {
                        // Newline not found
                        throw logic_error("Uh oh");
                    }
                } else {
                    // Else break
                    break;
                }
            }

            return result;
        }

        auto test() -> void {
            string s = "mov DWORD eax, 100\n"
                       "mov BYTE ah, [eax + ebx * 2 + 128]\n";
            auto mnemos = assembly::parse::parse(s);

            for (auto &mnemo : mnemos) {
                mnemo.print();
                cout << "\n";
            }
        }
    }
}
