#pragma once

#include <iostream>

#include "strvec.hxx"

#define print(a) std::cout << (a)
#define println(a) std::cout << (a) << std::endl

auto is_prefix(const string &s, size_t start, const string &prefix) -> bool;