#pragma once

#include "../int.hxx"
#include "../strvec.hxx"

namespace test {
    struct test_t {
        string name;

    };

    auto run_test(const test_t &x_test) -> bool;
}