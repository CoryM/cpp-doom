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
//
// DESCRIPTION:  the automap code
//

#include "am_map.hpp"

#include <algorithm> // for min, clamp, max
#include <array>     // for array, array<>::value_type
#include <bitset>    // for bitset, operator&, operator|
#include <cmath>     // for cos, sin
#include <cstdint>   // for int64_t, uint64_t
#include <cstdio>    // for size_t, stderr
#include <cstring>   // for memset
#include <limits>    // for numeric_limits
#include <numbers>   // for pi
#include <span>      // for span

#include "../../utils/lump.hpp" // for cache_lump_name
#include "../common.hpp"        // for ASSERT
#include "../crispy.hpp"        // for crispy, crispy_t
#include "../d_event.hpp"       // for event_t, evtype_t, evtype_t::ev_keyup
#include "../d_mode.hpp"        // for exe_doom_1_8
#include "../deh_str.hpp"       // for DEH_String, DEH_snprintf, DEH_fprintf
#include "../doomtype.hpp"      // for pixel_t
#include "../i_timer.hpp"       // for I_GetTime
#include "../i_video.hpp"       // for I_VideoBuffer, SCREENHEIGHT, SCREENW...
#include "../m_cheat.hpp"       // for cht_CheckCheat, CHEAT, cheatseq_t
#include "../m_controls.hpp"    // for joybautomap, key_map_east, key_map_n...
#include "../m_fixed.hpp"       // for FRACUNIT, fixed_t, FixedDiv, FixedMul
#include "../m_misc.hpp"        // for M_snprintf
#include "../tables.hpp"        // for angle_t, ANG90
#include "../v_video.hpp"       // for V_DrawPatch, V_MarkRect
#include "../w_wad.hpp"         // for W_ReleaseLumpName
#include "../z_zone.hpp"        // for PU, PU::STATIC
#include "d_englsh.hpp"         // for AMSTR_FOLLOWOFF, AMSTR_FOLLOWON, AMS...
#include "d_player.hpp"         // for player_t
#include "doomdata.hpp"         // for ML_SECRET, ML_DONTDRAW, ML_MAPPED
#include "doomdef.hpp"          // for MAXPLAYERS, pw_allmap, pw_invisibility
#include "doomstat.hpp"         // for gameepisode, gamemap, players, viewa...
#include "globals_doom.hpp"     // for Globals::Doom namespace
#include "info.hpp"             // for MT_BLOOD, MT_PUFF, mobjinfo_t
#include "p_local.hpp"          // for PLAYERRADIUS, MAPBLOCKUNITS, bmaporgx
#include "p_mobj.hpp"           // for mobj_t, MF_CORPSE, MF_COUNTKILL, MF_...
#include "r_defs.hpp"           // for line_s, vertex_t, sector_t, (anonymous)
#include "r_state.hpp"          // for lines, flipscreenwidth, colormaps
#include "st_stuff.hpp"         // for ST_Responder, ST_HEIGHT

struct patch_t;

namespace globals {

bool       automapactive = false;
cheatseq_t cheat_amap    = CHEAT("iddt", 0);

} // end of namespace globals


// For use if I do walls with outsides/insides
constexpr int REDS       = (256 - 5 * 16);
constexpr int REDRANGE   = 16;
constexpr int BLUES      = (256 - 4 * 16 + 8);
constexpr int GREENS     = (7 * 16);
constexpr int GREENRANGE = 16;
constexpr int GRAYS      = (6 * 16);
constexpr int GRAYSRANGE = 16;
constexpr int BROWNS     = (4 * 16);
constexpr int YELLOWS    = (256 - 32 + 7);
constexpr int BLACK      = 0;
constexpr int WHITE      = (256 - 47);
// Unused Variables
//constexpr int BLUERANGE   = 8;
//constexpr int BROWNRANGE  = 16;
//constexpr int YELLOWRANGE = 1;


// Automap colors
constexpr int BACKGROUND               = BLACK;
constexpr int WALLRANGE                = REDRANGE;
constexpr int TSWALLCOLORS             = GRAYS;
constexpr int THINGCOLORS              = GREENS;
constexpr int THINGRANGE               = GREENRANGE;
constexpr int SECRETWALLCOLORS         = 252; // [crispy] purple
constexpr int REVEALEDSECRETWALLCOLORS = 112; // [crispy] green
constexpr int GRIDCOLORS               = (GRAYS + GRAYSRANGE / 2);
constexpr int XHAIRCOLORS              = GRAYS;
// Unused variables
//constexpr int YOURCOLORS               = WHITE;
//constexpr int YOURRANGE                = 0;
//constexpr int TSWALLRANGE              = GRAYSRANGE;
//constexpr int FDWALLRANGE              = BROWNRANGE;
//constexpr int CDWALLRANGE              = YELLOWRANGE;
//constexpr int SECRETWALLRANGE          = WALLRANGE;
//constexpr int GRIDRANGE                = 0;
#define CRISPY_HIGHLIGHT_REVEALED_SECRETS

constexpr auto CDWALLCOLORS = []() {
    constexpr auto crispy_orange = 215;
    return (crispy->b_extautomap() ? crispy_orange : YELLOWS); // [crispy] orange
};

constexpr auto FDWALLCOLORS = []() {
    constexpr auto crispy_brown = 55;
    return (crispy->b_extautomap() ? crispy_brown : BROWNS); // [crispy] lt brown
};

constexpr auto WALLCOLORS = []() {
    constexpr auto crispy_red = 23;
    return (crispy->b_extautomap() ? crispy_red : REDS); // [crispy] red-brown
};

// drawing stuff
constexpr int AM_NUMMARKPOINTS = 10;

// scale on entry
constexpr auto INITSCALEMTOF = (.2 * FRACUNIT);
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
constexpr int F_PANINC = 4;
// how much zoom-in per tic
// goes to 2x in 1 second
constexpr auto M_ZOOMIN = static_cast<int>(1.02 * FRACUNIT);
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
constexpr auto M_ZOOMOUT = static_cast<int>(FRACUNIT / 1.02);
// [crispy] zoom faster with the mouse wheel
constexpr auto M2_ZOOMIN  = static_cast<int>(1.08 * FRACUNIT);
constexpr auto M2_ZOOMOUT = static_cast<int>(FRACUNIT / 1.08);

struct fpoint_t {
    int x = 0;
    int y = 0;
};

struct fline_t {
    fpoint_t a;
    fpoint_t b;
};

struct mpoint_t {
    int64_t x = 0;
    int64_t y = 0;

    [[nodiscard]] auto half() const -> mpoint_t
    {
        return { x >> 1, y >> 1 };
    }

    [[nodiscard]] auto operator*(const fixed_t &pIn) const -> mpoint_t
    {
        return { FixedMul(this->x, pIn), FixedMul(this->y, pIn) };
    }

    [[nodiscard]] auto operator/(const fixed_t &pIn) const -> mpoint_t
    {
        return { FixedDiv(this->x, pIn), FixedDiv(this->y, pIn) };
    }

    [[nodiscard]] auto operator+(const mpoint_t &mpIn) const -> mpoint_t
    {
        return { this->x + mpIn.x, this->y + mpIn.y };
    }

