#include "parse.hxx"

#include <tuple>
#include <string_view>
#include <functional>
#include <variant>
#include <exception>

// Parse assembly text using parser combinators

using namespace assembly;

template<typename T>
using infallible_parser_result = std::tuple<std::string_view, T>;

template<typename T>
using parser_result = std::optional<std::tuple<std::string_view, T>>;

template<typename T, typename A>
using parser_type = std::function<parser_result<T>(std::string_view, A)>;

static auto is_digit(char c) -> bool {
    return '0' <= c && c <= '9';
}

static auto
skip_while_char(std::string_view s, std::function<bool(char)> predicate) -> infallible_parser_result<std::monostate> {
    std::string_view rest = s;
    for (; predicate(rest[0]); rest = rest.substr(1));
    return std::make_tuple(rest, std::monostate());
}

static auto
scan_while_char(std::string_view s, std::function<bool(char)> predicate) -> infallible_parser_result<string> {
    string result{};
    std::string_view rest = s;
    for (; predicate(rest[0]); rest = rest.substr(1)) {
        result.push_back(rest[0]);
    }
    return std::make_tuple(rest, result);
}

static auto scan_char(std::string_view s) -> parser_result<char> {
    std::string_view rest = s;
    if (!rest.empty()) {
        return std::make_tuple(rest.substr(1), rest[0]);
    } else {
        return std::nullopt;
    }
}

template<typename T>
static auto consume_prefix_char(std::string_view s, char prefix, T on_success) -> parser_result<T> {
    std::string_view rest = s;
    if (parser_result<char> result1 = scan_char(rest)) {
        rest = std::get<0>(*result1);
        char c = std::get<1>(*result1);
        if (c == prefix) {
            return std::make_tuple(rest, on_success);
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}

template<typename T>
static auto consume_prefix_str(std::string_view s, std::string_view prefix, T on_success) -> parser_result<T> {
    std::string_view rest = s;
    std::string_view prefix_rest = prefix;
    for (;;) {
        /* // Alternative implementation
        if (parser_result<char> result1 = scan_char(prefix_rest)) {
            rest = std::get<0>(*result1);
            char prefix_char = std::get<1>(*result1);

            if (parser_result<std::monostate> result2 = consume_prefix_char(rest, prefix_char, std::monostate())) {
                rest = std::get<0>(*result2);
                std::monostate is_successful = std::get<1>(*result2);

                // Continue the loop
            } else {
                return std::nullopt;
            }
        } else {
            return std::make_tuple(rest, on_success);
        }*/

        if (prefix_rest.empty()) {
            return std::make_tuple(rest, on_success);
        }
        if (rest.empty()) {
            return std::nullopt;
        }
        if (prefix_rest[0] != rest[0]) {
            return std::nullopt;
        }
        prefix_rest = prefix_rest.substr(1);
        rest = rest.substr(1);
    }
}

// Tries to apply the parsers in the list `funs` in order, until one of them succeeds. Returns the value of the succeeding parser.
template<typename T>
static auto choice_combinator(std::string_view s,
                              vector<parser_type<T, std::monostate>> funs) -> parser_result<T> {
    std::string_view rest = s;
    for (auto &fun : funs) {
        if (parser_result<T> result1 = fun(rest, std::monostate())) {
            rest = std::get<0>(*result1);
            T fun_result = std::get<1>(*result1);
            return std::make_tuple(rest, fun_result);
        }
    }
    return std::nullopt;
}

static auto parse_register(std::string_view s) -> parser_result<mnemo_t::arg_t::reg_t> {
    mnemo_t::arg_t::reg_t result;
    std::string_view rest = s;
/*
#define test_prefix_combinator(id, prefix, value)\
            if (std::optional<std::tuple<std::string_view, std::monostate>> result##id = consume_prefix_str(rest, prefix)) {\
                rest = std::get<0>(*result##id);\
                result = value;\
            } else
*/

    vector<parser_type<mnemo_t::arg_t::reg_t, std::monostate>> funs{
            [](std::string_view s, std::monostate) -> parser_result<mnemo_t::arg_t::reg_t> {
                return consume_prefix_str(s, "eax", mnemo_t::arg_t::reg_t::Eax);
            },
            [](std::string_view s, std::monostate) -> parser_result<mnemo_t::arg_t::reg_t> {
                return consume_prefix_str(s, "ebx", mnemo_t::arg_t::reg_t::Ebx);
            },
    };

    if (parser_result<mnemo_t::arg_t::reg_t> result1 = choice_combinator(rest, funs)) {
        rest = std::get<0>(*result1);
        result = std::get<1>(*result1);
    } else {
        return std::nullopt;
    }

    return std::make_tuple(rest, result);
}

static auto parse_i64(std::string_view s) -> parser_result<i64> {
    // Parses an i64 like
    // 100
    // -100

    std::string_view rest = s;

    i64 result = 0;

    bool is_negative;
    if (rest[0] == '-') {
        is_negative = true;
        rest = rest.substr(1);
    } else {
        is_negative = false;
    }

    // If there is no digit, return error
    if (!is_digit(rest[0])) {
        return std::nullopt;
    }

    // Continues while first char in `rest` is a digit
    // Each iteration slices off one char
    for (; is_digit(rest[0]); rest = rest.substr(1)) {
        result *= 10;
        result += rest[0] - '0';
    }

    if (is_negative) {
        result = -result;
    }

    return std::make_tuple(rest, result);
}

static auto parse_arg(std::string_view s) -> parser_result<mnemo_t::arg_t> {
    // Parses an arg from text assembly like
    // 100
    // eax
    // [eax]

    std::string_view rest = s;

    mnemo_t::arg_t result;
    if (parser_result<i64> result1 = parse_i64(rest)) {
        rest = std::get<0>(*result1);
        result.tag = mnemo_t::arg_t::tag_t::Immediate;
        result.data.imm = std::get<1>(*result1);
    } else if (parser_result<mnemo_t::arg_t::reg_t> result2 = parse_register(rest)) {
        rest = std::get<0>(*result2);
        result.tag = mnemo_t::arg_t::tag_t::Register;
        result.data.reg = std::get<1>(*result2);
    } else {
        throw std::logic_error("todo");
    }

    return std::make_tuple(rest, result);
}

static auto parse_line(std::string_view s) -> parser_result<mnemo_t> {
    // Parses a line from text assembly like
    // mov eax, 100

    mnemo_t result;

    std::string_view rest = s;

    std::tuple<std::string_view, string> result1 = scan_while_char(rest, [](char c) { return c != ' '; });
    rest = std::get<0>(result1);
    string mnemo_name = std::get<1>(result1);

    mnemo_t::arg_t arg1;

    throw std::logic_error("todo");

    return std::make_tuple(rest, result);
}

namespace assembly {
    namespace parse {
        auto parse(std::string_view s) -> vector<mnemo_t> {
            // Parses multiline assembly text

            vector<mnemo_t> result{};
            std::string_view rest = s;

            for (;;) {
                if (parser_result<mnemo_t> result1 = parse_line(rest)) {
                    // If result is available, continue iteration
                    rest = std::get<0>(*result1);
                    result.push_back(std::get<1>(*result1));

                    // Consume a newline
                    if (parser_result<std::monostate> result2 = consume_prefix_char(rest, '\n', std::monostate())) {
                        // If result is available, continue iteration
                        rest = std::get<0>(*result1);
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
