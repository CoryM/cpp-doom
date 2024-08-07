module;

//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Random number LUT.
//	Default Config File.
//	PCX Screenshots.
//

#include <concepts> // std::convertible_to
#include <type_traits> // std::true_type, std::false_type

export module crispy;

namespace details {
// By defualt Nothing is int_convertable_impl
template<class T>
struct int_convertable_impl : std::false_type {};

// make int_convertable_impl a concept
template<typename T>
concept int_convertable = int_convertable_impl<T>::value;

// is wrapable makes usre wrap_next can work
template<typename T>
concept is_wrapable = requires {
    int_convertable<T>;
    { static_cast<int>(T::NUM) };
};

}

// Convert to int from approved types
export [[nodiscard]] constexpr int to_int(const details::int_convertable auto b) {
    return static_cast<int>(b);
};

// Convert to int* from approved types
export [[nodiscard]] int* to_ptr(details::int_convertable auto &b) {
    return reinterpret_cast<int*>(&b);
};

// Convert to int_convertable type from int
export template<details::int_convertable T>
[[nodiscard]] constexpr T from_int(const int i) {
    return static_cast<T>(i);
};

// Bit AND int_convertable type
export template<details::int_convertable T, typename U = int>
[[nodiscard]] constexpr U bit_AND(const T a, const T b) {
    return static_cast<U>(to_int(a) & to_int(b));
};

// Bit OR int_convertable type
export template<details::int_convertable T, typename U = T>
[[nodiscard]] constexpr U bit_OR(const T a, const T b) {
    return static_cast<U>(to_int(a) | to_int(b));
};


// Convert to int_convertable type from int
export template<details::is_wrapable T>
[[nodiscard]] constexpr T wrap_next(const T e) {
    return static_cast<T>((to_int(e) + 1) % to_int(T::NUM));
};

// Convert to int_convertable type from int
export template<details::is_wrapable T>
[[nodiscard]] constexpr int get_max() {
    return static_cast<int>(T::NUM);
};


// ENUMS


// [crispy] "crispness" config variables
// Amount of Head/Weapon bobbing
export enum class eBobFactor : int
{
    Full,
    Mid,
    Off,
    NUM,
};
// eBobFactor is int_convertable_impl
template<>
struct details::int_convertable_impl<eBobFactor> : std::true_type {};


// Brightmaps
export enum class eBrightmaps : int
{
    Off,
    Textures,
    Sprites,
    Both,
    NUM,
};
// eBrightmaps is int_convertable_impl
template<>
struct details::int_convertable_impl<eBrightmaps> : std::true_type {};


// Center Weapon
export enum class eCenterWeapon : int
{
    Off,
    Center,
    Bob,
    NUM,
};
// eCenterWeapon is int_convertable_impl
template<>
struct details::int_convertable_impl<eCenterWeapon> : std::true_type {};


// ColoredHud
export enum class eColoredHud : int
{
    Off,
    Bar,
    Text,
    Both,
    NUM,
};
// eColoredHud is int_convertable_impl
template<>
struct details::int_convertable_impl<eColoredHud> : std::true_type {};


  // eCrosshair
export enum class eCrosshair : int
{
    Off,
    Static,
    Projected,
    NUM,
    intercept = 0x10,
};
// eCrosshair is int_convertable_impl
template<>
struct details::int_convertable_impl<eCrosshair> : std::true_type {};