    [[nodiscard]] auto operator-(const mpoint_t &mpIn) const -> mpoint_t
    {
        return { this->x - mpIn.x, this->y - mpIn.y };
    }

    auto operator*=(const fixed_t &pIn) -> mpoint_t
    {
        this->x = FixedMul(this->x, pIn);
        this->y = FixedMul(this->y, pIn);
        return (*this);
    }

    auto operator-=(const mpoint_t &mpIn) -> mpoint_t
    {
        x = x - mpIn.x;
        y = y - mpIn.y;
        return (*this);
    }

    auto operator+=(const mpoint_t &mpIn) -> mpoint_t
    {
        x = x + mpIn.x;
        y = y + mpIn.y;
        return (*this);
    }
};

struct mline_t {
    mpoint_t a;
    mpoint_t b;
};

struct islope_t {
    fixed_t slp;
    fixed_t islp;
};

enum keycolor_t
{
    no_key,
    red_key,
    yellow_key,
    blue_key
};

class AM_MAP {
private:
public:
    AM_MAP()  = default;
    ~AM_MAP() = default;

    // the following is crap
    //#define LINE_NEVERSEE globals::ML_DONTDRAW
    const decltype(globals::ML_DONTDRAW) LINE_NEVERSEE = globals::ML_DONTDRAW;

    int cheating = 0;
    int grid     = 0;

    // location of window on screen
    int f_x = 0;
    int f_y = 0;

    // size of window on screen
    int f_w = 0;
    int f_h = 0;

    int lightlev = 0; // used for funky strobing effect
    int amclock  = 0;

    mpoint_t m_paninc;         // how far the window pans each tic (map coords)
    fixed_t  mtof_zoommul = 0; // how far the window zooms in each tic (map coords)
    fixed_t  ftom_zoommul = 0; // how far the window zooms in each tic (I_VideoBuffer coords)

    mpoint_t mapLL; // LL x,y where the window is on the map (map coords)
    mpoint_t mapUR; // UR x,y where the window is on the map (map coords)
    mpoint_t mapWH; // width/height of window on map (map coords)

    mpoint_t old_map_ll; // LL x,y where the window is on the map (map coords)
    mpoint_t old_map_wh; // UR x,y where the window is on the map (map coords)

    fixed_t min_scale_mtof = 0; // used to tell when to stop zooming out
    fixed_t max_scale_mtof = 0; // used to tell when to stop zooming in


    // based on level size
    fixed_t min_x = 0;
    fixed_t min_y = 0;
    fixed_t max_x = 0;
    fixed_t max_y = 0;

    fixed_t max_w = 0; // max_x-min_x,
    fixed_t max_h = 0; // max_y-min_y

    // based on player size
    const fixed_t min_w = 2 * PLAYERRADIUS;
    const fixed_t min_h = 2 * PLAYERRADIUS;


    // old location used by the Follower routine
    mpoint_t f_oldloc;

    // used by MTOF to scale from map-to-frame-buffer coords
    fixed_t scale_mtof = static_cast<fixed_t>(INITSCALEMTOF);

    // used by FTOM to scale from frame-buffer-to-map coords (=1/locals.scale_mtof)
    fixed_t scale_ftom = { 0 };

    player_t *plr = nullptr; // the player represented by an arrow

    std::array<patch_t *, AM_NUMMARKPOINTS> marknums = {};    // numbers used for marking by the automap
    std::array<mpoint_t, AM_NUMMARKPOINTS>  markpoints;       // where the points are
    int                                     markpointnum = 0; // next point to be assigned

    int followplayer = 1; // specifies whether to follow the player around

    bool stopped = true;

    mpoint_t mapcenter;
    angle_t  mapangle = 0;


    // [crispy] moved here for extended savegames
    int lastlevel   = -1;
    int lastepisode = -1;

    //
    // The vector graphics for the automap.
    //  A line drawing of the player pointing right,
    //   starting from the middle.
    //
#define R (FRACUNIT)

    const std::array<mline_t, 3> thintriangle_guy = { { //
        { { (fixed_t)(-.5 * R), (fixed_t)(-.7 * R) }, { (fixed_t)(R), (fixed_t)(0) } },
        { { (fixed_t)(R), (fixed_t)(0) }, { (fixed_t)(-.5 * R), (fixed_t)(.7 * R) } },
        { { (fixed_t)(-.5 * R), (fixed_t)(.7 * R) }, { (fixed_t)(-.5 * R), (fixed_t)(-.7 * R) } } } };

    // [crispy] print keys as crosses
    const std::array<mline_t, 2> cross_mark = { {
        { { -R, 0 }, { R, 0 } },
        { { 0, -R }, { 0, R } },
    } };

    const std::array<mline_t, 4> square_mark = { {
        { { -R, 0 }, { 0, R } },
        { { 0, R }, { R, 0 } },
        { { R, 0 }, { 0, -R } },
        { { 0, -R }, { -R, 0 } },
    } };


#undef R

#define R ((8 * PLAYERRADIUS) / 7)

    const std::array<mline_t, 7> player_arrow = { {       // >---->
        { { -R + R / 8, 0 }, { R, 0 } },                  // >---->
        { { R, 0 }, { R - R / 2, R / 4 } },               // >---->
        { { R, 0 }, { R - R / 2, -R / 4 } },              // >---->
        { { -R + R / 8, 0 }, { -R - R / 8, R / 4 } },     // >---->
        { { -R + R / 8, 0 }, { -R - R / 8, -R / 4 } },    // >---->
        { { -R + 3 * R / 8, 0 }, { -R + R / 8, R / 4 } }, // >---->
        { { -R + 3 * R / 8, 0 }, { -R + R / 8, -R / 4 } } } };


    const std::array<mline_t, 16> cheat_player_arrow = { {          //
        { { -R + R / 8, 0 }, { R, 0 } },                            // -----
        { { R, 0 }, { R - R / 2, R / 6 } },                         // ----->
        { { R, 0 }, { R - R / 2, -R / 6 } },                        //
        { { -R + R / 8, 0 }, { -R - R / 8, R / 6 } },               // >----->
        { { -R + R / 8, 0 }, { -R - R / 8, -R / 6 } },              //
        { { -R + 3 * R / 8, 0 }, { -R + R / 8, R / 6 } },           // >>----->
        { { -R + 3 * R / 8, 0 }, { -R + R / 8, -R / 6 } },          //
        { { -R / 2, 0 }, { -R / 2, -R / 6 } },                      // >>-d--->
        { { -R / 2, -R / 6 }, { -R / 2 + R / 6, -R / 6 } },         //
        { { -R / 2 + R / 6, -R / 6 }, { -R / 2 + R / 6, R / 4 } },  //
        { { -R / 6, 0 }, { -R / 6, -R / 6 } },                      // >>-dd-->
        { { -R / 6, -R / 6 }, { 0, -R / 6 } },                      //
        { { 0, -R / 6 }, { 0, R / 4 } },                            //
        { { R / 6, R / 4 }, { R / 6, -R / 7 } },                    // >>-ddt->
        { { R / 6, -R / 7 }, { R / 6 + R / 32, -R / 7 - R / 32 } }, //
        { { R / 6 + R / 32, -R / 7 - R / 32 }, { R / 6 + R / 10, -R / 7 } } } };

#undef R

