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
// DESCRIPTION:  none
//	Implements special effects:
//	Texture animation, height or lighting changes
//	 according to adjacent sectors, respective
//	 utility functions, etc.
//


#ifndef __P_SPEC__
#define __P_SPEC__

#include "p_mobj.hpp"
#include "p_ceilng.hpp"

//
// End-level timer (-TIMER option)
//
extern bool levelTimer;
extern int  levelTimeCount;


//      Define values for map objects
#define MO_TELEPORTMAN 14


// at game start
void P_InitPicAnims(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);

// when needed
bool P_UseSpecialLine(mobj_t *thing,
    line_s *                  line,
    int                       side);

void P_ShootSpecialLine(mobj_t *thing,
    line_s *                    line);

void P_CrossSpecialLine(int linenum,
    int                     side,
    mobj_t *                thing);

// [crispy] more MBF code pointers
void P_CrossSpecialLinePtr(line_s *line,
    int                            side,
    mobj_t *                       thing);

void P_PlayerInSpecialSector(player_t *player);

int twoSided(int sector,
    int          line);

sector_t *
    getSector(int currentSector,
        int       line,
        int       side);

side_t *
    getSide(int currentSector,
        int     line,
        int     side);

fixed_t P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);

fixed_t
    P_FindNextHighestFloor(sector_t *sec,
        int                          currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);

int P_FindSectorFromLineTag(line_s *line,
    int                             start);

int P_FindMinSurroundingLight(sector_t *sector,
    int                                 max);

sector_t *
    getNextSector(line_s *line,
        sector_t *        sec);


//
// SPECIAL
//
int EV_DoDonut(line_s *line);


//
// P_LIGHTS
//
typedef struct
{
    thinker_s thinker;
    sector_t *sector;
    int       count;
    int       maxlight;
    int       minlight;

} fireflicker_t;


typedef struct
{
    thinker_s thinker;
    sector_t *sector;
    int       count;
    int       maxlight;
    int       minlight;
    int       maxtime;
    int       mintime;

} lightflash_t;


typedef struct
{
    thinker_s thinker;
    sector_t *sector;
    int       count;
    int       minlight;
    int       maxlight;
    int       darktime;
    int       brighttime;

} strobe_t;


typedef struct
{
    thinker_s thinker;
    sector_t *sector;
    int       minlight;
    int       maxlight;
    int       direction;

} glow_t;


#define GLOWSPEED    8
#define STROBEBRIGHT 5
#define FASTDARK     15
#define SLOWDARK     35

void P_SpawnFireFlicker(sector_t *sector);
void T_LightFlash(lightflash_t *flash);
void P_SpawnLightFlash(sector_t *sector);
void T_StrobeFlash(strobe_t *flash);

void P_SpawnStrobeFlash(sector_t *sector,
    int                           fastOrSlow,
    int                           inSync);

void EV_StartLightStrobing(line_s *line);
void EV_TurnTagLightsOff(line_s *line);

void EV_LightTurnOn(line_s *line,
    int                     bright);

void T_Glow(glow_t *g);
void P_SpawnGlowingLight(sector_t *sector);


//
// P_SWITCH
//
// [crispy] add PACKEDATTR for reading SWITCHES lumps from memory
typedef PACKED_STRUCT(
    {
        char  name1[9];
        char  name2[9];
        short episode;
    }) switchlist_t;


typedef enum
{
    top,
    middle,
    bottom

} bwhere_e;


typedef struct
{
    line_s *     line;
    bwhere_e     where;
    int          btexture;
    int          btimer;
    degenmobj_t *soundorg;

} button_t;


// max # of wall switches in a level
#define MAXSWITCHES 50

// 4 players, 4 buttons each at once, max.
#define MAXBUTTONS 16

// 1 second, in ticks.
#define BUTTONTIME 35

extern button_t *buttonlist;
extern int       maxbuttons;

void P_ChangeSwitchTexture(line_s *line,
    int                            useAgain);

void P_InitSwitchList(void);


//
// P_PLATS
//
typedef enum
{
    up,
    down,
    waiting,
    in_stasis

} plat_e;


enum plattype_e
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS

};


struct plat_t {
    thinker_s  thinker;
    sector_t * sector;
    fixed_t    speed;
    fixed_t    low;
    fixed_t    high;
    int        wait;
    int        count;
    plat_e     status;
    plat_e     oldstatus;
    bool       crush;
    int        tag;
    plattype_e type;
};


#define PLATWAIT  3
#define PLATSPEED FRACUNIT
#define MAXPLATS  30 * 256


extern plat_t *activeplats[MAXPLATS];

