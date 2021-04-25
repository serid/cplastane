#pragma once

#include <iostream>

#include "../../strvec.hxx"
#include "../assembly.hxx"
#include "../../parsec/parsec.hxx"

namespace assembly::parse {
    struct ParserError {
        // Error description
        const char *what;

        parsec::strive where;
    };

    template<typename T>
    using ParserResultResult = Result<parsec::ParserResult<T>, ParserError>;

    template<typename T>
    auto unwrap_or_log_error(ParserResultResult<T> p) -> parsec::ParserResult<T> {
        if (p.is_ok())
            return p.value();
        auto error = p.error();
        std::cout << "Parsing error:\n";
        std::cout << error.what << "\n";
        std::cout << "Somewhere in:\n";
        for (u64 i = 0; i < error.where.get_size(); i++)
            std::cout << error.where[i];
        std::cout << "\n";
        throw std::runtime_error("parsing error");
    }

    auto parse(parsec::strive tail) -> ParserResultResult<vector<mnemo_t>>;

    auto test() -> void;
}