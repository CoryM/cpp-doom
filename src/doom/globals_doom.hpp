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
    extern bool       automapactive;
    extern cheatseq_t cheat_amap;

    // Pulled from m_menu.cpp
    extern bool inhelpscreens;
    extern bool menuactive;

} // namespace doom

} // namespace globals

#endif // GLOBALS_DOOM_HPP