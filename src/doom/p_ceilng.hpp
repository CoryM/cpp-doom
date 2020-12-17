#pragma once

#ifndef DOOM_P_CEILING_HPP_INCLUDE
#define DOOM_P_CEILING_HPP_INCLUDE

#include "../m_fixed.hpp" // fixed_t
#include "d_think.hpp"    // thinker_s
#include "r_defs.hpp"     // sector_t

#include <array>

#define CEILSPEED FRACUNIT
//#define CEILWAIT  150

constexpr size_t MAXCEILINGS = 30;

//
// P_CEILNG
//
enum ceiling_e
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise
};


struct ceiling_t {
    thinker_s thinker;
    ceiling_e type;
    sector_t *sector;
    fixed_t   bottomheight;
    fixed_t   topheight;
    fixed_t   speed;
    bool      crush;

    // 1 = up, 0 = waiting, -1 = down
    int direction;

    // ID
    int tag;
    int olddirection;
};

extern std::array<ceiling_t *, MAXCEILINGS> activeceilings;

auto EV_DoCeiling(line_s *line, ceiling_e type) -> int;
void T_MoveCeiling(ceiling_t *ceiling);
void P_AddActiveCeiling(ceiling_t *c);
void P_RemoveActiveCeiling(ceiling_t *c);
auto EV_CeilingCrushStop(line_s *line) -> int;
void P_ActivateInStasisCeiling(line_s *line);

#endif // DOOM_P_CEILING_HPP_INCLUDE