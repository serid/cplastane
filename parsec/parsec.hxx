#pragma once

#include <tuple>
#include <string_view>
#include <functional>
#include <optional>
#include <variant>

#include "../int.hxx"
#include "../strvec.hxx"

namespace parsec {
    template<typename T>
    using infallible_parser_result = std::tuple<std::string_view, T>;

    template<typename T>
    using parser_result = std::optional<std::tuple<std::string_view, T>>;

    template<typename T, typename A>
    using parser_type = std::function<parser_result<T>(std::string_view, A)>;

    auto skip_while_char(std::string_view tail,
                         std::function<bool(char)> predicate) -> infallible_parser_result<std::monostate>;

    auto
    scan_while_char(std::string_view tail, std::function<bool(char)> predicate) -> infallible_parser_result<string>;

    auto scan_char(std::string_view tail) -> parser_result<char>;

    auto parse_i64(std::string_view tail) -> parser_result<i64>;

    template<typename T>
    auto consume_prefix_char(std::string_view tail, char prefix, T on_success) -> parser_result<T>;

    template<typename T>
    auto consume_prefix_str(std::string_view tail, std::string_view prefix, T on_success) -> parser_result<T>;

    template<typename T>
    auto choice_combinator(std::string_view tail, vector<parser_type<T, std::monostate>> funs) -> parser_result<T>;

    template<typename T>
    auto consume_prefix_char(std::string_view tail, char prefix, T on_success) -> parser_result<T> {
        if (parser_result<char> result1 = scan_char(tail)) {
            tail = std::get<0>(*result1);
            char c = std::get<1>(*result1);
            if (c == prefix) {
                return std::make_tuple(tail, on_success);
            } else {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }

    template<typename T>
    auto consume_prefix_str(std::string_view tail, std::string_view prefix, T on_success) -> parser_result<T> {
        std::string_view prefix_rest = prefix;
        for (;;) {
            /* // Alternative implementation
            if (parser_result<char> result1 = scan_char(prefix_rest)) {
                tail = std::get<0>(*result1);
                char prefix_char = std::get<1>(*result1);

                if (parser_result<std::monostate> result2 = consume_prefix_char(tail, prefix_char, std::monostate())) {
                    tail = std::get<0>(*result2);
                    std::monostate is_successful = std::get<1>(*result2);

                    // Continue the loop
                } else {
                    return std::nullopt;
                }
            } else {
                return std::make_tuple(tail, on_success);
            }*/

            if (prefix_rest.empty()) {
                return std::make_tuple(tail, on_success);
            }
            if (tail.empty()) {
                return std::nullopt;
            }
            if (prefix_rest[0] != tail[0]) {
                return std::nullopt;
            }
            prefix_rest = prefix_rest.substr(1);
            tail = tail.substr(1);
        }
    }

    // Tries to apply the parsers in the list `funs` in order, until one of them succeeds. Returns the value of the succeeding parser.
    template<typename T>
    auto choice_combinator(std::string_view tail, vector<parser_type<T, std::monostate>> funs) -> parser_result<T> {
        for (auto &fun : funs) {
            if (parser_result<T> result1 = fun(tail, std::monostate())) {
                tail = std::get<0>(*result1);
                T fun_result = std::get<1>(*result1);
                return std::make_tuple(tail, fun_result);
            }
        }
        return std::nullopt;
    }
}