    // translates between frame-buffer and map distances
    // [crispy] fix int overflow that causes map and grid lines to disappear

    [[nodiscard]] auto FTOM(int64_t x) const -> int
    {
        //return (((x << FRACBITS) * scale_ftom) >> FRACBITS);
        return (x * scale_ftom);
    }

    [[nodiscard]] auto MTOF(int64_t x) const -> int64_t
    {
        return (static_cast<uint64_t>(x * scale_mtof) >> FRACBITS) >> FRACBITS;
    }


    // translates between frame-buffer and map coordinates
    [[nodiscard]] auto CXMTOF(int64_t x) const -> int
    {
        return static_cast<int>(f_x + MTOF(x - mapLL.x));
    }


    // translates between frame-buffer and map coordinates
    [[nodiscard]] auto CYMTOF(int64_t y) const -> int
    {
        return static_cast<int>(f_y + (f_h - MTOF(y - mapLL.y)));
    }

    //
    //
    auto AM_activateNewScale() -> void
    {
        mapLL += mapWH.half();
        mapWH.x = FTOM(f_w);
        mapWH.y = FTOM(f_h);
        mapLL -= mapWH.half();
        mapUR = mapLL + mapWH;
    }

    //
    inline auto AM_saveScaleAndLoc() -> void
    {
        old_map_ll = mapLL;
        old_map_wh = mapWH;
    }

    //
    auto AM_restoreScaleAndLoc() -> void
    {
        mapWH = old_map_wh;
        if (followplayer == 0)
        {
            mapLL = old_map_ll;
        }
        else
        {
            mapLL = mpoint_t { plr->mo->x, plr->mo->y } - mapWH.half();
        }
        mapUR = mapLL + mapWH;

        // Change the scaling multipliers
        scale_mtof = FixedDiv(static_cast<unsigned int>(f_w) << FRACBITS, mapWH.x);
        scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    }

    //
    // adds a marker at the current location
    //
    auto AM_addMark() -> void
    {
        // [crispy] keep the map static in overlay mode
        // if not following the player
        if (!(!followplayer && crispy->automapoverlay))
        {
            markpoints[markpointnum] = mapLL + mapWH.half();
        }
        else
        {
            markpoints[markpointnum].x = plr->mo->x;
            markpoints[markpointnum].y = plr->mo->y;
        }
        markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;
    }


