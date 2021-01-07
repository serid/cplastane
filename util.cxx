#include "util.hxx"

auto is_prefix(const string &s, size_t start, const string &prefix) -> bool {
    size_t i = start; // Index in s
    size_t j = 0; // Index in prefix
    for (;;) {
        // Prefix ended
        if (j >= prefix.size())
            return true;
        // String ended
        if (i >= s.size())
            return false;
        // Character divergence
        if (s[i] != prefix[j])
            return false;
        i++;
        j++;
    }
}