#ifndef P_PSPR_HPP
#define P_PSPR_HPP

// Include any necessary libraries or headers

// Declare any global variables or constants

// Declare any function prototypes
struct mobj_t;
struct mapthing_t;
struct player_t;
struct pspdef_t;

enum statenum_t;
enum weapontype_t;

#ifndef bool
using bool = bool;
#endif

void P_OpenWeapons(void);
void P_AddMaceSpot(mapthing_t * mthing);
void P_RepositionMace(mobj_t * mo);
void P_CloseWeapons(void);
void P_SetPsprite(player_t * player, int position, statenum_t stnum);
void P_ActivateBeak(player_t * player);
void P_PostChickenWeapon(player_t * player, weapontype_t weapon);
void P_BringUpWeapon(player_t * player);
bool P_CheckAmmo(player_t * player);
void P_FireWeapon(player_t * player);
void P_DropWeapon(player_t * player);
void A_WeaponReady(player_t * player, pspdef_t * psp);
void P_UpdateBeak(player_t * player, pspdef_t * psp);
void A_BeakReady(player_t * player, pspdef_t * psp);
void A_ReFire(player_t * player, pspdef_t * psp);
void A_Lower(player_t * player, pspdef_t * psp);
void A_BeakRaise(player_t * player, pspdef_t * psp);
void A_Raise(player_t * player, pspdef_t * psp);
void P_BulletSlope(mobj_t * mo);
void A_BeakAttackPL1(player_t * player, pspdef_t * psp);
void A_BeakAttackPL2(player_t * player, pspdef_t * psp);
void A_StaffAttackPL1(player_t * player, pspdef_t * psp);
void A_StaffAttackPL2(player_t * player, pspdef_t * psp);
void A_FireBlasterPL1(player_t * player, pspdef_t * psp);
void A_FireBlasterPL2(player_t * player, pspdef_t * psp);
void A_FireGoldWandPL1(player_t * player, pspdef_t * psp);
void A_FireGoldWandPL2(player_t * player, pspdef_t * psp);
void A_FireMacePL1B(player_t * player, pspdef_t * psp);
void A_FireMacePL1(player_t * player, pspdef_t * psp);
void A_MacePL1Check(mobj_t * ball);
void A_MaceBallImpact(mobj_t * ball);
void A_MaceBallImpact2(mobj_t * ball);
void A_FireMacePL2(player_t * player, pspdef_t * psp);
void A_DeathBallImpact(mobj_t * ball);
void A_SpawnRippers(mobj_t * actor);
void A_FireCrossbowPL1(player_t * player, pspdef_t * psp);
void A_FireCrossbowPL2(player_t * player, pspdef_t * psp);
void A_BoltSpark(mobj_t * bolt);
void A_FireSkullRodPL1(player_t * player, pspdef_t * psp);
void A_FireSkullRodPL2(player_t * player, pspdef_t * psp);
void A_SkullRodPL2Seek(mobj_t * actor);
void A_AddPlayerRain(mobj_t * actor);
void A_SkullRodStorm(mobj_t * actor);
void A_RainImpact(mobj_t * actor);
void A_HideInCeiling(mobj_t * actor);
void A_FirePhoenixPL1(player_t * player, pspdef_t * psp);
void A_PhoenixPuff(mobj_t * actor);
void A_RemovedPhoenixFunc(mobj_t *actor);
void A_InitPhoenixPL2(player_t * player, pspdef_t * psp);
void A_FirePhoenixPL2(player_t * player, pspdef_t * psp);
void A_ShutdownPhoenixPL2(player_t * player, pspdef_t * psp);
void A_FlameEnd(mobj_t * actor);
void A_FloatPuff(mobj_t * puff);
void A_GauntletAttack(player_t * player, pspdef_t * psp);
void A_Light0(player_t * player, pspdef_t * psp);
void A_Light1(player_t * player, pspdef_t * psp);
void A_Light2(player_t * player, pspdef_t * psp);
void P_SetupPsprites(player_t * player);
void P_MovePsprites(player_t * player);


#endif // P_PSPR_HPP
