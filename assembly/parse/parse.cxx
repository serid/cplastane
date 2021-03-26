#include "parse.hxx"

#include <exception>

#include "../../parsec/parsec.hxx"

// Parse assembly text using parser combinators

using namespace std;
using namespace assembly;
using namespace parsec;

static auto parse_register(string_view tail) -> parser_result<mnemo_t::arg_t::reg_t> {
    return consume_prefix_str(tail, "eax", mnemo_t::arg_t::reg_t::Eax).choice(
            [tail]() -> parser_result<mnemo_t::arg_t::reg_t> {
                return consume_prefix_str(tail, "ebx", mnemo_t::arg_t::reg_t::Ebx);
            }).bind<tuple<string_view, mnemo_t::arg_t::reg_t>>(
            [](tuple<string_view, mnemo_t::arg_t::reg_t> tuple) -> parser_result<mnemo_t::arg_t::reg_t> {
                return make_option(tuple);
            });
}

static auto parse_arg(string_view tail) -> parser_result<mnemo_t::arg_t> {
    // Parses an arg from text assembly like
    // 100
    // eax
    // [eax]


    mnemo_t::arg_t result;
    if (parser_result<i64> result1 = parse_i64(tail)) {
        tail = get<0>(*result1);
        result.tag = mnemo_t::arg_t::tag_t::Immediate;
        result.data.imm = get<1>(*result1);
    } else if (parser_result<mnemo_t::arg_t::reg_t> result2 = parse_register(tail)) {
        tail = get<0>(*result2);
        result.tag = mnemo_t::arg_t::tag_t::Register;
        result.data.reg = get<1>(*result2);
    } else {
        throw logic_error("todo");
    }

    return make_option(make_tuple(tail, result));
}

static auto parse_line(string_view tail) -> parser_result<mnemo_t> {
    // Parses a line from text assembly like
    // mov eax, 100

    mnemo_t result;


    tuple<string_view, string> result1 = scan_while_char(tail, [](char c) { return c != ' '; });
    tail = get<0>(result1);
    string mnemo_name = get<1>(result1);

    mnemo_t::arg_t arg1;

    throw logic_error("todo");

    return make_option(make_tuple(tail, result));
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
                        tail = get<0>(*result1);
                        result.push_back(get<1>(*result1));
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
    }
}
