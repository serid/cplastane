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
}
