//
// Created by jitrs on 07/02/2021.
//

#include "parsec.hxx"

static auto is_digit(char c) -> bool {
    return '0' <= c && c <= '9';
}

namespace parsec {
    auto skip_while_char(std::string_view s,
                         std::function<bool(char)> predicate) -> infallible_parser_result<std::monostate> {
        std::string_view rest = s;
        for (; predicate(rest[0]); rest = rest.substr(1));
        return std::make_tuple(rest, std::monostate());
    }

    auto scan_while_char(std::string_view s, std::function<bool(char)> predicate) -> infallible_parser_result<string> {
        string result{};
        std::string_view rest = s;
        for (; predicate(rest[0]); rest = rest.substr(1)) {
            result.push_back(rest[0]);
        }
        return std::make_tuple(rest, result);
    }

    auto scan_char(std::string_view s) -> parser_result<char> {
        std::string_view rest = s;
        if (!rest.empty()) {
            return std::make_tuple(rest.substr(1), rest[0]);
        } else {
            return std::nullopt;
        }
    }

    auto parse_i64(std::string_view s) -> parser_result<i64> {
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
}