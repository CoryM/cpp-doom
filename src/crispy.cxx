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
//	Crispy Doom specific variables.
//	Main loop menu stuff.
//	Random number LUT.
//	Default Config File.
//	PCX Screenshots.
//

#include <bitset>       // std::bitset
#include <concepts>     // std::convertible_to
#include <cstddef>      // std::size_t
#include <string>       // std::string
#include <type_traits>  // std::true_type, std::false_type

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


// Changes to next value wraping around if needed
export template<details::is_wrapable T>
[[nodiscard]] constexpr T wrap_next(const T e) {
    return static_cast<T>((to_int(e) + 1) % to_int(T::NUM));
};

// Convert from int_convertable to bool
export template<details::int_convertable T>
[[nodiscard]] constexpr bool to_bool(const T e) {
    return to_int(e) != 0;
}; 


// Convert to int_convertable type from int
export template<details::is_wrapable T>
[[nodiscard]] constexpr int get_max() {
    return static_cast<int>(T::NUM);
};


// Do a bitwise shift right on int_convertable type
export template<details::int_convertable T, int B = 1>
[[nodiscard]] constexpr T bit_shift_right(const T a) {
    return static_cast<T>(to_int(a) >> B);
};


// Do a bitwise shift left on int_convertable type
export template<details::int_convertable T, int B = 1>
[[nodiscard]] constexpr T bit_shift_left(const T a) {
    return static_cast<T>(to_int(a) << B);
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


// eDemoTimer
export enum class eDemoTimer : int
{
    Off,
    Record,
    Playback,
    Both,
    NUM,
};
// eDemoTimer is int_convertable_impl
template<>
struct details::int_convertable_impl<eDemoTimer> : std::true_type {};


// eFreeaim
export enum class eFreeaim : int
{
    Auto,
    Direct,
    Both,
    NUM,
};
// eFreeaim is int_convertable_impl
template<>
struct details::int_convertable_impl<eFreeaim> : std::true_type {};


// eFreelook
export enum class eFreelook : int
{
    Off,
    Spring,
    Lock,
    NUM,
};
// eFreelook is int_convertable_impl
template<>
struct details::int_convertable_impl<eFreelook> : std::true_type {};

// eJump
export enum class eJump : int
{
    Off,
    Low,
    High,
    NUM,
};
// eJump is int_convertable_impl
template<>
struct details::int_convertable_impl<eJump> : std::true_type {};


// eTranslucency
export enum class eTranslucency : int
{
    Off,
    Missile,
    Item,
    Both,
    NUM,
};
template<>
struct details::int_convertable_impl<eTranslucency> : std::true_type {};


// eWidescreen
export enum class eWidescreen : int
{
    Off,
    Wide,
    Compact,
    NUM,
};
template<>
struct details::int_convertable_impl<eWidescreen> : std::true_type {};


// eSecretMessage
export enum class eSecretMessage : int
{
    Off,
    On,
    Count,
    NUM,
};
template<>
struct details::int_convertable_impl<eSecretMessage> : std::true_type {};


// eWidgets
export enum class eWidgets : int
{
    Off,
    Automap,
    Always,
    NUM,
};
template<>
struct details::int_convertable_impl<eWidgets> : std::true_type {};


// eWidget
export enum class eWidget : int
{
    Off,
    Automap,
    NUM,
};
template<>
struct details::int_convertable_impl<eWidget> : std::true_type {};


// ------------- Bitset Based Enums -------------
export struct bReinit : std::bitset<4> {
    using TYPE = std::bitset<4>;
    static constexpr TYPE FrameBuffers{1};
    static constexpr TYPE Renderer{2};
    static constexpr TYPE Textures{4};
    static constexpr TYPE AspectRatio{8};
    bReinit(TYPE a) : TYPE(a) {};
    // Test if a bit matches
    [[nodiscard]] bool is_set(const TYPE a) const {
        return (to_ulong() & a.to_ulong()) != 0;
    };
};

// ------

export struct crispy_t
{
    // [crispy] "crispness" config variables
    int automapoverlay;
    int automaprotate;
    eWidgets automapstats;
    eBobFactor bobfactor;
    eBrightmaps brightmaps;
    eCenterWeapon centerweapon;
    int coloredblood;
    eColoredHud coloredhud;
    eCrosshair crosshair;
    int crosshairhealth;
    int crosshairtarget;
    int crosshairtype;
    eDemoTimer demotimer;
    int demotimerdir;
    int demobar;
    int extautomap;
    int extsaveg;
    int flipcorpses;
    eFreeaim freeaim;
    eFreelook freelook;
    int hires;
    eJump jump;
    eWidgets leveltime;
    int mouselook;
    int neghealth;
    int overunder;
    int pitch;
    eWidget playercoords;
    int recoil;
    eSecretMessage secretmessage;
    int smoothlight;
    int smoothscaling;
    int soundfix;
    int soundfull;
    int soundmono;
    eTranslucency translucency;
#if CRISPY_TRUECOLOR
    int truecolor;
#endif
    int uncapped;
    int vsync;
    int weaponsquat;
    eWidescreen widescreen;

    // [crispy] in-game switches and variables
    int screenshotmsg;
    int cleanscreenshot;
    int demowarp;
    int fps;

    bool flashinghom;
    bool fliplevels;
    bool flipweapons;
    bool haved1e5;
    bool havee1m10;
    bool havemap33;
    bool havessg;
    bool singleplayer;
    bool stretchsky;

    std::string sdlversion;
    std::string platform;

    void (*post_rendering_hook)(void);
};

// [crispy] "regular" config variables
constinit static crispy_t crispy_s = [](){
    crispy_t c = {};
    c.extautomap    = 1;
    c.extsaveg      = 1;
    c.hires         = 1;
    c.smoothscaling = 1;
    c.soundfix      = 1;
    c.vsync         = 1;
    return c;
}();

export crispy_t *const crispy = &crispy_s;

// [crispy] "critical" config variables
static const crispy_t critical_s = { };
export const crispy_t *critical = &critical_s;

// [crispy] update the "singleplayer" variable and the "critical" struct
export void CheckCrispySingleplayer(bool singleplayer)
{
    if ((crispy->singleplayer = singleplayer))
    {
        critical = &crispy_s;
    }
    else
    {
        critical = &critical_s;
    }
};

