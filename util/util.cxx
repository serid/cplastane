#include "util.hxx"

auto is_prefix(std::string_view s, std::string_view prefix) -> bool {
    if (s.size() < prefix.size())
        return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (s[i] != prefix[i])
            return false;
    }
    return true;
}