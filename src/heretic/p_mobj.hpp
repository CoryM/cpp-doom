#ifndef P_MOBJ_HPP
#define P_MOBJ_HPP

// Include any necessary headers here

// Define your class or functions here

struct mobj_t;
struct mapthing_t;

enum statenum_t;
enum mobjtype_t;

#ifndef bool
using bool = bool;
#endif 
#ifndef fixed_t
using fixed_t = int;
#endif
#ifndef angle_t
using angle_t = unsigned int;
#endif

bool P_SetMobjState(mobj_t * mobj, statenum_t state);
bool P_SetMobjStateNF(mobj_t * mobj, statenum_t state);
void P_ExplodeMissile(mobj_t * mo);
void P_FloorBounceMissile(mobj_t * mo);
void P_ThrustMobj(mobj_t * mo, angle_t angle, fixed_t move);
int P_FaceMobj(mobj_t * source, mobj_t * target, angle_t * delta);
bool P_SeekerMissile(mobj_t * actor, angle_t thresh, angle_t turnMax);
void P_XYMovement(mobj_t * mo);
void P_ZMovement(mobj_t * mo);
void P_NightmareRespawn(mobj_t * mobj);
void P_BlasterMobjThinker(mobj_t * mobj);
void P_MobjThinker(mobj_t * mobj);
mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void P_RemoveMobj(mobj_t * mobj);
void P_SpawnPlayer(mapthing_t * mthing);
void P_SpawnMapThing(mapthing_t * mthing);
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator);
void P_RipperBlood(mobj_t * mo);
int P_GetThingFloorType(mobj_t * thing);
int P_HitFloor(mobj_t * thing);
bool P_CheckMissileSpawn(mobj_t * missile);
mobj_t *P_SpawnMissile(mobj_t * source, mobj_t * dest, mobjtype_t type);
mobj_t *P_SpawnMissileAngle(mobj_t * source, mobjtype_t type, angle_t angle, fixed_t momz);
mobj_t *P_SpawnPlayerMissile(mobj_t * source, mobjtype_t type);
mobj_t *P_SPMAngle(mobj_t * source, mobjtype_t type, angle_t angle);
void A_ContMobjSound(mobj_t * actor);

#endif // P_MOBJ_HPP
