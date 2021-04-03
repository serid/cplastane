//
// Created by jitrs on 07/02/2021.
//

#include "parsec.hxx"

using namespace std;

static auto is_digit(char c) -> bool {
    return '0' <= c && c <= '9';
}

namespace parsec {
    auto skip_while_char(strive tail, const function<bool(char)> &predicate) -> ParserResult<monostate> {
        for (; !tail.empty() && predicate(tail.front()); tail = tail.substr(1));
        return ParserResult(tail, monostate());
    }

    auto
    scan_while_char(strive tail, const function<bool(char)> &predicate) -> ParserResult<string> {
        string result{};
        for (; !tail.empty() && predicate(tail.front()); tail = tail.substr(1)) {
            result.push_back(tail.front());
        }
        return ParserResult<string>(tail, move(result));
    }

    auto scan_char(strive tail) -> OptionParserResult<char> {
        if (tail.empty())
            return OptionParserResult<char>();
        return make_option(ParserResult(tail.substr(1), tail.front()));
    }

    auto parse_i64(strive tail) -> OptionParserResult<i64> {
        // Parses an i64 like
        // 100
        // -100

        i64 result = 0;

        if (tail.empty())
            return OptionParserResult<i64>();

        bool is_negative = tail.front() == '-';
        if (is_negative)
            tail = tail.substr(1);

        // If there is no digit, return error
        if (tail.empty() || !is_digit(tail.front()))
            return OptionParserResult<i64>();

        // Continues while first char in `tail` is a digit
        // Each iteration slices off one char
        for (; !tail.empty() && is_digit(tail.front()); tail = tail.substr(1)) {
            result *= 10;
            result += tail.front() - '0';
        }

        if (is_negative) {
            result = -result;
        }

        return make_option(ParserResult(tail, result));
    }
}