    // Determines bounding box of all vertices,
    // sets global variables controlling zoom range.
    auto AM_findMinMaxBoundaries() -> void
    {
        min_x = min_y = std::numeric_limits<int>::max();
        max_x = max_y = -std::numeric_limits<int>::min();

        auto vertex_span = std::span<vertex_t>(vertexes, numvertexes);

        for (auto &i : vertex_span)
        {
            min_x = std::min(min_x, i.x);
            max_x = std::max(max_x, i.x);
            min_y = std::min(min_y, i.y);
            max_y = std::max(max_y, i.y);
        }

        // [crispy] cope with huge level dimensions which span the entire INT range
        max_w = max_x / 2 - min_x / 2;
        max_h = max_y / 2 - min_y / 2;

        const fixed_t a = FixedDiv(f_w << FRACBITS, max_w);
        const fixed_t b = FixedDiv(f_h << FRACBITS, max_h);

        min_scale_mtof = std::min(a, b) / 2;
        max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * PLAYERRADIUS);
    }

    //
    // Rotation in 2D.
    // Used to rotate player arrow line character.
    auto AM_rotate(mpoint_t &mp, const angle_t a) -> void
    {
        static double  dSin  = 0.0;
        static double  dCos  = 1.0;
        static angle_t old_a = 0;
        if (old_a != a)
        {
            constexpr double  res     = 2147483648;
            constexpr angle_t off     = 262144;
            constexpr double  radians = std::numbers::pi / res;
            const double      ra      = radians * static_cast<double>(a + off);
            old_a                     = a;
            dSin                      = sin(ra);
            dCos                      = cos(ra);
        }
        const auto dx = static_cast<double>(mp.x);
        const auto dy = static_cast<double>(mp.y);

        mp = { .x = static_cast<fixed_t>((dx * dCos) - (dy * dSin)),
            .y    = static_cast<fixed_t>((dx * dSin) + (dy * dCos)) };
    }

    // [crispy] rotate point around map center
    // adapted from prboom-plus/src/am_map.c:898-920
    auto AM_rotatePoint(mpoint_t &pt) -> void
    {
        pt -= mapcenter;
        AM_rotate(pt, mapangle);
        pt += mapcenter;
    }


    //
    auto AM_changeWindowLoc() -> void
    {
        if (m_paninc.x || m_paninc.y)
        {
            followplayer = 0;
            f_oldloc.x   = std::numeric_limits<int>::max();
        }

        auto inc = m_paninc;
        if (crispy->automaprotate != 0)
        {
            AM_rotate(inc, -mapangle);
        }
        mapLL += inc;


        const auto half = mapWH.half();
        mapLL.x         = std::clamp(mapLL.x, min_x - half.x, max_x - half.x);
        mapLL.y         = std::clamp(mapLL.y, min_y - half.y, max_y - half.y);

        mapUR = mapLL + mapWH;

        // [crispy] reset after moving with the mouse
        if (f_oldloc.y == std::numeric_limits<int>::max())
        {
            m_paninc = { 0, 0 };
        }
    }


    //
    auto AM_initVariables() -> void
    {
        static event_t st_notify = { evtype_t::ev_keyup, globals::AM_MSGENTERED, 0, 0 };

        globals::automapactive = true;
        //  fb = I_VideoBuffer; // [crispy] simplify

        f_oldloc.x = std::numeric_limits<int>::max();
        amclock    = 0;
        lightlev   = 0;

        m_paninc.x   = 0;
        m_paninc.y   = 0;
        ftom_zoommul = FRACUNIT;
        mtof_zoommul = FRACUNIT;

        mapWH.x = FTOM(f_w);
        mapWH.y = FTOM(f_h);

        // find player to center on initially
        if (playeringame[consoleplayer])
        {
            plr = &players[consoleplayer];
        }
        else
        {
            plr = &players[0];

            for (int pnum = 0; pnum < MAXPLAYERS; pnum++)
            {
                if (playeringame[pnum])
                {
                    plr = &players[pnum];
                    break;
                }
            }
        }

        mapLL = (mpoint_t { plr->mo->x, plr->mo->y }) - mapWH.half();
        AM_changeWindowLoc();

        // for saving & restoring
        old_map_ll = mapLL;
        old_map_wh = mapWH;

        // inform the status bar of the change
        ST_Responder(&st_notify);
    }

    //
    auto AM_loadPics() -> void
    {
        int index = 0;
        for (auto &i : marknums)
        {
            char namebuf[9];
            DEH_snprintf(namebuf, 9, "AMMNUM%d", index++);
            i = cache_lump_name<patch_t *>(namebuf, PU::STATIC);
        }
    }


    auto AM_unloadPics() -> void
    {
        for (int i = 0; i < 10; i++)
        {
            char namebuf[9];
            DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
            W_ReleaseLumpName(namebuf);
        }
    }


    // set the window scale to the maximum size
    auto AM_minOutWindowScale() -> void
    {
        scale_mtof = min_scale_mtof;
        scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
        AM_activateNewScale();
    }


    auto AM_clearMarks() -> void
    {
        for (auto &i : markpoints)
        {
            i.x = -1; // means empty
        }
        markpointnum = 0;
    }

    // set the window scale to the minimum size
    auto AM_maxOutWindowScale() -> void
    {
        scale_mtof = max_scale_mtof;
        scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
        AM_activateNewScale();
    }


    // Zooming
    auto AM_changeWindowScale() -> void
    {
        // Change the scaling multipliers
        scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
        scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

        // [crispy] reset after zooming with the mouse wheel
        if (ftom_zoommul == M2_ZOOMIN || ftom_zoommul == M2_ZOOMOUT)
        {
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
        }

        if (scale_mtof < min_scale_mtof)
        {
            AM_minOutWindowScale();
        }
        else if (scale_mtof > max_scale_mtof)
        {
            AM_maxOutWindowScale();
        }
        else
        {
            AM_activateNewScale();
        }
    }


    //
    auto AM_dofollowplayer() -> void
    {
        if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
        {
            const auto half = mapWH.half();
            mapLL.x         = FTOM(MTOF(plr->mo->x)) - half.x;
            mapLL.y         = FTOM(MTOF(plr->mo->y)) - half.y;
            mapUR           = mapLL + mapWH;
            f_oldloc.x      = plr->mo->x;
            f_oldloc.y      = plr->mo->y;
        }
    }

    // Clear automap frame buffer.
    auto AM_clearFB(const int color) -> void
    {
        memset(I_VideoBuffer, color, f_w * f_h * sizeof(*I_VideoBuffer));
    }

    // [crispy] keyed linedefs (PR, P1, SR, S1)
    auto AM_DoorColor(const int type) -> keycolor_t
    {
        keycolor_t oc = no_key;
        if (crispy->b_extautomap())
        {
            switch (type)
            {
            case 26:
            case 32:
            case 99:
            case 133: {
                oc = blue_key;
                break;
            }
            case 27:
            case 34:
            case 136:
            case 137: {
                oc = yellow_key;
                break;
            }
            case 28:
            case 33:
            case 134:
            case 135: {
                oc = red_key;
                break;
            }
            default: {
                oc = no_key;
                break;
            }
            }
        }
        return oc;
    }

    //
    // should be called at the start of every level
    // right now, i figure it out myself
    //
    auto AM_LevelInit() -> void
    {
        //leveljuststarted = 0;

        f_x = 0;
        f_y = 0;
        f_w = SCREENWIDTH;
        f_h = SCREENHEIGHT;
        // [crispy] automap without status bar in widescreen mode
        if (crispy->widescreen == 0)
        {
            f_h -= (ST_HEIGHT << crispy->hires);
        }

        AM_clearMarks();

        AM_findMinMaxBoundaries();
        // [crispy] initialize zoomlevel on all maps so that a 4096 units
        // square map would just fit in (MAP01 is 3376x3648 units)
        //const fixed_t a = FixedDiv(locals.f_w, (locals.max_w >> FRACBITS < 2048) ? 2 * (locals.max_w >> FRACBITS) : 4096);
        //const fixed_t b = FixedDiv(locals.f_h, (locals.max_h >> FRACBITS < 2048) ? 2 * (locals.max_h >> FRACBITS) : 4096);
        const fixed_t a = std::min(max_w >> FRACBITS, 2048) * 2;
        const fixed_t b = std::min(max_h >> FRACBITS, 2048) * 2;
        scale_mtof      = FixedDiv(std::min(a, b), static_cast<int>(0.7 * FRACUNIT));
        if (scale_mtof > max_scale_mtof)
        {
            scale_mtof = min_scale_mtof;
        }
        scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    }

    auto AM_ReInit() -> void
    {
        f_w = SCREENWIDTH;
        f_h = SCREENHEIGHT;
        // [crispy] automap without status bar in widescreen mode
        if (!crispy->widescreen)
        {
            f_h -= (ST_HEIGHT << crispy->hires);
        }

        AM_findMinMaxBoundaries();

        scale_mtof = crispy->hires ? scale_mtof * 2 : scale_mtof / 2;
        if (scale_mtof > max_scale_mtof)
            scale_mtof = min_scale_mtof;
        scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    }

    auto AM_Stop() -> void
    {
        static event_t st_notify = { evtype_t::ev_keyup, globals::AM_MSGEXITED, 0 };

        AM_unloadPics();
        globals::automapactive = false;
        ST_Responder(&st_notify);
        stopped = true;
    }

    auto AM_Start() -> void
    {
        if (!stopped) { AM_Stop(); };
        stopped = false;
        if (lastlevel != gamemap || lastepisode != gameepisode)
        {
            AM_LevelInit();
            lastlevel   = gamemap;
            lastepisode = gameepisode;
        }
        // [crispy] reset IDDT cheat when re-starting map during demo recording
        else if (demorecording)
        {
            cheating = 0;
        }
        AM_initVariables();
        AM_loadPics();
    }

    // Handle events (user inputs) in automap mode
    auto AM_Responder(event_t *ev) -> bool
    {
        static int bigstate = 0;

        bool rc = false;

        if (ev->type == evtype_t::ev_joystick && joybautomap >= 0
            && (static_cast<unsigned int>(ev->data1) & (1 << static_cast<unsigned int>(joybautomap))) != 0)
        {
            constexpr unsigned int waitTime = 5;
            joywait                         = I_GetTime() + waitTime;

            if (!globals::automapactive)
            {
                AM_Start();
                viewactive = false;
            }
            else
            {
                bigstate   = 0;
                viewactive = true;
                AM_Stop();
            }

            return true;
        }

        if (!globals::automapactive)
        {
            if (ev->type == evtype_t::ev_keydown && ev->data1 == key_map_toggle)
            {
                AM_Start();
                viewactive = false;
                rc         = true;
            }
        }
        // [crispy] zoom and move Automap with the mouse (wheel)
        else if (ev->type == evtype_t::ev_mouse && !crispy->automapoverlay && !globals::doom::menuactive && !globals::doom::inhelpscreens)
        {
            if (mousebprevweapon >= 0 && ev->data1 & (1 << mousebprevweapon))
            {
                mtof_zoommul = M2_ZOOMOUT;
                ftom_zoommul = M2_ZOOMIN;
                rc           = true;
            }
            else if (mousebnextweapon >= 0 && ev->data1 & (1 << mousebnextweapon))
            {
                mtof_zoommul = M2_ZOOMIN;
                ftom_zoommul = M2_ZOOMOUT;
                rc           = true;
            }
            else if (!followplayer && (ev->data2 || ev->data3))
            {
                // [crispy] mouse sensitivity for strafe
                m_paninc.x = FTOM(ev->data2 * (mouseSensitivity_x2 + 5) / 80);
                m_paninc.y = FTOM(ev->data3 * (mouseSensitivity_x2 + 5) / 80);
                f_oldloc.y = std::numeric_limits<int>::max();
                rc         = true;
            }
        }
        else if (ev->type == evtype_t::ev_keydown)
        {
            rc       = true;
            auto key = ev->data1;

            if (key == key_map_east) // pan right
            {
                // [crispy] keep the map static in overlay mode
                // if not following the player
                if (!followplayer && !crispy->automapoverlay)
                {
                    m_paninc.x = crispy->fliplevels ? -FTOM(F_PANINC) : FTOM(F_PANINC);
                }
                else
                {
                    rc = false;
                }
            }
            else if (key == key_map_west) // pan left
            {
                if ((followplayer == 0) && (crispy->automapoverlay == 0))
                {
                    m_paninc.x = crispy->fliplevels ? FTOM(F_PANINC) : -FTOM(F_PANINC);
                }
                else
                {
                    rc = false;
                }
            }
            else if (key == key_map_north) // pan up
            {
                if ((followplayer == 0) && (crispy->automapoverlay == 0))
                {
                    m_paninc.y = FTOM(F_PANINC);
                }
                else
                {
                    rc = false;
                }
            }
            else if (key == key_map_south) // pan down
            {
                if ((followplayer == 0) && (crispy->automapoverlay == 0))
                {
                    m_paninc.y = -FTOM(F_PANINC);
                }
                else
                {
                    rc = false;
                }
            }
            else if (key == key_map_zoomout) // zoom out
            {
                mtof_zoommul = M_ZOOMOUT;
                ftom_zoommul = M_ZOOMIN;
            }
            else if (key == key_map_zoomin) // zoom in
            {
                mtof_zoommul = M_ZOOMIN;
                ftom_zoommul = M_ZOOMOUT;
            }
            else if (key == key_map_toggle)
            {
                bigstate   = 0;
                viewactive = true;
                AM_Stop();
            }
            else if (key == key_map_maxzoom)
            {
                bigstate = (bigstate == 0);
                if (bigstate != 0)
                {
                    AM_saveScaleAndLoc();
                    AM_minOutWindowScale();
                }
                else
                {
                    AM_restoreScaleAndLoc();
                }
            }
            else if (key == key_map_follow)
            {
                followplayer = (followplayer == 0);
                f_oldloc.x   = std::numeric_limits<int>::max();
                if (followplayer != 0)
                {
                    plr->message = DEH_String(AMSTR_FOLLOWON);
                }
                else
                {
                    plr->message = DEH_String(AMSTR_FOLLOWOFF);
                }
            }
            else if (key == key_map_grid)
            {
                grid = (grid == 0);
                if (grid != 0)
                {
                    plr->message = DEH_String(AMSTR_GRIDON);
                }
                else
                {
                    plr->message = DEH_String(AMSTR_GRIDOFF);
                }
            }
            else if (key == key_map_mark)
            {
                constexpr size_t                    bufferSize = 20;
                static std::array<char, bufferSize> buffer;
                M_snprintf(buffer.data(), bufferSize, "%s %d",
                    DEH_String(AMSTR_MARKEDSPOT), markpointnum);
                plr->message = buffer.data();
                AM_addMark();
            }
            else if (key == key_map_clearmark)
            {
                AM_clearMarks();
                plr->message = DEH_String(AMSTR_MARKSCLEARED);
            }
            else if (key == key_map_overlay)
            {
                // [crispy] force redraw status bar
                globals::doom::inhelpscreens = true;

                crispy->automapoverlay = !crispy->automapoverlay;
                if (crispy->automapoverlay)
                    plr->message = DEH_String(AMSTR_OVERLAYON);
                else
                    plr->message = DEH_String(AMSTR_OVERLAYOFF);
            }
            else if (key == key_map_rotate)
            {
                crispy->automaprotate = !crispy->automaprotate;
                if (crispy->automaprotate)
                    plr->message = DEH_String(AMSTR_ROTATEON);
                else
                    plr->message = DEH_String(AMSTR_ROTATEOFF);
            }
            else
            {
                rc = false;
            }

            if (((deathmatch == 0) || gameversion <= exe_doom_1_8)
                && cht_CheckCheat(&globals::cheat_amap, ev->data2) != 0)
            {
                rc       = false;
                cheating = (cheating + 1) % 3;
            }
        }
        else if (ev->type == evtype_t::ev_keyup)
        {
            rc       = false;
            auto key = ev->data1;

            if (key == key_map_east)
            {
                if (followplayer == 0) { m_paninc.x = 0; }
            }
            else if (key == key_map_west)
            {
                if (followplayer == 0) { m_paninc.x = 0; }
            }
            else if (key == key_map_north)
            {
                if (followplayer == 0) { m_paninc.y = 0; }
            }
            else if (key == key_map_south)
            {
                if (followplayer == 0) { m_paninc.y = 0; }
            }
            else if (key == key_map_zoomout || key == key_map_zoomin)
            {
                mtof_zoommul = FRACUNIT;
                ftom_zoommul = FRACUNIT;
            }
        }

        return rc;
    }

    auto AM_Ticker() -> void
    {
        if (!globals::automapactive)
            return;

        amclock++;

        if (followplayer)
        {
            AM_dofollowplayer();
        }

        // Change the zoom if necessary
        if (ftom_zoommul != FRACUNIT)
        {
            AM_changeWindowScale();
        }

        // Change x,y location
        if (m_paninc.x || m_paninc.y)
        {
            AM_changeWindowLoc();
        }

        // Update light level
        // AM_updateLightLev();

        // [crispy] required for AM_rotatePoint()
        if (crispy->automaprotate)
        {
            mapcenter = mapLL + mapWH.half();
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!(!followplayer && crispy->automapoverlay))
            {
                mapangle = ANG90 - viewangle;
            }
        }
    }

    // Automap clipping of lines.
    //
    // Based on Cohen-Sutherland clipping algorithm but with a slightly
    // faster reject and precalculated slopes.  If the speed is needed,
    // use a hash algorithm to handle  the common cases.
    auto AM_clipMline(mline_t *ml, fline_t *fl) -> bool
    {
        constexpr auto noBit     = std::bitset<4>(0b0000);
        constexpr auto topBit    = std::bitset<4>(0b0001);
        constexpr auto bottomBit = std::bitset<4>(0b0010);
        constexpr auto leftBit   = std::bitset<4>(0b0100);
        constexpr auto rightBit  = std::bitset<4>(0b1000);

        auto edge_code = [noBit](const auto val, const auto lo, const auto hi, const auto lo_flag, const auto hi_flag) {
            return (val < lo) ? lo_flag : (val >= hi) ? hi_flag :
                                                        noBit;
        };

        auto get_edge_code = [this, edge_code, topBit, bottomBit, leftBit, rightBit](const fpoint_t m) {
            auto oc = edge_code(m.y, 0, f_h, topBit, bottomBit);
            oc |= edge_code(m.x, 0, f_w, leftBit, rightBit);
            return oc;
        };

        // do trivial rejects and outcodes
        const auto a_top_bottom = edge_code(ml->a.y, mapLL.y, mapUR.y, bottomBit, topBit);
        const auto b_top_bottom = edge_code(ml->b.y, mapLL.y, mapUR.y, bottomBit, topBit);
        if ((a_top_bottom & b_top_bottom).any())
        {
            return false; // trivially outside
        }

        const auto a_edges = a_top_bottom | edge_code(ml->a.x, mapLL.x, mapUR.x, leftBit, rightBit);
        const auto b_edges = b_top_bottom | edge_code(ml->b.x, mapLL.x, mapUR.x, leftBit, rightBit);
        if ((a_edges & b_edges).any())
        {
            return false; // trivially outside
        }

        // transform to frame-buffer coordinates.
        fl->a.x = CXMTOF(ml->a.x);
        fl->a.y = CYMTOF(ml->a.y);
        fl->b.x = CXMTOF(ml->b.x);
        fl->b.y = CYMTOF(ml->b.y);

        auto outcode1 = get_edge_code(fl->a);
        auto outcode2 = get_edge_code(fl->b);

        if ((outcode1 & outcode2).any())
        {
            return false;
        }

        while ((outcode1 | outcode2).any())
        {
            // may be partially inside box
            // find an outside point
            const auto outside = (outcode1.any()) ? outcode1 : outcode2;

            const auto dx = fl->a.x - fl->b.x;
            const auto dy = fl->a.y - fl->b.y;

            fpoint_t tmp = { 0, 0 };
            // clip to each side
            if ((outside & topBit).any())
            {
                ASSERT(dy != 0, "Divide by zero in top trim.  dy is zero");
                tmp = { fl->a.x - (dx * fl->a.y) / dy, 0 };
            }
            else if ((outside & bottomBit).any())
            {
                ASSERT(dy != 0, "Divide by zero in bottom trim.  dy is zero");
                tmp = { fl->a.x - (dx * (fl->a.y - f_h)) / dy, f_h - 1 };
            }
            else if ((outside & rightBit).any())
            {
                ASSERT(dx != 0, "Divide by zero in right trim.  dx is zero");
                tmp = { f_w - 1, fl->a.y - (dy * (f_w - 1 - fl->a.x)) / -dx };
            }
            else if ((outside & leftBit).any())
            {
                ASSERT(dx != 0, "Divide by zero in left trim.  dx is zero");
                tmp = { 0, fl->a.y - (dy * (-fl->a.x)) / -dx };
            }


            if (outside == outcode1)
            {
                fl->a    = tmp;
                outcode1 = get_edge_code(fl->a);
            }
            else
            {
                fl->b    = tmp;
                outcode2 = get_edge_code(fl->b);
            }

            if ((outcode1 & outcode2).any())
            {
                return false; // trivially outside
            }
        }
        return true;
    }

    //
    // Classic Bresenham w/ whatever optimizations needed for speed
    //
    void AM_drawFline(fline_t *fl,
        const int              color)
    {
        // For debugging only
        if (fl->a.x < 0 || fl->a.x >= f_w
            || fl->a.y < 0 || fl->a.y >= f_h
            || fl->b.x < 0 || fl->b.x >= f_w
            || fl->b.y < 0 || fl->b.y >= f_h)
        {
            static int fuck = 0;
            DEH_fprintf(stderr, "fuck %d \r", fuck++);
            return;
        }

        const auto PUTDOT = [=, this](const auto xx, const auto yy, const auto cc) {
            I_VideoBuffer[yy * f_w + flipscreenwidth[xx]] = colormaps[cc];
        };

        const int dx = fl->b.x - fl->a.x;
        const int ax = 2 * (dx < 0 ? -dx : dx);
        const int sx = dx < 0 ? -1 : 1;

        const int dy = fl->b.y - fl->a.y;
        const int ay = 2 * (dy < 0 ? -dy : dy);
        const int sy = dy < 0 ? -1 : 1;

        int x = fl->a.x;
        int y = fl->a.y;

        if (ax > ay)
        {
            int d = ay - ax / 2;
            while (1)
            {
                PUTDOT(x, y, color);
                if (x == fl->b.x) { break; }
                if (d >= 0)
                {
                    y += sy;
                    d -= ax;
                }
                x += sx;
                d += ay;
            }
        }
        else
        {
            int d = ax - ay / 2;
            while (1)
            {
                PUTDOT(x, y, color);
                if (y == fl->b.y) { break; }
                if (d >= 0)
                {
                    x += sx;
                    d -= ay;
                }
                y += sy;
                d += ax;
            }
        }
    }

    //
    // Clip lines, draw visible part sof lines.
    //
    void AM_drawMline(mline_t *ml, int color)
    {
        static fline_t fl;

        if (AM_clipMline(ml, &fl))
        {
            AM_drawFline(&fl, color); // draws it on frame buffer using I_VideoBuffer coords
        }
    }

    //
    // Draws flat (floor/ceiling tile) aligned grid lines.
    //
    void AM_drawGrid(int color)
    {
        // Figure out start of vertical gridlines
        auto start = mapLL.x;
        if (crispy->automaprotate)
        {
            start -= mapWH.y / 2;
        }
        // [crispy] fix losing grid lines near the automap boundary
        if ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS))
        {
            start += // (MAPBLOCKUNITS<<FRACBITS)
                -((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS));
        }
        auto end = mapLL.x + mapWH.x;
        if (crispy->automaprotate)
        {
            end += mapWH.y / 2;
        }

        // draw vertical gridlines
        for (auto x = start; x < end; x += (MAPBLOCKUNITS << FRACBITS))
        {
            // [crispy] moved here
            mline_t ml = {
                .a = { x, mapLL.y },          //
                .b = { x, mapLL.y + mapWH.y } //
            };

            if (crispy->automaprotate)
            {
                const auto half_x = mapWH.x / 2;
                ml.a.y -= half_x;
                ml.b.y += half_x;
                AM_rotatePoint(ml.a);
                AM_rotatePoint(ml.b);
            }
            AM_drawMline(&ml, color);
        }

        // Figure out start of horizontal gridlines
        start = mapLL.y;
        if (crispy->automaprotate)
        {
            start -= mapWH.x / 2;
        }
        // [crispy] fix losing grid lines near the automap boundary
        if ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS))
        {
            start += // (MAPBLOCKUNITS<<FRACBITS)
                -((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS));
        }

        end = mapLL.y + mapWH.y;

        if (crispy->automaprotate)
        {
            end += mapWH.x / 2;
        }

        // draw horizontal gridlines
        for (auto y = start; y < end; y += (MAPBLOCKUNITS << FRACBITS))
        {
            // [crispy] moved here
            mline_t ml = {
                .a = { mapLL.x, y },
                .b = { mapLL.x + mapWH.x, y }
            };

            if (crispy->automaprotate)
            {
                const auto half_y = mapWH.y / 2;
                ml.a.x -= half_y;
                ml.b.x += half_y;
                AM_rotatePoint(ml.a);
                AM_rotatePoint(ml.b);
            }
            AM_drawMline(&ml, color);
        }
    }

    void AM_drawLineCharacter( //
        const std::span<const mline_t> drawIt,
        const fixed_t                  scale,
        const angle_t                  angle,
        const int                      color,
        const mpoint_t                 offset)
    {
        auto tmp_angle = angle;
        if (crispy->automaprotate != 0)
        {
            tmp_angle += mapangle;
        }

        for (const auto &line : drawIt)
        {
            mline_t l = line;

            if (scale != 0)
            {
                l.a *= scale;
                l.b *= scale;
            }

            if (tmp_angle != 0)
            {
                AM_rotate(l.a, tmp_angle);
                AM_rotate(l.b, tmp_angle);
            }

            l.a += offset;
            l.b += offset;

            AM_drawMline(&l, color);
        }
    }

    //
    // Determines visible lines, draws them.
    // This is LineDef based, not LineSeg based.
    //
    auto AM_drawWalls() -> void
    {
        for (int i = 0; i < numlines; i++)
        {
            mline_t l = {
                .a = { lines[i].v1->x, lines[i].v1->y },
                .b = { lines[i].v2->x, lines[i].v2->y }
            };

            if (crispy->automaprotate)
            {
                AM_rotatePoint(l.a);
                AM_rotatePoint(l.b);
            }

            if (cheating || (lines[i].flags & ML_MAPPED))
            {
                if ((lines[i].flags & LINE_NEVERSEE) && !cheating)
                {
                    continue;
                }
                {
                    // [crispy] draw keyed doors in their respective colors
                    // (no Boom multiple keys)
                    keycolor_t amd;
                    if (!(lines[i].flags & ML_SECRET) && (amd = AM_DoorColor(lines[i].special)) > no_key)
                    {
                        switch (amd)
                        {
                        case blue_key:
                            AM_drawMline(&l, BLUES);
                            continue;
                        case yellow_key:
                            AM_drawMline(&l, YELLOWS);
                            continue;
                        case red_key:
                            AM_drawMline(&l, REDS);
                            continue;
                        default:
                            // [crispy] it should be impossible to reach here
                            break;
                        }
                    }
                }
                // [crispy] draw exit lines in white (no Boom exit lines 197, 198)
                // NB: Choco does not have this at all, Boom/PrBoom+ have this disabled by default
                if (crispy->b_extautomap() && (lines[i].special == 11 || lines[i].special == 51 || lines[i].special == 52 || lines[i].special == 124))
                {
                    AM_drawMline(&l, WHITE);
                    continue;
                }
                if (!lines[i].backsector)
                {
                    // [crispy] draw 1S secret sector boundaries in purple
                    if (crispy->b_extautomap() && cheating && (lines[i].frontsector->special == 9))
                        AM_drawMline(&l, SECRETWALLCOLORS);
#ifdef CRISPY_HIGHLIGHT_REVEALED_SECRETS
                    // [crispy] draw revealed secret sector boundaries in green
                    else if (crispy->b_extautomap() && crispy->secretmessage && (lines[i].frontsector->oldspecial == 9))
                        AM_drawMline(&l, REVEALEDSECRETWALLCOLORS);
#endif
                    else
                        AM_drawMline(&l, WALLCOLORS() + lightlev);
                }
                else
                {
                    // [crispy] draw teleporters in green
                    // and also WR teleporters 97 if they are not secret
                    // (no monsters-only teleporters 125, 126; no Boom teleporters)
                    if (lines[i].special == 39 || (crispy->b_extautomap() && !(lines[i].flags & ML_SECRET) && lines[i].special == 97))
                    { // teleporters
                        AM_drawMline(&l, crispy->b_extautomap() ? (GREENS + GREENRANGE / 2) : (WALLCOLORS() + WALLRANGE / 2));
                    }
                    else if (lines[i].flags & ML_SECRET) // secret door
                    {
                        // [crispy] NB: Choco has this check, but (SECRETWALLCOLORS == WALLCOLORS)
                        // Boom/PrBoom+ does not have this check at all
                        if (false && cheating)
                        {
                            AM_drawMline(&l, SECRETWALLCOLORS + lightlev);
                        }
                        else
                        {
                            AM_drawMline(&l, WALLCOLORS() + lightlev);
                        }
                    }
#if defined CRISPY_HIGHLIGHT_REVEALED_SECRETS
                    // [crispy] draw revealed secret sector boundaries in green
                    else if (crispy->b_extautomap() && crispy->secretmessage && (lines[i].backsector->oldspecial == 9 || lines[i].frontsector->oldspecial == 9))
                    {
                        AM_drawMline(&l, REVEALEDSECRETWALLCOLORS);
                    }
#endif
                    // [crispy] draw 2S secret sector boundaries in purple
                    else if (crispy->b_extautomap() && cheating && (lines[i].backsector->special == 9 || lines[i].frontsector->special == 9))
                    {
                        AM_drawMline(&l, SECRETWALLCOLORS);
                    }
                    else if (lines[i].backsector->floorheight
                             != lines[i].frontsector->floorheight)
                    {
                        AM_drawMline(&l, FDWALLCOLORS() + lightlev); // floor level change
                    }
                    else if (lines[i].backsector->ceilingheight
                             != lines[i].frontsector->ceilingheight)
                    {
                        AM_drawMline(&l, CDWALLCOLORS() + lightlev); // ceiling level change
                    }
                    else if (cheating)
                    {
                        AM_drawMline(&l, TSWALLCOLORS + lightlev);
                    }
                }
            }
            else if (plr->powers[pw_allmap])
            {
                if (!(lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, GRAYS + 3);
            }
        }
    }

    void AM_drawPlayers()
    {
        int        i;
        player_t * p;
        static int their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
        int        their_color    = -1;
        int        color;
        mpoint_t   pt;

        if (!netgame)
        {
            pt = { .x = plr->mo->x, .y = plr->mo->y };
            if (crispy->automaprotate != 0)
            {
                AM_rotatePoint(pt);
            }

            if (cheating != 0)
            {
                AM_drawLineCharacter(cheat_player_arrow, 0, plr->mo->angle, WHITE, pt);
            }
            else
            {
                AM_drawLineCharacter(player_arrow, 0, plr->mo->angle, WHITE, pt);
            }
            return;
        }

        for (i = 0; i < MAXPLAYERS; i++)
        {
            their_color++;
            p = &players[i];

            if ((deathmatch && !singledemo) && p != plr)
            {
                continue;
            }

            if (!playeringame[i])
            {
                continue;
            }

            if (p->powers[pw_invisibility])
            {
                color = 246; // *close* to black
            }
            else
            {
                color = their_colors[their_color];
            }

            pt = { p->mo->x, p->mo->y };
            if (crispy->automaprotate)
            {
                AM_rotatePoint(pt);
            }

            AM_drawLineCharacter(player_arrow, 0, p->mo->angle, color, pt);
        }
    }
};

