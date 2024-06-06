#include "doomtype.hpp"

#include <string_view> // std::string_view
#include <locale>      // std::tolower
#include <algorithm>   // std::lexicographical_compare

bool strcasecmp(std::string_view s1, std::string_view s2) {
    return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), [](char a, char b) {
        return std::tolower(a) < std::tolower(b);
    });
};

bool strncasecmp(std::string_view s1, std::string_view s2, size_t n) {
    const auto s1_end = (n > s1.size()) ? s1.begin() + n : s1.end();
    const auto s2_end = (n > s2.size()) ? s2.begin() + n : s2.end();
    return std::lexicographical_compare(s1.begin(), s1_end, s2.begin(), s2_end, [](char a, char b) {
        return std::tolower(a) < std::tolower(b);
    });
};
