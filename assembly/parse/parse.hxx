#pragma once

#include "../../strvec.hxx"
#include "../assembly.hxx"

using namespace std;

namespace assembly {
    namespace parse {
        auto parse(string_view tail) -> vector<mnemo_t>;
    }
}