#include "config.hpp"

[[nodiscard]] auto Options::cxx11_core() -> bool
{
    return cxx11 && core_features;
}

[[nodiscard]] auto Options::cxx14_core() -> bool
{
    return cxx14 && core_features;
}

[[nodiscard]] auto Options::cxx14_libs() -> bool
{
    return cxx14 && lib_features;
}

[[nodiscard]] auto Options::cxx17_core() -> bool
{
    return cxx17 && core_features;
}

[[nodiscard]] auto Options::cxx17_libs() -> bool
{
    return cxx17 && lib_features;
}

[[nodiscard]] auto Options::cxx20_core() -> bool
{
    return cxx20 && core_features;
}

[[nodiscard]] auto Options::cxx20_libs() -> bool
{
    return cxx20 && lib_features;
}

[[nodiscard]] auto Options::cxx23_core() -> bool
{
    return cxx23 && core_features;
}

[[nodiscard]] auto Options::cxx23_libs() -> bool
{
    return cxx23 && lib_features;
}