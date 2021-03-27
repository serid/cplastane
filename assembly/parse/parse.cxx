#include "parse.hxx"

#include <exception>

#include "../../parsec/parsec.hxx"

// Parse assembly text using parser combinators

using namespace std;
using namespace assembly;
using namespace parsec;

using arg_t = mnemo_t::arg_t;
using reg_t = arg_t::reg_t;

static auto parse_register(string_view tail) -> parser_result<reg_t> {
    return consume_prefix_str(tail, "eax", reg_t::Eax).choice([=]() -> parser_result<reg_t> {
        return consume_prefix_str(tail, "ebx", reg_t::Ebx);
    }).choice([=]() -> parser_result<reg_t> {
        return consume_prefix_str(tail, "ecx", reg_t::Ecx);
    }).choice([=]() -> parser_result<reg_t> {
        return consume_prefix_str(tail, "edx", reg_t::Edx);
    });
}

static auto parse_mnemo_name(string_view tail) -> parser_result<mnemo_t::tag_t> {
    return consume_prefix_str(tail, "mov", mnemo_t::tag_t::Mov).choice([=]() -> parser_result<mnemo_t::tag_t> {
        return consume_prefix_str(tail, "add", mnemo_t::tag_t::Add);
    }).choice([=]() -> parser_result<mnemo_t::tag_t> {
        return consume_prefix_str(tail, "push", mnemo_t::tag_t::Push);
    }).choice([=]() -> parser_result<mnemo_t::tag_t> {
        return consume_prefix_str(tail, "pop", mnemo_t::tag_t::Pop);
    }).choice([=]() -> parser_result<mnemo_t::tag_t> {
        return consume_prefix_str(tail, "ret", mnemo_t::tag_t::Ret);
    });
}

static auto parse_mnemo_width(string_view tail) -> parser_result<mnemo_t::width_t> {
    return consume_prefix_str(tail, "BYTE", mnemo_t::width_t::Byte).choice([=]() -> parser_result<mnemo_t::width_t> {
        return consume_prefix_str(tail, "WORD", mnemo_t::width_t::Word);
    }).choice([=]() -> parser_result<mnemo_t::width_t> {
        return consume_prefix_str(tail, "DWORD", mnemo_t::width_t::Dword);
    }).choice([=]() -> parser_result<mnemo_t::width_t> {
        return consume_prefix_str(tail, "QWORD", mnemo_t::width_t::Qword);
    }).choice([=]() -> parser_result<mnemo_t::width_t> {
        return consume_prefix_str(tail, "NotSet", mnemo_t::width_t::NotSet);
    });
}

static auto parse_arg(string_view tail) -> parser_result<arg_t> {
    // Parses an arg from text assembly like
    // 100
    // eax
    // [eax]

    parser_result<arg_t> result = parse_i64(tail).bind<tuple<string_view, arg_t>>(
            [](tuple<string_view, i64> result1) -> parser_result<arg_t> {
                string_view tail2 = get<0>(result1);
                arg_t result = {
                        .tag = arg_t::tag_t::Immediate,
                        .data = {.imm = get<1>(result1)}
                };
                return make_option(make_tuple(tail2, result));
            }).choice([tail]() -> parser_result<arg_t> {
        return parse_register(tail).bind<tuple<string_view, arg_t>>(
                [](tuple<string_view, reg_t> result1) -> parser_result<arg_t> {
                    string_view tail2 = get<0>(result1);
                    arg_t result = {
                            .tag = arg_t::tag_t::Register,
                            .data = {.reg = get<1>(result1)}
                    };
                    return make_option(make_tuple(tail2, result));
                });
    });

    return result;
}

static auto parse_line(string_view tail) -> parser_result<mnemo_t> {
    // Parses a line from text assembly like
    // mov eax, 100

    parser_result<mnemo_t> result = parse_mnemo_name(tail).bind<tuple<string_view, mnemo_t>>(
            [](tuple<string_view, mnemo_t::tag_t> result1) -> parser_result<mnemo_t> {
                string_view tail1 = get<0>(result1);
                mnemo_t::tag_t tag = get<1>(result1);

                // Skip spaces
                tuple<string_view, monostate> result2 = skip_while_char(tail1, [](char c) { return c == ' '; });
                string_view tail2 = get<0>(result2);

                return parse_mnemo_width(tail2).bind<tuple<string_view, mnemo_t>>(
                        [=](tuple<string_view, mnemo_t::width_t> result3) -> parser_result<mnemo_t> {
                            string_view tail3 = get<0>(result3);
                            mnemo_t::width_t width = get<1>(result3);

                            // Skip spaces
                            tuple<string_view, monostate> result4 = skip_while_char(tail3,
                                                                                    [](char c) { return c == ' '; });
                            string_view tail4 = get<0>(result4);

                            return parse_arg(tail4).bind<tuple<string_view, mnemo_t>>(
                                    [=](tuple<string_view, arg_t> result5) -> parser_result<mnemo_t> {
                                        string_view tail5 = get<0>(result5);
                                        arg_t arg1 = get<1>(result5);

                                        return consume_prefix_str(tail5, ", ",
                                                                  monostate()).bind<tuple<string_view, mnemo_t>>(
                                                [=](tuple<string_view, monostate> result6) -> parser_result<mnemo_t> {
                                                    string_view tail6 = get<0>(result6);

                                                    return parse_arg(tail6).bind<tuple<string_view, mnemo_t>>(
                                                            [=](tuple<string_view, arg_t> result7) -> parser_result<mnemo_t> {
                                                                string_view tail7 = get<0>(result7);
                                                                arg_t arg2 = get<1>(result7);

                                                                mnemo_t mnemo = {
                                                                        .tag = tag,
                                                                        .width = width,
                                                                        .a1 = arg1,
                                                                        .a2 = arg2,
                                                                };

                                                                return make_option(make_tuple(tail7, mnemo));
                                                            });
                                                });
                                    });
                        });
            });

    return result;
}

namespace assembly {
    namespace parse {
        auto parse(string_view tail) -> vector<mnemo_t> {
            // Parses multiline assembly text

            vector<mnemo_t> result{};

            for (;;) {
                if (parser_result<mnemo_t> result1 = parse_line(tail)) {
                    // If result is available, continue iteration
                    tail = get<0>(*result1);
                    result.push_back(get<1>(*result1));

                    // Consume a newline
                    if (parser_result<monostate> result2 = consume_prefix_char(tail, '\n', monostate())) {
                        // If result is available, continue iteration
                        tail = get<0>(*result2);
                    } else {
                        // Newline not found
                        throw logic_error("Uh oh");
                    };
                } else {
                    // Else break
                    break;
                };
            }

            return result;
        }

        auto test() -> void {
            string s = "mov DWORD eax, ecx\n";
            auto mnemos = assembly::parse::parse(s);

            for (auto &mnemo : mnemos) {
                mnemo.print();
            }
        }
    }
}