auto Generate_AM_MAP() -> AM_MAP &
{
    static AM_MAP return_me = {};
    return return_me;
}

static auto &locals = Generate_AM_MAP();


// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.
// auto AM_getIslope(mline_t *ml,
//     islope_t *             is) -> void
// {
//     const auto dy        = ml->a.y - ml->b.y;
//     const auto dx        = ml->b.x - ml->a.x;
//     const auto halfSlope = [](auto a, auto b) {
//         if (b == 0)
//         {
//             return (a < 0 ? -INT_MAX : INT_MAX);
//         }
//         return FixedDiv(a, b);
//     };
//
//     is->islp = halfSlope(dx, dy);
//     is->slp  = halfSlope(dy, dx);
// }


auto AM_ReInit() -> void
{
    locals.AM_ReInit();
}

auto AM_Stop() -> void
{
    locals.AM_Stop();
}


// Handle events (user inputs) in automap mode
auto AM_Responder(event_t *ev) -> bool
{
    return locals.AM_Responder(ev);
}


//
// auto AM_updateLightLev() -> void
// {
//     static int nexttic = 0;
//     //static int litelevels[] = { 0, 3, 5, 6, 6, 7, 7, 7 };
//     static int litelevels[]  = { 0, 4, 7, 10, 12, 14, 15, 15 };
//     static int litelevelscnt = 0;
//
//     // Change light level
//     if (locals.amclock > nexttic)
//     {
//         locals.lightlev = litelevels[litelevelscnt++];
//         if (litelevelscnt == std::ssize(litelevels)) litelevelscnt = 0;
//         nexttic = locals.amclock + 6 - (locals.amclock % 6);
//     }
// }


