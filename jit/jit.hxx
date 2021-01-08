#pragma once

#include <cstdlib>

#include "../int.hxx"

namespace jit {
    typedef i64 (*jit_func_t)();

    auto eval_mc(const u8 *mc, size_t len) -> i64;
}