void T_PlatRaise(plat_t *plat);

int EV_DoPlat(line_s *line,
    plattype_e        type,
    int               amount);

void P_AddActivePlat(plat_t *plat);
void P_RemoveActivePlat(plat_t *plat);
void EV_StopPlat(line_s *line);
void P_ActivateInStasis(int tag);


//
// P_DOORS
enum vldoor_e
{
    vld_normal,
    vld_close30ThenOpen,
    vld_close,
    vld_open,
    vld_raiseIn5Mins,
    vld_blazeRaise,
    vld_blazeOpen,
    vld_blazeClose

};


struct vldoor_t {
    thinker_s thinker;
    vldoor_e  type;
    sector_t *sector;
    fixed_t   topheight;
    fixed_t   speed;

    // 1 = up, 0 = waiting at top, -1 = down
    int direction;

    // tics to wait at the top
    int topwait;

    // (keep in case a door going down is reset)
    // when it reaches 0, start going down
    int topcountdown;
};


#define VDOORSPEED FRACUNIT * 2
#define VDOORWAIT  150

void EV_VerticalDoor(line_s *line,
    mobj_t *                 thing);

int EV_DoDoor(line_s *line,
    vldoor_e          type);

int EV_DoLockedDoor(line_s *line,
    vldoor_e                type,
    mobj_t *                thing);

void T_VerticalDoor(vldoor_t *door);
void P_SpawnDoorCloseIn30(sector_t *sec);

void P_SpawnDoorRaiseIn5Mins(sector_t *sec,
    int                                secnum);


#if 0 // UNUSED
//
//      Sliding doors...
//
typedef enum
{
    sd_opening,
    sd_waiting,
    sd_closing

} sd_e;



typedef enum
{
    sdt_openOnly,
    sdt_closeOnly,
    sdt_openAndClose

} sdt_e;




typedef struct
{
    thinker_s	thinker;
    sdt_e	type;
    line_s*	line;
    int		frame;
    int		whichDoorIndex;
    int		timer;
    sector_t*	frontsector;
    sector_t*	backsector;
    sd_e	 status;

} slidedoor_t;



typedef struct
{
    char	frontFrame1[9];
    char	frontFrame2[9];
    char	frontFrame3[9];
    char	frontFrame4[9];
    char	backFrame1[9];
    char	backFrame2[9];
    char	backFrame3[9];
    char	backFrame4[9];

} slidename_t;



typedef struct
{
    int             frontFrames[4];
    int             backFrames[4];

} slideframe_t;



// how many frames of animation
#define SNUMFRAMES 4

#define SDOORWAIT     35 * 3
#define SWAITTICS     4

// how many diff. types of anims
#define MAXSLIDEDOORS 5

void P_InitSlidingDoorFrames(void);

void
EV_SlidingDoor
( line_s*	line,
  mobj_t*	thing );
#endif // unused


//
// P_FLOOR
//
typedef enum
{
    // lower floor to highest surrounding floor
    lowerFloor,

    // lower floor to lowest surrounding floor
    lowerFloorToLowest,

    // lower floor to highest surrounding floor VERY FAST
    turboLower,

    // raise floor to lowest surrounding CEILING
    raiseFloor,

    // raise floor to next highest surrounding floor
    raiseFloorToNearest,

    // raise floor to shortest height texture around it
    raiseToTexture,

    // lower floor to lowest surrounding floor
    //  and change floorpic
    lowerAndChange,

    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,

    // raise to next highest floor, turbo-speed
    raiseFloorTurbo,
    donutRaise,
    raiseFloor512

} floor_e;


typedef enum
{
    build8, // slowly build by 8
    turbo16 // quickly build by 16

} stair_e;


typedef struct
{
    thinker_s thinker;
    floor_e   type;
    bool      crush;
    sector_t *sector;
    int       direction;
    int       newspecial;
    short     texture;
    fixed_t   floordestheight;
    fixed_t   speed;

} floormove_t;


#define FLOORSPEED FRACUNIT

enum class result_e
{
    ok,
    crushed,
    pastdest
};
result_e operator&(const result_e &a, const result_e &b);


result_e
    T_MovePlane(sector_t *sector,
        fixed_t           speed,
        fixed_t           dest,
        bool              crush,
        int               floorOrCeiling,
        int               direction);

int EV_BuildStairs(line_s *line,
    stair_e                type);

int EV_DoFloor(line_s *line,
    floor_e            floortype);

void T_MoveFloor(floormove_t *floor);

//
// P_TELEPT
//
int EV_Teleport(line_s *line,
    int                 side,
    mobj_t *            thing);

#endif
