#include "h2def.hpp"

// haleyjd: was WATCOMC, again preserved for historical interest as in Heretic
#if 0
extern bool useexterndriver;

#define EBT_FIRE			1
#define EBT_OPENDOOR 		2
#define EBT_SPEED			4
#define EBT_STRAFE			8
#define EBT_MAP				0x10
#define EBT_INVENTORYLEFT 	0x20
#define EBT_INVENTORYRIGHT 	0x40
#define EBT_USEARTIFACT		0x80
#define EBT_FLYDROP			0x100
#define EBT_CENTERVIEW		0x200
#define EBT_PAUSE			0x400
#define EBT_WEAPONCYCLE		0x800
#define EBT_JUMP			0x1000

typedef struct
{
    short vector;               // Interrupt vector

    signed char moveForward;    // forward/backward (maxes at 50)
    signed char moveSideways;   // strafe (maxes at 24)
    short angleTurn;            // turning speed (640 [slow] 1280 [fast])
    short angleHead;            // head angle (+2080 [left] : 0 [center] : -2048 [right])
    signed char pitch;          // look up/down (-110 : +90)
    signed char flyDirection;   // flyheight (+1/-1)
    unsigned short buttons;     // EBT_* flags
} externdata_t;
#endif

