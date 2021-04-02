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
    using strive = string_view;

    template<typename T>
    struct ParserResult {
        strive tail;
        T data;

        ParserResult(strive tail, T data) : tail(tail), data(data) {}
    };

    template<typename T>
    using OptionParserResult = Option<ParserResult<T>>;

    auto skip_while_char(strive tail, const function<bool(char)> &predicate) -> ParserResult<monostate>;

    auto
    scan_while_char(strive tail, const function<bool(char)> &predicate) -> ParserResult<string>;

    auto scan_char(strive tail) -> OptionParserResult<char>;

    auto parse_i64(strive tail) -> OptionParserResult<i64>;

    template<typename T>
    auto consume_prefix_char(strive tail, char prefix, T on_success) -> OptionParserResult<T> {
        if (OptionParserResult<char> result1 = scan_char(tail)) {
            tail = result1.value().tail;
            char c = result1.value().data;
            if (c == prefix) {
                return make_option(ParserResult(tail, on_success));
            } else {
                return OptionParserResult<T>();
            }
        } else {
            return OptionParserResult<T>();
        }
    }

    template<typename T>
    auto consume_prefix_str(strive tail, strive prefix, T on_success) -> OptionParserResult<T> {
        if (tail.size() < prefix.size())
            return OptionParserResult<T>();
        for (size_t i = 0; i < prefix.size(); ++i) {
            if (tail[i] != prefix[i])
                return OptionParserResult<T>();
        }
        return make_option(ParserResult(tail.substr(prefix.size()), on_success));
    }
}
