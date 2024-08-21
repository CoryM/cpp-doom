//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


#ifndef __H2DEF__
#define __H2DEF__

// gamemode/mission
#include "d_mode.hpp"

// ticcmd:
#include "d_ticcmd.hpp"

// all exterior data is defined here
#include "xddefs.hpp"

import m_fixed; //#include "m_fixed.hpp" // fixed_t
#include "tables.hpp"
#include "info.hpp"
//#include "player.hpp"

#define HEXEN_VERSION 110
#define HEXEN_VERSION_TEXT "v1.1"

// if rangecheck is undefined, most parameter validation debugging code
// will not be compiled
#ifndef NORANGECHECKING
#define RANGECHECK
#endif

// Past distributions
//#ifndef VER_ID
//#define VER_ID "DVL"
//#endif
//#define HEXEN_VERSIONTEXT "ID V1.2"
//#define HEXEN_VERSIONTEXT "RETAIL STORE BETA"               // 9/26/95
//#define HEXEN_VERSIONTEXT "DVL BETA 10 05 95" // Used for GT for testing
//#define HEXEN_VERSIONTEXT "DVL BETA 10 07 95" // Just an update for Romero
//#define HEXEN_VERSIONTEXT "FINAL 1.0 (10 13 95)" // Just an update for Romero
//#ifdef RANGECHECK
//#define HEXEN_VERSIONTEXT "Version 1.1 +R "__DATE__" ("VER_ID")"
//#else
//#define HEXEN_VERSIONTEXT "Version 1.1 "__DATE__" ("VER_ID")"
//#endif
#define HEXEN_VERSIONTEXT ((gamemode == shareware) ? \
                           "DEMO 10 16 95" : \
                           "VERSION 1.1 MAR 22 1996 (BCP)")

/*
===============================================================================

						GLOBAL TYPES

===============================================================================
*/

//#define NUMARTIFCTS   28
#define MAXPLAYERS	8

//#define	BT_ATTACK		1
//#define	BT_USE			2
//#define	BT_CHANGE		4       // if true, the next 3 bits hold weapon num
//#define	BT_WEAPONMASK	(8+16+32)
//#define	BT_WEAPONSHIFT	3
//
//#define BT_SPECIAL		128     // game events, not really buttons
//#define	BTS_SAVEMASK	(4+8+16)
//#define	BTS_SAVESHIFT	2
//#define	BT_SPECIALMASK	3
//#define	BTS_PAUSE		1       // pause the game
//#define	BTS_SAVEGAME	2       // save the game at each console
// savegame slot numbers occupy the second byte of buttons

// The top 3 bits of the artifact field in the ticcmd_t struct are used
//              as additional flags 
#define AFLAG_MASK			0x3F
#define AFLAG_SUICIDE		0x40
#define AFLAG_JUMP			0x80

enum gamestate_t
{
    GS_LEVEL,
    GS_INTERMISSION,
    GS_FINALE,
    GS_DEMOSCREEN
};

enum gameaction_t
{
    ga_nothing,
    ga_loadlevel,
    ga_initnew,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_playdemo,
    ga_completed,
    ga_leavemap,
    ga_singlereborn,
    ga_victory,
    ga_worlddone,
    ga_screenshot
};

enum wipe_t
{
    wipe_0,
    wipe_1,
    wipe_2,
    wipe_3,
    wipe_4,
    NUMWIPES,
    wipe_random
};

/*
===============================================================================

							MAPOBJ DATA

===============================================================================
*/

//
// frame flags
//
#define	FF_FULLBRIGHT	0x8000  // flag in thing->frame
#define FF_FRAMEMASK	0x7fff

// --- mobj.flags ---

#define	MF_SPECIAL		1       // call P_SpecialThing when touched
#define	MF_SOLID		2
#define	MF_SHOOTABLE	4
#define	MF_NOSECTOR		8       // don't use the sector links
                                                                        // (invisible but touchable)
#define	MF_NOBLOCKMAP	16      // don't use the blocklinks
                                                                        // (inert but displayable)
#define	MF_AMBUSH		32
#define	MF_JUSTHIT		64      // try to attack right back
#define	MF_JUSTATTACKED	128     // take at least one step before attacking
#define	MF_SPAWNCEILING	256     // hang from ceiling instead of floor
#define	MF_NOGRAVITY	512     // don't apply gravity every tic