//
// Updates on Game Tick
//
auto AM_Ticker() -> void
{
    locals.AM_Ticker();
}


void AM_drawThings(int colors, int colorrange [[maybe_unused]])
{
    auto sector_span = std::span<sector_t>(sectors, numsectors);
    for (auto &i : sector_span)
    {
        auto *t = i.thinglist;
        while (t != nullptr)
        {
            // [crispy] do not draw an extra triangle for the player
            if (t == locals.plr->mo)
            {
                t = t->snext;
                continue;
            }
            mpoint_t pt = { t->x, t->y };
            if (crispy->automaprotate != 0)
            {
                locals.AM_rotatePoint(pt);
            }

            if (crispy->b_extautomap())
            {
                keycolor_t key;
                // [crispy] skull keys and key cards
                switch (t->info->doomednum)
                {
                case 13:
                case 38:
                    key = red_key;
                    break;
                case 6:
                case 39:
                    key = yellow_key;
                    break;
                case 5:
                case 40:
                    key = blue_key;
                    break;
                default:
                    key = no_key;
                    break;
                }

                // [crispy] draw keys as crosses in their respective colors
                if (key > no_key)
                {
                    locals.AM_drawLineCharacter(locals.cross_mark, 16 << FRACBITS, t->angle,
                        (key == red_key)    ? REDS :
                        (key == yellow_key) ? YELLOWS :
                        (key == blue_key)   ? BLUES :
                                              colors + locals.lightlev,
                        pt);
                }
                // [crispy] draw blood splats and puffs as small squares
                else if (t->type == MT_BLOOD || t->type == MT_PUFF)
                {
                    locals.AM_drawLineCharacter(locals.square_mark,
                        t->radius >> 2, t->angle,
                        (t->type == MT_BLOOD) ? REDS : GRAYS,
                        pt);
                }
                else
                {
                    // [crispy] show countable kills in red ...
                    const auto color = ((t->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL) ? REDS :
                                                                                                   // [crispy] ... show Lost Souls and missiles in orange ...
                                           (t->flags & (MF_FLOAT | MF_MISSILE)) ? 216 :
                                                                                  // [crispy] ... show other shootable items in dark gold ...
                                           (t->flags & MF_SHOOTABLE) ? 164 :
                                                                       // [crispy] ... corpses in gray ...
                                           (t->flags & MF_CORPSE) ? GRAYS :
                                                                    // [crispy] ... and countable items in yellow
                                           (t->flags & MF_COUNTITEM) ? YELLOWS :
                                                                       colors + locals.lightlev;

                    locals.AM_drawLineCharacter(locals.thintriangle_guy,
                        // [crispy] triangle size represents actual thing size
                        t->radius, t->angle,
                        color, pt);
                }
            }
            else
            {
                locals.AM_drawLineCharacter(locals.thintriangle_guy,
                    16 << FRACBITS, t->angle, colors + locals.lightlev, { t->x, t->y });
            }
            t = t->snext;
        }
    }
}

void AM_drawMarks(void)
{
    for (int i = 0; i < AM_NUMMARKPOINTS; i++)
    {
        if (locals.markpoints[i].x != -1)
        {
            //      w = SHORT(locals.marknums[i]->width);
            //      h = SHORT(locals.marknums[i]->height);
            int w = 5; // because something's wrong with the wad, i guess
            int h = 6; // because something's wrong with the wad, i guess
            // [crispy] center marks around player
            auto pt = locals.markpoints[i];
            if (crispy->automaprotate)
            {
                locals.AM_rotatePoint(pt);
            }
            int fx = (flipscreenwidth[locals.CXMTOF(pt.x)] >> crispy->hires) - 1 - DELTAWIDTH;
            int fy = (locals.CYMTOF(pt.y) >> crispy->hires) - 2;
            if (fx >= locals.f_x && fx <= (locals.f_w >> crispy->hires) - w && fy >= locals.f_y && fy <= (locals.f_h >> crispy->hires) - h)
                V_DrawPatch(fx, fy, locals.marknums[i]);
        }
    }
}

void AM_drawCrosshair(int color)
{
    // [crispy] draw an actual crosshair
    if (!locals.followplayer)
    {
        static fline_t h, v;

        if (!h.a.x)
        {
            h.a.x = h.b.x = v.a.x = v.b.x = locals.f_x + locals.f_w / 2;
            h.a.y = h.b.y = v.a.y = v.b.y = locals.f_y + locals.f_h / 2;
            h.a.x -= 2;
            h.b.x += 2;
            v.a.y -= 2;
            v.b.y += 2;
        }

        locals.AM_drawFline(&h, color);
        locals.AM_drawFline(&v, color);
    }
    // [crispy] do not draw the useless dot on the player arrow
    /*
    else
    I_VideoBuffer[(locals.f_w*(locals.f_h+1))/2] = colormaps[color]; // single point for now
    */
}

auto AM_Drawer() -> void
{
    if (!globals::automapactive) { return; }

    if (!crispy->automapoverlay)
    {
        locals.AM_clearFB(BACKGROUND);
    }
    if (locals.grid != 0)
    {
        locals.AM_drawGrid(GRIDCOLORS);
    }
    locals.AM_drawWalls();
    locals.AM_drawPlayers();
    if (locals.cheating == 2)
    {
        AM_drawThings(THINGCOLORS, THINGRANGE);
    }
    AM_drawCrosshair(XHAIRCOLORS);

    AM_drawMarks();

    V_MarkRect(locals.f_x, locals.f_y, locals.f_w, locals.f_h);
}

// [crispy] extended savegames
void AM_GetMarkPoints(int *n, long *p)
{
    *n = locals.markpointnum;
    *p = -1L;

    // [crispy] prevent saving locals.markpoints from previous map
    if (locals.lastlevel == gamemap && locals.lastepisode == gameepisode)
    {
        for (int i = 0; i < AM_NUMMARKPOINTS; i++)
        {
            *p++ = (long)locals.markpoints[i].x;
            *p++ = (locals.markpoints[i].x == -1) ? 0L : (long)locals.markpoints[i].y;
        }
    }
}

void AM_SetMarkPoints(int n, long *p)
{
    locals.AM_LevelInit();
    locals.lastlevel   = gamemap;
    locals.lastepisode = gameepisode;

    locals.markpointnum = n;

    for (auto &i : locals.markpoints)
    {
        i.x = (int64_t)*p++;
        i.y = (int64_t)*p++;
    }
}
