#pragma once

#include "../../strvec.hxx"
#include "../assembly.hxx"
#include "../../parsec/parsec.hxx"

using namespace std;
using namespace parsec;

namespace assembly::parse {
    auto parse(strive tail) -> vector<mnemo_t>;

    auto test() -> void;
}