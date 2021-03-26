#pragma once

#include "../../strvec.hxx"
#include "../assembly.hxx"

namespace assembly {
    namespace parse {
        auto parse(std::string_view tail) -> vector<mnemo_t>;
    }
}