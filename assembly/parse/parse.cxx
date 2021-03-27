#include "parse.hxx"

#include <exception>

#include "../../parsec/parsec.hxx"

// Parse assembly text using parser combinators

using namespace std;
using namespace assembly;
using namespace parsec;

using arg_t = mnemo_t::arg_t;
using reg_t = arg_t::reg_t;

static auto parse_register(strive tail) -> OptionParserResult<reg_t> {
    return consume_prefix_str(tail, "eax", reg_t::Eax).choice([=]() {
        return consume_prefix_str(tail, "ebx", reg_t::Ebx);
    }).choice([=]() {
        return consume_prefix_str(tail, "ecx", reg_t::Ecx);
    }).choice([=]() {
        return consume_prefix_str(tail, "edx", reg_t::Edx);
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
    // [eax]

    OptionParserResult<arg_t> result =
            parse_i64(tail).bind<ParserResult<arg_t>>([](ParserResult<i64> result1) {
                strive tail2 = result1.tail;
                arg_t result = {
                        .tag = arg_t::tag_t::Immediate,
                        .data = {.imm = result1.data}
                };
                return make_option(ParserResult(tail2, result));
            }).choice([tail]() {
                return parse_register(tail).bind<ParserResult<arg_t>>([](ParserResult<reg_t> result1) {
                    strive tail2 = result1.tail;
                    arg_t result = {
                            .tag = arg_t::tag_t::Register,
                            .data = {.reg = result1.data}
                    };
                    return make_option(ParserResult(tail2, result));
                });
            });

    return result;
}

static auto parse_line(strive tail) -> OptionParserResult<mnemo_t> {
    // Parses a line from text assembly like
    // mov eax, 100

    OptionParserResult<mnemo_t> result = parse_mnemo_name(tail).bind<ParserResult<mnemo_t>>(
            [](ParserResult<mnemo_t::tag_t> result1) {
                strive tail1 = result1.tail;
                mnemo_t::tag_t tag = result1.data;

                // Skip spaces
                ParserResult<monostate> result2 = skip_while_char(tail1, [](char c) { return c == ' '; });
                strive tail2 = result2.tail;

                return parse_mnemo_width(tail2).bind<ParserResult<mnemo_t>>(
                        [=](ParserResult<mnemo_t::width_t> result3) {
                            strive tail3 = result3.tail;
                            mnemo_t::width_t width = result3.data;

                            // Skip spaces
                            ParserResult<monostate> result4 = skip_while_char(tail3, [](char c) { return c == ' '; });
                            strive tail4 = result4.tail;

                            return parse_arg(tail4).bind<ParserResult<mnemo_t>>([=](ParserResult<arg_t> result5) {
                                strive tail5 = result5.tail;
                                arg_t arg1 = result5.data;

                                return consume_prefix_str(tail5, ", ",
                                                          monostate()).bind<ParserResult<mnemo_t>>(
                                        [=](ParserResult<monostate> result6) {
                                            strive tail6 = result6.tail;

                                            return parse_arg(tail6).bind<ParserResult<mnemo_t>>(
                                                    [=](ParserResult<arg_t> result7) {
                                                        strive tail7 = result7.tail;
                                                        arg_t arg2 = result7.data;

                                                        mnemo_t mnemo = {
                                                                .tag = tag,
                                                                .width = width,
                                                                .a1 = arg1,
                                                                .a2 = arg2,
                                                        };

                                                        return make_option(ParserResult(tail7, mnemo));
                                                    });
                                        });
                            });
                        });
            });

    return result;
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
            string s = "mov DWORD eax, ecx\n";
            auto mnemos = assembly::parse::parse(s);

            for (auto &mnemo : mnemos) {
                mnemo.print();
            }
        }
    }
}
