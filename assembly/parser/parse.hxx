#pragma once

#include "../../strvec.hxx"
#include "../assembler.hxx"

namespace assembly {
    namespace parse {
        auto parse(std::string_view s) -> vector<mnemo_t>;
    }
}