// movement flags
#define	MF_DROPOFF		0x400   // allow jumps from high places
#define	MF_PICKUP		0x800   // for players to pick up items
#define	MF_NOCLIP		0x1000  // player cheat
#define	MF_SLIDE		0x2000  // keep info about sliding along walls
#define	MF_FLOAT		0x4000  // allow moves to any height, no gravity
#define	MF_TELEPORT		0x8000  // don't cross lines or look at heights
#define MF_MISSILE		0x10000 // don't hit same species, explode on block

#define	MF_ALTSHADOW	0x20000 // alternate translucent draw
#define	MF_SHADOW		0x40000 // use translucent draw (shadow demons / invis)
#define	MF_NOBLOOD		0x80000 // don't bleed when shot (use puff)
#define	MF_CORPSE		0x100000        // don't stop moving halfway off a step
#define	MF_INFLOAT		0x200000        // floating to a height for a move, don't
                                        // auto float to target's height

#define	MF_COUNTKILL	0x400000        // count towards intermission kill total
#define	MF_ICECORPSE	0x800000        // a frozen corpse (for blasting)

#define	MF_SKULLFLY		0x1000000       // skull in flight
#define	MF_NOTDMATCH	0x2000000       // don't spawn in death match (key cards)

//#define       MF_TRANSLATION  0xc000000       // if 0x4 0x8 or 0xc, use a translation
#define	MF_TRANSLATION	0x1c000000      // use a translation table (>>MF_TRANSHIFT)
#define	MF_TRANSSHIFT	26      // table for player colormaps


// --- mobj.flags2 ---

#define MF2_LOGRAV			0x00000001      // alternate gravity setting
#define MF2_WINDTHRUST		0x00000002      // gets pushed around by the wind
                                                                                // specials
#define MF2_FLOORBOUNCE		0x00000004      // bounces off the floor
#define MF2_BLASTED			0x00000008      // missile will pass through ghosts
#define MF2_FLY				0x00000010      // fly mode is active
#define MF2_FLOORCLIP		0x00000020      // if feet are allowed to be clipped
#define MF2_SPAWNFLOAT		0x00000040      // spawn random float z
#define MF2_NOTELEPORT		0x00000080      // does not teleport
#define MF2_RIP				0x00000100      // missile rips through solid
                                                                                // targets
#define MF2_PUSHABLE		0x00000200      // can be pushed by other moving
                                                                                // mobjs
#define MF2_SLIDE			0x00000400      // slides against walls
#define MF2_ONMOBJ			0x00000800      // mobj is resting on top of another
                                                                                // mobj
#define MF2_PASSMOBJ		0x00001000      // Enable z block checking.  If on,
                                                                                // this flag will allow the mobj to
                                                                                // pass over/under other mobjs.
#define MF2_CANNOTPUSH		0x00002000      // cannot push other pushable mobjs
#define MF2_DROPPED			0x00004000      // dropped by a demon
#define MF2_BOSS			0x00008000      // mobj is a major boss
#define MF2_FIREDAMAGE		0x00010000      // does fire damage
#define MF2_NODMGTHRUST		0x00020000      // does not thrust target when
                                                                                // damaging
#define MF2_TELESTOMP		0x00040000      // mobj can stomp another
#define MF2_FLOATBOB		0x00080000      // use float bobbing z movement
#define MF2_DONTDRAW		0x00100000      // don't generate a vissprite
#define MF2_IMPACT			0x00200000      // an MF_MISSILE mobj can activate
                                                                                // SPAC_IMPACT
#define MF2_PUSHWALL		0x00400000      // mobj can push walls
#define MF2_MCROSS			0x00800000      // can activate monster cross lines
#define MF2_PCROSS			0x01000000      // can activate projectile cross lines
#define MF2_CANTLEAVEFLOORPIC 0x02000000        // stay within a certain floor type
#define MF2_NONSHOOTABLE	0x04000000      // mobj is totally non-shootable,
                                                                                // but still considered solid
