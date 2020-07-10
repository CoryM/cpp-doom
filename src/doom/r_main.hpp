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
//	System specific interface stuff.
//


#ifndef __R_MAIN__
#define __R_MAIN__

#include <vector>

#include "d_player.hpp"
#include "r_data.hpp"


//
// POV related.
//
extern fixed_t viewcos;
extern fixed_t viewsin;

extern int viewwindowx;
extern int viewwindowy;


extern int centerx;
extern int centery;

extern fixed_t centerxfrac;
extern fixed_t centeryfrac;
extern fixed_t projection;

extern int validcount;

extern int linecount;
extern int loopcount;


//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
// Now why not 32 levels here?
// [crispy] parameterized for smooth diminishing lighting
extern int LIGHTLEVELS;
extern int LIGHTSEGSHIFT;
extern int LIGHTBRIGHT;
extern int MAXLIGHTSCALE;
extern int LIGHTSCALESHIFT;
extern int MAXLIGHTZ;
extern int LIGHTZSHIFT;

extern std::vector< std::vector<lighttable_t*> > scalelight;
extern lighttable_t ** scalelightfixed;
extern lighttable_t ***zlight;

extern int           extralight;
extern lighttable_t *fixedcolormap;


// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS 32

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
extern fixed_t fractionaltic;

// Blocky/low detail mode.
//B remove this?
//  0 = high, 1 = low
extern int detailshift;


//
// Function pointers to switch refresh/drawing functions.
// Used to select shadow mode etc.
//
extern void (*colfunc)(void);
extern void (*transcolfunc)(void);
extern void (*basecolfunc)(void);
extern void (*fuzzcolfunc)(void);
extern void (*tlcolfunc)(void);
// No shadow effects on floors.
extern void (*spanfunc)(void);


//
// Utility functions.
int R_PointOnSide(fixed_t x,
    fixed_t               y,
    node_t *              node);

int R_PointOnSegSide(fixed_t x,
    fixed_t                  y,
    seg_t *                  line);

angle_t
    R_PointToAngle(fixed_t x,
        fixed_t            y);

angle_t
    R_PointToAngleCrispy(fixed_t x,
        fixed_t                  y);

angle_t
    R_PointToAngle2(fixed_t x1,
        fixed_t             y1,
        fixed_t             x2,
        fixed_t             y2);

fixed_t
    R_PointToDist(fixed_t x,
        fixed_t           y);


fixed_t R_ScaleFromGlobalAngle(angle_t visangle);

subsector_t *
    R_PointInSubsector(fixed_t x,
        fixed_t                y);

void R_AddPointToBox(int x,
    int                  y,
    fixed_t *            box);


// [AM] Interpolate between two angles.
angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale);

//
// REFRESH - the actual rendering functions.
//

// Called by G_Drawer.
void R_RenderPlayerView(player_t *player);

// Called by startup code.
void R_Init(void);

// Called by M_Responder.
void R_SetViewSize(int blocks, int detail);

#endif
