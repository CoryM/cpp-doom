#// Originally based on https://en.cppreference.com/w/cpp/feature_test

#if __cplusplus < 201709
#error "C++20 (C++2a) or better is required"
#endif

#include <version> // for __cpp_lib_array_constexpr, __cpp_lib...

#include "compiler_feature.hpp" // for COMPILER_FEATURE_ENTRY, show, Compil...
#include "config.hpp"           // for PrintOptions, print
#include "cxx.hpp"
#include "cxx-attributes.hpp"
#include "cxx11.hpp"
#include "cxx14.hpp"
#include "cxx17.hpp"
#include "cxx20.hpp"
#include "cxx23.hpp"


auto main() -> int
{
    if (Options::general_features) show("C++ GENERAL", cxx);
    if (Options::cxx11_core()) show("C++11 CORE", cxx11);
    if (Options::cxx14_core()) show("C++14 CORE", cxx14);
    if (Options::cxx14_libs()) show("C++14 LIB", cxx14lib);
    if (Options::cxx17_core()) show("C++17 CORE", cxx17);
    if (Options::cxx17_libs()) show("C++17 LIB", cxx17lib);
    if (Options::cxx20_core()) show("C++20 CORE", cxx20);
    if (Options::cxx20_libs()) show("C++20 LIB", cxx20lib);
    if (Options::cxx23_core()) show("C++23 CORE", cxx23);
    if (Options::cxx23_libs()) show("C++23 LIB", cxx23lib);
    if (Options::attributes) show("ATTRIBUTES", attributes);
}
