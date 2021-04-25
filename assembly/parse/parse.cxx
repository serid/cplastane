#include "parse.hxx"

// Parse assembly text using parser combinators

using namespace std;
using namespace parsec;
using namespace assembly;
using namespace assembly::parse;

using arg_t = mnemo_t::arg_t;
using reg_t = arg_t::reg_t;

static auto make_error(parsec::strive s, const char *what) -> ParserError {
    return {
            .what=what,
            .where=s,
    };
}

static auto parse_register(strive tail) -> ParserResultResult<reg_t> {
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
        return consume_prefix_str(tail, "esp", reg_t::Esp);
    }).choice([=]() {
        return consume_prefix_str(tail, "ebp", reg_t::Ebp);
    }).choice([=]() {
        return consume_prefix_str(tail, "esi", reg_t::Esi);
    }).choice([=]() {
        return consume_prefix_str(tail, "edi", reg_t::Edi);
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
    }).unwrap_or(make_error(tail, "expected register"));
}

static auto parse_mnemo_name(strive tail) -> ParserResultResult<mnemo_t::tag_t> {
    return consume_prefix_str(tail, "mov", mnemo_t::tag_t::Mov).choice([=]() {
        return consume_prefix_str(tail, "add", mnemo_t::tag_t::Add);
    }).choice([=]() {
        return consume_prefix_str(tail, "push", mnemo_t::tag_t::Push);
    }).choice([=]() {
        return consume_prefix_str(tail, "pop", mnemo_t::tag_t::Pop);
    }).choice([=]() {
        return consume_prefix_str(tail, "ret", mnemo_t::tag_t::Ret);
    }).unwrap_or(make_error(tail, "expected mnemo name"));
}

static auto parse_mnemo_width(strive tail) -> ParserResultResult<mnemo_t::width_t> {
    return consume_prefix_str(tail, "BYTE", mnemo_t::width_t::Byte).choice([=]() {
        return consume_prefix_str(tail, "WORD", mnemo_t::width_t::Word);
    }).choice([=]() {
        return consume_prefix_str(tail, "DWORD", mnemo_t::width_t::Dword);
    }).choice([=]() {
        return consume_prefix_str(tail, "QWORD", mnemo_t::width_t::Qword);
    }).choice([=]() {
        return consume_prefix_str(tail, "NotSet", mnemo_t::width_t::NotSet);
    }).unwrap_or(make_error(tail, "expected instruction size"));
}

static auto parse_arg(strive tail) -> ParserResultResult<arg_t> {
    // Parses an arg from text assembly like
    // 100
    // eax
    // [eax * 2 + 100]
    //🗿
    if (ParserResultResult<i64> a1 = parse_i64(tail).unwrap_or(make_error(tail, "expected int literal"))) {
        return ParserResult(a1.value().tail, arg_t::imm(a1.value().data));
    } else if (ParserResultResult<reg_t> a2 = parse_register(tail)) {
        return ParserResult(a2.value().tail, arg_t::reg(a2.value().data));
    } else if (ParserResultResult<monostate> a = consume_prefix_char(tail, '[')
            .unwrap_or(make_error(tail, "expected a memory argument"))) {
        if (ParserResultResult<reg_t> b = parse_register(a.value().tail)) {
            reg_t base = b.value().data;

            // Maybe parse an index with scale
            reg_t index = reg_t::Undef;
            arg_t::memory_t::scale_t scale = arg_t::memory_t::scale_t::S0;
            if (ParserResultResult<monostate> c = consume_prefix_str(b.value().tail, " + ", monostate())
                    .unwrap_or(make_error(b.value().tail, "todo2"))) {
                if (ParserResultResult<reg_t> d = parse_register(c.value().tail)) {
                    index = d.value().data;
                    b.value().tail = d.value().tail;

                    if (ParserResultResult<monostate> e = consume_prefix_str(d.value().tail, " * ", monostate())
                            .unwrap_or(make_error(d.value().tail, "todo3"))) {
                        if (ParserResultResult<i64> f = parse_i64(e.value().tail)
                                .unwrap_or(make_error(e.value().tail, "expected int literal"))) {
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
                                default:
                                    throw logic_error("scale should be one of: 0 1 2 4 8");
                            }

                            b.value().tail = f.value().tail;
                        }
                    }
                }
            }

            i32 disp = 0;
            if (ParserResultResult<monostate> c = consume_prefix_str(b.value().tail, " + ", monostate())
                    .unwrap_or(make_error(b.value().tail, "todo4"))) {
                if (ParserResultResult<i64> d = parse_i64(c.value().tail)
                        .unwrap_or(make_error(c.value().tail, "todo5"))) {
                    disp = i32(d.value().data);
                    b.value().tail = d.value().tail;
                }
            }

            if (ParserResultResult<monostate> c = consume_prefix_char(b.value().tail, ']')
                    .unwrap_or(make_error(b.value().tail, "expected '['"))) {
                arg_t result = arg_t::mem(base, index, scale, disp);
                return ParserResult(c.value().tail, result);
            } else {
                return c.copy_error();
            }
        } else {
            return b.copy_error();
        }
    } else {
        return a.copy_error();
    }
}

