#pragma once

#include <tuple>
#include <string_view>
#include <functional>
#include <variant>
#include <cstring>

#include "../int.hxx"
#include "../strvec.hxx"
#include "../util/option/option.hxx"

using namespace std;

namespace parsec {
    // Cannot use std::string_view since we need "start" field for providing error messages later.
    class strive {
        const char *s;
        size_t start;
        size_t size; // size of char sequence from index "start" to end of "s" string

        strive(const char *s, size_t start, size_t size) : s(s), start(start), size(size) {}

    public:
        [[nodiscard]] auto get_start() const -> size_t {
            return this->start;
        }

        [[nodiscard]] auto get_size() const -> size_t {
            return this->size;
        }

        [[nodiscard]] auto empty() const -> bool {
            return this->get_size() == 0;
        }

        [[nodiscard]] auto front() const -> char {
            return this->s[start];
        }

        auto operator[](size_t i) const -> char {
            return this->s[start + i];
        }

        [[nodiscard]] auto substr(size_t i) const -> strive {
            return {
                    this->s,
                    this->start + i,
                    this->size - i,
            };
        }

        [[nodiscard]] auto to_string() const -> string {
            return string(this->s + this->start, this->get_size());
        }

        auto operator==(const char *str) const -> bool {
            return strcmp(this->s + this->start, str) == 0;
        }

        strive(const char *s) : s(s), start(0), size(strlen(s)) {}

        strive(const string &s) : s(s.data()), start(0), size(s.size()) {}
    };

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
        if (tail.get_size() < prefix.get_size())
            return OptionParserResult<T>();
        for (size_t i = 0; i < prefix.get_size(); ++i) {
            if (tail[i] != prefix[i])
                return OptionParserResult<T>();
        }
        return make_option(ParserResult(tail.substr(prefix.get_size()), on_success));
    }
}