#define MF2_INVULNERABLE	0x08000000      // mobj is invulnerable
#define MF2_DORMANT			0x10000000      // thing is dormant
#define MF2_ICEDAMAGE		0x20000000      // does ice damage
#define MF2_SEEKERMISSILE	0x40000000      // is a seeker (for reflection)
#define MF2_REFLECTIVE		0x80000000      // reflects missiles



/* Old Heretic key type
typedef enum
{
	key_yellow,
	key_green,
	key_blue,
	NUMKEYS
} keytype_t;
*/

enum keytype_t
{
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_A,
    KEY_B,
    NUMKEYS
};

enum weapontype_t
{
    WP_FIRST,
    WP_SECOND,
    WP_THIRD,
    WP_FOURTH,
    NUMWEAPONS,
    WP_NOCHANGE
};

enum manatype_t
{
    MANA_1,
    MANA_2,
    NUMMANA,
    MANA_BOTH,
    MANA_NONE
};

#define MAX_MANA	200

#define WPIECE1		1
#define WPIECE2		2
#define WPIECE3		4

struct weaponinfo_t;

//extern weaponinfo_t WeaponInfo[NUMWEAPONS][NUMCLASSES];

enum artitype_t
{
    arti_none,
    arti_invulnerability,
    arti_health,
    arti_superhealth,
    arti_healingradius,
    arti_summon,
    arti_torch,
    arti_egg,
    arti_fly,
    arti_blastradius,
    arti_poisonbag,
    arti_teleportother,
    arti_speed,
    arti_boostmana,
    arti_boostarmor,
    arti_teleport,
    // Puzzle artifacts
    arti_firstpuzzitem,
    arti_puzzskull = arti_firstpuzzitem,
    arti_puzzgembig,
    arti_puzzgemred,
    arti_puzzgemgreen1,
    arti_puzzgemgreen2,
    arti_puzzgemblue1,
    arti_puzzgemblue2,
    arti_puzzbook1,
    arti_puzzbook2,
    arti_puzzskull2,
    arti_puzzfweapon,
    arti_puzzcweapon,
    arti_puzzmweapon,
    arti_puzzgear1,
    arti_puzzgear2,
    arti_puzzgear3,
    arti_puzzgear4,
    NUMARTIFACTS
};

enum powertype_t
{
    pw_None,
    pw_invulnerability,
    pw_allmap,
    pw_infrared,
    pw_flight,
    pw_shield,
    pw_health2,
    pw_speed,
    pw_minotaur,
    NUMPOWERS
};

#define	INVULNTICS (30*35)
#define	INVISTICS (60*35)
#define	INFRATICS (120*35)
#define	IRONTICS (60*35)
#define WPNLEV2TICS (40*35)
#define FLIGHTTICS (60*35)
#define SPEEDTICS (45*35)
#define MORPHTICS (40*35)
#define MAULATORTICS (25*35)

#define MESSAGETICS (4*35)
#define BLINKTHRESHOLD (4*35)

#define NUMINVENTORYSLOTS	NUMARTIFACTS

/*
================
=
= player_t
=
================
*/

#define CF_NOCLIP		1
#define	CF_GODMODE		2
#define	CF_NOMOMENTUM	4       // not really a cheat, just a debug aid

#define	SBARHEIGHT	(39 << crispy->hires)      // status bar height at bottom of screen

/*
===============================================================================

					GLOBAL VARIABLES

===============================================================================
*/

#define TELEFOGHEIGHT (32*FRACUNIT)

extern GameMode_t gamemode;         // Always commercial

extern gameaction_t gameaction;

extern bool paused;

extern bool DevMaps;         // true = map development mode
extern char *DevMapsDir;        // development maps directory

extern bool nomonsters;      // checkparm of -nomonsters

extern bool respawnparm;     // checkparm of -respawn

extern bool randomclass;     // checkparm of -randclass

extern bool debugmode;       // checkparm of -debug

extern bool usergame;        // ok to save / end game

extern bool ravpic;          // checkparm of -ravpic

extern bool altpal;          // checkparm to use an alternate palette routine

extern bool cdrom;           // true if cd-rom mode active ("-cdrom")

extern bool deathmatch;      // only if started as net death

extern bool netgame;         // only true if >1 player

extern bool cmdfrag;         // true if a CMD_FRAG packet should be sent out every
                                                // kill