static auto parse_line(strive tail) -> ParserResultResult<mnemo_t> {
    // Parses a line from text assembly like
    // mov eax, 100

    // Parse mnemo tag
    if (ParserResultResult<mnemo_t::tag_t> a = parse_mnemo_name(tail)) {
        mnemo_t::tag_t tag = a.value().data;

        // Skip spaces
        ParserResult<monostate> b = skip_while_char(a.value().tail, [](char c) { return c == ' '; });

        mnemo_t::width_t width = mnemo_t::width_t::NotSet;
        // Parse mnemo width
        if (ParserResultResult<mnemo_t::width_t> c = parse_mnemo_width(b.tail)) {
            width = c.value().data;
            b.tail = c.value().tail;
        }

        // Skip spaces
        ParserResult<monostate> c = skip_while_char(b.tail, [](char c) { return c == ' '; });

        arg_t arg1{};
        arg_t arg2{};
        // Maybe parse arg1
        if (ParserResultResult<mnemo_t::arg_t> d = parse_arg(c.tail)) {
            arg1 = d.value().data;
            c.tail = d.value().tail;

            // Skip ", "
            if (ParserResultResult<monostate> e = consume_prefix_str(d.value().tail, ", ", monostate())
                    .unwrap_or(make_error(d.value().tail, "expected ', '"))) {
                // Maybe parse arg2
                if (ParserResultResult<mnemo_t::arg_t> f = parse_arg(e.value().tail)) {
                    arg2 = f.value().data;
                    c.tail = f.value().tail;
                } else {
                    return f.copy_error();
                }
            }
        }

        // Consume a newline
        if (ParserResultResult<monostate> d = consume_prefix_char(c.tail, '\n')
                .unwrap_or(make_error(c.tail, "expected a newline"))) {
            mnemo_t mnemo = {
                    .tag = tag,
                    .width = width,
                    .a1 = arg1,
                    .a2 = arg2,
            };

            return ParserResult(d.value().tail, mnemo);
        } else {
            return d.copy_error();
        }
    } else {
        return a.copy_error();
    }
}

namespace assembly::parse {
    auto parse(strive tail) -> ParserResultResult<vector<mnemo_t>> {
        // Parses multiline assembly text

        vector<mnemo_t> result{};

        for (; !tail.empty();) {
            if (ParserResultResult<mnemo_t> result1 = parse_line(tail)) {
                // If result is available, continue iteration
                tail = result1.value().tail;
                result.push_back(result1.value().data);
            } else {
                return result1.copy_error();
            }
        }

        return ParserResult(tail, result);
    }

    auto test() -> void {
        string s = "mov DWORD eax, 100\n"
                   "mov BYTE ah, [eax + ebx * 2 + 128]\n"
                   "push DWORD [eax]\n"
                   "ret\n";
        auto mnemos = assembly::parse::parse(s).value().data;

        for (auto &mnemo : mnemos) {
            mnemo.print();
            cout << "\n";
        }
    }
}
