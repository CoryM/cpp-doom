#// Originally based on https://en.cppreference.com/w/cpp/feature_test
#ifndef BUILD_INFO_COMPILER_FEATURE_HPP
#define BUILD_INFO_COMPILER_FEATURE_HPP

#include <iomanip>  // for operator<<, setw
#include <iostream> // for operator<<, basic_ostream, left, basic_ostream...
#include <string>   // for allocator, char_traits, operator<<, string
#include <vector>   // for vector

#include "config.hpp" // for PrintOptions, print


#define COMPILER_FEATURE_VALUE(value) #value
#define COMPILER_FEATURE_ENTRY(name)  { #name, COMPILER_FEATURE_VALUE(name) },

#ifdef __has_cpp_attribute
#define COMPILER_ATTRIBUTE_VALUE_AS_STRING(s) #s
#define COMPILER_ATTRIBUTE_AS_NUMBER(x)       COMPILER_ATTRIBUTE_VALUE_AS_STRING(x)
#define COMPILER_ATTRIBUTE_ENTRY(attr) \
    { #attr, COMPILER_ATTRIBUTE_AS_NUMBER(__has_cpp_attribute(attr)) },
#else
#define COMPILER_ATTRIBUTE_ENTRY(attr) { #attr, "_" },
#endif


struct CompilerFeature {
    CompilerFeature(const char *name = nullptr, const char *value = nullptr)
        : name(name)
        , value(value)
    {
    }
    //const char *name;
    std::string_view name;
    std::string_view value;
};

constexpr bool is_feature_supported(const CompilerFeature &x)
{
    return x.value[0] != '_' && x.value[0] != '0';
}

inline void print_compiler_feature(const CompilerFeature &x)
{
    constexpr static int max_name_length = 44; //< Update if necessary
    std::string          value { is_feature_supported(x) ? x.value : "------" };
    if (value.back() == 'L') value.pop_back(); //~ 201603L -> 201603
    // value.insert(4, 1, '-'); //~ 201603 -> 2016-03
    if ((Options::supported_features && is_feature_supported(x))
        or (Options::unsupported_features && !is_feature_supported(x)))
    {
        std::cout << std::left << std::setw(max_name_length)
                  << x.name << " " << value << '\n';
    }
}

void show(char const *title, const std::vector<CompilerFeature> &features);


#endif //  BUILD_INFO_COMPILER_FEATURE_HPP