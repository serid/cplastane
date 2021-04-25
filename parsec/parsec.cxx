//
// Created by jitrs on 07/02/2021.
//

#include "parsec.hxx"

using namespace std;

static auto is_dec_digit(char c) -> bool {
    return '0' <= c && c <= '9';
}

static auto is_hex_digit(char c) -> bool {
    return is_dec_digit(c) || ('A' <= c && c <= 'F');
}

// Assumes that c is a dec digit
static auto dec_digit_to_int(char c) -> i64 {
    return c - '0';
}

// Assumes that c is a hex digit
static auto hex_digit_to_int(char c) -> i64 {
    return is_dec_digit(c) ? dec_digit_to_int(c) : c + 10 - 'A';
}

namespace parsec {
    auto skip_while_char(strive tail, const function<bool(char)> &predicate) -> ParserResult<monostate> {
        for (; !tail.empty() && predicate(tail.front()); tail = tail.substr(1));
        return ParserResult(tail, monostate());
    }

    auto scan_while_char(strive tail, const function<bool(char)> &predicate) -> ParserResult<string> {
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

    // Assumes that base is 10 or 16
    template<u8 base>
    auto parse_positive_i64(strive tail) -> OptionParserResult<i64> {
        // Parses an i64 like
        // 100
        // 0xFF

        if constexpr (base == 10) {
            if (tail.empty() || !is_dec_digit(tail.front()))
                return OptionParserResult<i64>();
        } else if constexpr (base == 16) {
            if (tail.empty() || !is_hex_digit(tail.front()))
                return OptionParserResult<i64>();
        }

        i64 result = 0;

        // Continues while first char in `tail` is a digit
        // Each iteration slices off one char
        for (;; tail = tail.substr(1)) {
            if constexpr (base == 10) {
                if (tail.empty() || !is_dec_digit(tail.front()))
                    break;
                result *= 10;
                result += dec_digit_to_int(tail.front());
            } else if constexpr (base == 16) {
                if (tail.empty() || !is_hex_digit(tail.front()))
                    break;
                result *= 16;
                result += hex_digit_to_int(tail.front());
            }
        }

        return make_option(ParserResult(tail, result));
    }

    auto parse_i64(strive tail) -> OptionParserResult<i64> {
        // Parses an i64 like
        // 100
        // -100
        // 0xFF
        // -0xFF

        if (tail.empty())
            return OptionParserResult<i64>();

        i64 sign = 1;
        if (tail.front() == '-') {
            sign = -1;
            tail = tail.substr(1);
        }

        u8 base = 10;
        if (tail.get_size() >= 2 && tail[0] == '0' && tail[1] == 'x') {
            base = 16;
            tail = tail.substr(2);
        }

        OptionParserResult<i64> a;
        if (base == 10)
            a = parse_positive_i64<10>(tail);
        else
            a = parse_positive_i64<16>(tail);

        if (a) {
            i64 result = a.value().data;
            result = result * sign;

            return make_option(ParserResult(a.value().tail, result));
        } else {
            return OptionParserResult<i64>();
        }
    }

    auto consume_prefix_char(strive tail, char prefix) -> OptionParserResult<monostate> {
        if (OptionParserResult<char> result1 = scan_char(tail)) {
            tail = result1.value().tail;
            char c = result1.value().data;
            if (c == prefix) {
                return make_option(ParserResult(tail, monostate()));
            } else {
                return OptionParserResult<monostate>();
            }
        } else {
            return OptionParserResult<monostate>();
        }
    }
}