#include "parse.hxx"

#include <exception>

#include "../../parsec/parsec.hxx"

// Parse assembly text using parser combinators

using namespace assembly;
using namespace parsec;

static auto parse_register(std::string_view tail) -> parser_result<mnemo_t::arg_t::reg_t> {
    mnemo_t::arg_t::reg_t result;
    /*
#define test_prefix_combinator(id, prefix, value)\
            if (std::optional<std::tuple<std::string_view, std::monostate>> result##id = consume_prefix_str(tail, prefix)) {\
                tail = std::get<0>(*result##id);\
                result = value;\
            } else
*/

    vector<parser_type<mnemo_t::arg_t::reg_t, std::monostate>> funs{
            [](std::string_view tail, std::monostate) -> parser_result<mnemo_t::arg_t::reg_t> {
                return consume_prefix_str(tail, "eax", mnemo_t::arg_t::reg_t::Eax);
            },
            [](std::string_view tail, std::monostate) -> parser_result<mnemo_t::arg_t::reg_t> {
                return consume_prefix_str(tail, "ebx", mnemo_t::arg_t::reg_t::Ebx);
            },
    };

    if (parser_result<mnemo_t::arg_t::reg_t> result1 = choice_combinator(tail, funs)) {
        tail = std::get<0>(*result1);
        result = std::get<1>(*result1);
    } else {
        return parser_result<mnemo_t::arg_t::reg_t>();
    }

    return make_option(std::make_tuple(tail, result));
}

static auto parse_arg(std::string_view tail) -> parser_result<mnemo_t::arg_t> {
    // Parses an arg from text assembly like
    // 100
    // eax
    // [eax]

    
    mnemo_t::arg_t result;
    if (parser_result<i64> result1 = parse_i64(tail)) {
        tail = std::get<0>(*result1);
        result.tag = mnemo_t::arg_t::tag_t::Immediate;
        result.data.imm = std::get<1>(*result1);
    } else if (parser_result<mnemo_t::arg_t::reg_t> result2 = parse_register(tail)) {
        tail = std::get<0>(*result2);
        result.tag = mnemo_t::arg_t::tag_t::Register;
        result.data.reg = std::get<1>(*result2);
    } else {
        throw std::logic_error("todo");
    }

    return make_option(std::make_tuple(tail, result));
}

static auto parse_line(std::string_view tail) -> parser_result<mnemo_t> {
    // Parses a line from text assembly like
    // mov eax, 100

    mnemo_t result;

    
    std::tuple<std::string_view, string> result1 = scan_while_char(tail, [](char c) { return c != ' '; });
    tail = std::get<0>(result1);
    string mnemo_name = std::get<1>(result1);

    mnemo_t::arg_t arg1;

    throw std::logic_error("todo");

    return make_option(std::make_tuple(tail, result));
}

namespace assembly {
    namespace parse {
        auto parse(std::string_view tail) -> vector<mnemo_t> {
            // Parses multiline assembly text

            vector<mnemo_t> result{};
            
            for (;;) {
                if (parser_result<mnemo_t> result1 = parse_line(tail)) {
                    // If result is available, continue iteration
                    tail = std::get<0>(*result1);
                    result.push_back(std::get<1>(*result1));

                    // Consume a newline
                    if (parser_result<std::monostate> result2 = consume_prefix_char(tail, '\n', std::monostate())) {
                        // If result is available, continue iteration
                        tail = std::get<0>(*result1);
                        result.push_back(std::get<1>(*result1));
                    } else {
                        // Newline not found
                        throw std::logic_error("Uh oh");
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
