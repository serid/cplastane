#pragma once

#include "../../strvec.hxx"
#include "../assembly.hxx"

using namespace std;

namespace assembly::parse {
    auto parse(string_view tail) -> vector<mnemo_t>;

    auto test() -> void;
}