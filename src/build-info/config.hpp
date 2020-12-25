#ifndef BUILD_INFO_CONFIG_HPP
#define BUILD_INFO_CONFIG_HPP

// Change these options to print out only necessary info.
struct Options {
    constexpr static bool titles               = true;
    constexpr static bool attributes           = true;
    constexpr static bool general_features     = true;
    constexpr static bool core_features        = true;
    constexpr static bool lib_features         = true;
    constexpr static bool supported_features   = true;
    constexpr static bool unsupported_features = false;
    constexpr static bool sorted_by_value      = false;
    constexpr static bool cxx11                = true;
    constexpr static bool cxx14                = true;
    constexpr static bool cxx17                = true;
    constexpr static bool cxx20                = true;
    constexpr static bool cxx23                = true;

    [[nodiscard]] static auto cxx11_core() -> bool;
    [[nodiscard]] static auto cxx14_core() -> bool;
    [[nodiscard]] static auto cxx14_libs() -> bool;
    [[nodiscard]] static auto cxx17_core() -> bool;
    [[nodiscard]] static auto cxx17_libs() -> bool;
    [[nodiscard]] static auto cxx20_core() -> bool;
    [[nodiscard]] static auto cxx20_libs() -> bool;
    [[nodiscard]] static auto cxx23_core() -> bool;
    [[nodiscard]] static auto cxx23_libs() -> bool;
};


#endif // BUILD_INFO_CONFIG_HPP