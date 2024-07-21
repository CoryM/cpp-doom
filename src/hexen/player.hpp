#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "mobj.hpp"
#include "h2def.hpp"

// ===== Player Class Types =====
enum pclass_t
{
    PCLASS_FIGHTER,
    PCLASS_CLERIC,
    PCLASS_MAGE,
    PCLASS_PIG,
    NUMCLASSES
};

enum playerstate_t
{
    PST_LIVE,                   // playing
    PST_DEAD,                   // dead on the ground
    PST_REBORN                  // ready to restart
};

// psprites are scaled shapes directly on the view screen
// coordinates are given for a 320*200 view screen
enum psprnum_t
{
    ps_weapon,
    ps_flash,
    NUMPSPRITES
};

struct pspdef_t
{
    state_t *state;             // a NULL state means not active
    int tics;
    fixed_t sx, sy;
};

enum armortype_t
{
    ARMOR_ARMOR,
    ARMOR_SHIELD,
    ARMOR_HELMET,
    ARMOR_AMULET,
    NUMARMOR
};

struct inventory_t
{
    int type;
    int count;
};


struct player_t {
    mobj_t       *mo;
    playerstate_t playerstate;
    ticcmd_t      cmd;

    pclass_t pclass; // player class type

    fixed_t viewz;           // focal origin above r.z
    fixed_t viewheight;      // base height above floor for viewz
    fixed_t deltaviewheight; // squat speed
    fixed_t bob;             // bounded/scaled total momentum

    int     flyheight;
    int     lookdir;
    boolean centering;
    int     health; // only used between levels, mo->health
    // is used during levels
    int armorpoints[NUMARMOR];

    inventory_t  inventory[NUMINVENTORYSLOTS];
    artitype_t   readyArtifact;
    int          artifactCount;
    int          inventorySlotNum;
    int          powers[NUMPOWERS];
    int          keys;
    int          pieces;            // Fourth Weapon pieces
    signed int   frags[MAXPLAYERS]; // kills of other players
    weapontype_t readyweapon;
    weapontype_t pendingweapon; // wp_nochange if not changing
    boolean      weaponowned[NUMWEAPONS];
    int          mana[NUMMANA];
    int          attackdown, usedown; // true if button down last tic
    int          cheats;              // bit flags

    int refire; // refired shots are less accurate

    int          killcount, itemcount, secretcount; // for intermission
    char         message[80];                       // hint messages
    int          messageTics;                       // counter for showing messages
    short        ultimateMessage;
    short        yellowMessage;
    int          damagecount, bonuscount; // for screen flashing
    int          poisoncount;             // screen flash for poison damage
    mobj_t      *poisoner;                // NULL for non-player mobjs
    mobj_t      *attacker;                // who did damage (NULL for floors)
    int          extralight;              // so gun flashes light up areas
    int          fixedcolormap;           // can be set to REDCOLORMAP, etc
    int          colormap;                // 0-3 for which color to draw player
    pspdef_t     psprites[NUMPSPRITES];   // view sprites (gun, etc)
    int          morphTics;               // player is a pig if > 0
    unsigned int jumpTics;                // delay the next jump for a moment
    unsigned int worldTimer;              // total time the player's been playing
};

#endif //__PLAYER_HPP__