//
// Created by jitrs on 07/02/2021.
//

#include "parsec.hxx"

static auto is_digit(char c) -> bool {
    return '0' <= c && c <= '9';
}

namespace parsec {
    auto skip_while_char(std::string_view tail,
                         std::function<bool(char)> predicate) -> infallible_parser_result<std::monostate> {
        for (; predicate(tail[0]); tail = tail.substr(1));
        return std::make_tuple(tail, std::monostate());
    }

    auto scan_while_char(std::string_view tail, std::function<bool(char)> predicate) -> infallible_parser_result<string> {
        string result{};
        for (; predicate(tail[0]); tail = tail.substr(1)) {
            result.push_back(tail[0]);
        }
        return std::make_tuple(tail, result);
    }

    auto scan_char(std::string_view tail) -> parser_result<char> {
        if (!tail.empty()) {
            return std::make_tuple(tail.substr(1), tail[0]);
        } else {
            return std::nullopt;
        }
    }

    auto parse_i64(std::string_view tail) -> parser_result<i64> {
        // Parses an i64 like
        // 100
        // -100

        i64 result = 0;

        bool is_negative;
        if (tail[0] == '-') {
            is_negative = true;
            tail = tail.substr(1);
        } else {
            is_negative = false;
        }

        // If there is no digit, return error
        if (!is_digit(tail[0])) {
            return std::nullopt;
        }

        // Continues while first char in `tail` is a digit
        // Each iteration slices off one char
        for (; is_digit(tail[0]); tail = tail.substr(1)) {
            result *= 10;
            result += tail[0] - '0';
        }

        if (is_negative) {
            result = -result;
        }

        return std::make_tuple(tail, result);
    }
}