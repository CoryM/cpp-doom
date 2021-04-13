#ifndef GLOBALS_DOOM_HPP
#define GLOBALS_DOOM_HPP

#include "../m_cheat.hpp"

// Copy Posta for quick include
// #include "globals_doom.hpp"     // for Globals::Doom namespace

namespace globals {
// Used by ST StatusBar stuff.
constexpr int AM_MSGHEADER  = 0x616D0000; // am??
constexpr int AM_MSGENTERED = 0x616D6500; // ame?
constexpr int AM_MSGEXITED  = 0x616D7800; // amx?
namespace doom {

    // am_map.cpp
    static bool       automapactive = false;
    static cheatseq_t cheat_amap    = CHEAT("iddt", 0);

    // Pulled from m_menu.cpp
    static bool inhelpscreens = false;
    static bool menuactive    = false;

} // namespace doom

} // namespace globals

#endif // GLOBALS_DOOM_HPP