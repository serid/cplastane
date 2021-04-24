#pragma once

#include <iostream>

#include "../../strvec.hxx"
#include "../assembly.hxx"
#include "../../parsec/parsec.hxx"

using namespace std;
using namespace parsec;

namespace assembly::parse {
    struct ParserError {
        // Error description
        const char *what;

        strive where;
    };

    static auto make_error(strive s, const char *what) -> ParserError {
        return {
                .what=what,
                .where=s,
        };
    }

    template<typename T>
    using ParserResultResult = Result<ParserResult<T>, ParserError>;

    template<typename T>
    auto unwrap_or_log_error(ParserResultResult<T> p) -> ParserResult<T> {
        if (p.is_ok())
            return p.value();
        auto error = p.error();
        cout << "Parsing error:\n";
        cout << error.what << "\n";
        cout << "Somewhere in:\n";
        for (u64 i = 0; i < error.where.get_size(); i++)
            cout << error.where[i];
        cout << "\n";
        throw runtime_error("parsing error");
    }

    auto parse(strive tail) -> ParserResultResult<vector<mnemo_t>>;

    auto test() -> void;
}