extern int consoleplayer;       // player taking events and displaying

extern int displayplayer;

extern int viewangleoffset;     // ANG90 = left side, ANG270 = right

extern bool DebugSound;      // debug flag for displaying sound info

extern bool demoplayback;
extern bool demoextend;      // allow demos to persist through exit/respawn
extern int maxzone;             // Maximum chunk allocated for zone heap

// Truncate angleturn in ticcmds to nearest 256.
// Used when recording Vanilla demos in netgames.
extern bool lowres_turn;

extern int Sky1Texture;
extern int Sky2Texture;

extern gamestate_t gamestate;
extern skill_t gameskill;
//extern        bool         respawnmonsters;
extern int gameepisode;
extern int gamemap;
extern int prevmap;
extern int levelstarttic;       // gametic at level start
extern int leveltime;           // tics in game play for par

extern ticcmd_t *netcmds;

#define MAXDEATHMATCHSTARTS 16
extern mapthing_t *deathmatch_p;
extern mapthing_t deathmatchstarts[MAXDEATHMATCHSTARTS];

// Position indicator for cooperative net-play reborn
extern int RebornPosition;

#define MAX_PLAYER_STARTS 8
extern mapthing_t playerstarts[MAX_PLAYER_STARTS][MAXPLAYERS];
extern int maxplayers;

extern int mouseSensitivity;

extern bool precache;        // if true, load all graphics at level load

extern uint8_t *screen;            // off screen work buffer, from V_video.c

extern bool singledemo;      // quit after playing a demo from cmdline

extern int bodyqueslot;
extern skill_t startskill;
extern int startepisode;
extern int startmap;
extern bool autostart;

extern bool testcontrols;
extern int testcontrols_mousespeed;

extern int vanilla_savegame_limit;
extern int vanilla_demo_limit;

//-------
//SV_SAVE
//-------

#define HXS_VERSION_TEXT "HXS Ver 2.37"
#define HXS_VERSION_TEXT_LENGTH 16
#define HXS_DESCRIPTION_LENGTH 24

//-------
//REFRESH
//-------

// define the different areas for the dirty map
#define I_NOUPDATE	0
#define I_FULLVIEW	1
#define I_STATBAR	2
#define I_MESSAGES	4
#define I_FULLSCRN	8

//------------------------------
// SN_sonix.c
//------------------------------

enum
{
    SEQ_PLATFORM,
    SEQ_PLATFORM_HEAVY,         // same script as a normal platform
    SEQ_PLATFORM_METAL,
    SEQ_PLATFORM_CREAK,         // same script as a normal platform
    SEQ_PLATFORM_SILENCE,
    SEQ_PLATFORM_LAVA,
    SEQ_PLATFORM_WATER,
    SEQ_PLATFORM_ICE,
    SEQ_PLATFORM_EARTH,
    SEQ_PLATFORM_METAL2,
    SEQ_DOOR_STONE,
    SEQ_DOOR_HEAVY,
    SEQ_DOOR_METAL,
    SEQ_DOOR_CREAK,
    SEQ_DOOR_SILENCE,
    SEQ_DOOR_LAVA,
    SEQ_DOOR_WATER,
    SEQ_DOOR_ICE,
    SEQ_DOOR_EARTH,
    SEQ_DOOR_METAL2,
    SEQ_ESOUND_WIND,
    SEQ_NUMSEQ
};

enum seqtype_t : int
{
    SEQTYPE_STONE,
    SEQTYPE_HEAVY,
    SEQTYPE_METAL,
    SEQTYPE_CREAK,
    SEQTYPE_SILENCE,
    SEQTYPE_LAVA,
    SEQTYPE_WATER,
    SEQTYPE_ICE,
    SEQTYPE_EARTH,
    SEQTYPE_METAL2,
    SEQTYPE_NUMSEQ
};

struct seqnode_t;

//----------------------
// Interlude (IN_lude.c)
//----------------------

#define MAX_INTRMSN_MESSAGE_SIZE 1024

// ===== Player Class Types =====

struct weaponinfo_t
{
    manatype_t mana;
    int upstate;
    int downstate;
    int readystate;
    int atkstate;
    int holdatkstate;
    int flashstate;
};


#endif // __H2DEF__
