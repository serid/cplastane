#pragma once

#include <tuple>
#include <string_view>
#include <functional>
#include <variant>

#include "../int.hxx"
#include "../strvec.hxx"
#include "../util/option/option.hxx"

using namespace std;

namespace parsec {
    template<typename T>
    using infallible_parser_result = tuple<string_view, T>;

    template<typename T>
    using parser_result = Option<tuple<string_view, T>>;

    template<typename T, typename A>
    using parser_type = function<parser_result<T>(string_view, A)>;

    auto skip_while_char(string_view tail,
                         function<bool(char)> predicate) -> infallible_parser_result<monostate>;

    auto
    scan_while_char(string_view tail, function<bool(char)> predicate) -> infallible_parser_result<string>;

    auto scan_char(string_view tail) -> parser_result<char>;

    auto parse_i64(string_view tail) -> parser_result<i64>;

    template<typename T>
    auto consume_prefix_char(string_view tail, char prefix, T on_success) -> parser_result<T> {
        if (parser_result<char> result1 = scan_char(tail)) {
            tail = get<0>(*result1);
            char c = get<1>(*result1);
            if (c == prefix) {
                return make_option(make_tuple(tail, on_success));
            } else {
                return parser_result<T>();
            }
        } else {
            return parser_result<T>();
        }
    }

    template<typename T>
    auto consume_prefix_str(string_view tail, string_view prefix, T on_success) -> parser_result<T> {
        if (tail.size() < prefix.size())
            return parser_result<T>();
        for (size_t i = 0; i < prefix.size(); ++i) {
            if (tail[i] != prefix[i])
                return parser_result<T>();
        }
        return make_option(make_tuple(tail.substr(prefix.size()), on_success));
    }
}
