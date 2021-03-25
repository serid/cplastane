#pragma once

#include <tuple>
#include <string_view>
#include <functional>
#include <variant>

#include "../int.hxx"
#include "../strvec.hxx"
#include "../util/option/option.hxx"

namespace parsec {
    template<typename T>
    using infallible_parser_result = std::tuple<std::string_view, T>;

    template<typename T>
    using parser_result = Option<std::tuple<std::string_view, T>>;

    template<typename T, typename A>
    using parser_type = std::function<parser_result<T>(std::string_view, A)>;

    auto skip_while_char(std::string_view tail,
                         std::function<bool(char)> predicate) -> infallible_parser_result<std::monostate>;

    auto
    scan_while_char(std::string_view tail, std::function<bool(char)> predicate) -> infallible_parser_result<string>;

    auto scan_char(std::string_view tail) -> parser_result<char>;

    auto parse_i64(std::string_view tail) -> parser_result<i64>;

    template<typename T>
    auto consume_prefix_char(std::string_view tail, char prefix, T on_success) -> parser_result<T> {
        if (parser_result<char> result1 = scan_char(tail)) {
            tail = std::get<0>(*result1);
            char c = std::get<1>(*result1);
            if (c == prefix) {
                return make_option(std::make_tuple(tail, on_success));
            } else {
                return parser_result<T>();
            }
        } else {
            return parser_result<T>();
        }
    }

    template<typename T>
    auto consume_prefix_str(std::string_view tail, std::string_view prefix, T on_success) -> parser_result<T> {
        if (tail.size() < prefix.size())
            return parser_result<T>();
        for (size_t i = 0; i < prefix.size(); ++i) {
            if (tail[i] != prefix[i])
                return parser_result<T>();
        }
        return make_option(std::make_tuple(tail.substr(prefix.size()), on_success));
    }
}
