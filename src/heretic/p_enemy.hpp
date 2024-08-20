#ifndef P_ENEMY_HPP
#define P_ENEMY_HPP

// Include any necessary headers here
struct mobj_t;
struct sector_t;

#include "../tables.hpp"

import m_fixed;

// Declare any functions or classes here
void P_InitMonsters(void);
void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle);
void P_RecursiveSound(sector_t * sec, int soundblocks);
void P_NoiseAlert(mobj_t * target, mobj_t * emmiter);
bool P_CheckMeleeRange(mobj_t * actor);
bool P_CheckMissileRange(mobj_t * actor);
bool P_Move(mobj_t * actor);
bool P_TryWalk(mobj_t * actor);
void P_NewChaseDir(mobj_t * actor);
bool P_LookForMonsters(mobj_t * actor);
bool P_LookForPlayers(mobj_t * actor, bool allaround);
void A_Look(mobj_t * actor);
void A_Chase(mobj_t * actor);
void A_FaceTarget(mobj_t * actor);
void A_Pain(mobj_t * actor);
void A_DripBlood(mobj_t * actor);
void A_KnightAttack(mobj_t * actor);
void A_ImpExplode(mobj_t * actor);
void A_BeastPuff(mobj_t * actor);
void A_ImpMeAttack(mobj_t * actor);
void A_ImpMsAttack(mobj_t * actor);
void A_ImpMsAttack2(mobj_t * actor);
void A_ImpDeath(mobj_t * actor);
void A_ImpXDeath1(mobj_t * actor);
void A_ImpXDeath2(mobj_t * actor);
bool P_UpdateChicken(mobj_t * actor, int tics);
void A_ChicAttack(mobj_t * actor);
void A_ChicLook(mobj_t * actor);
void A_ChicChase(mobj_t * actor);
void A_ChicPain(mobj_t * actor);
void A_Feathers(mobj_t * actor);
void A_MummyAttack(mobj_t * actor);
void A_MummyAttack2(mobj_t * actor);
void A_MummyFX1Seek(mobj_t * actor);
void A_MummySoul(mobj_t * mummy);
void A_Sor1Pain(mobj_t * actor);
void A_Sor1Chase(mobj_t * actor);
void A_Srcr1Attack(mobj_t * actor);
void A_SorcererRise(mobj_t * actor);
void P_DSparilTeleport(mobj_t * actor);
void A_Srcr2Decide(mobj_t * actor);
void A_Srcr2Attack(mobj_t * actor);
void A_BlueSpark(mobj_t * actor);
void A_GenWizard(mobj_t * actor);
void A_Sor2DthInit(mobj_t * actor);
void A_Sor2DthLoop(mobj_t * actor);
void A_SorZap(mobj_t * actor);
void A_SorRise(mobj_t * actor);
void A_SorDSph(mobj_t * actor);
void A_SorDExp(mobj_t * actor);
void A_SorDBon(mobj_t * actor);
void A_SorSightSnd(mobj_t * actor);
void A_MinotaurAtk1(mobj_t * actor);
void A_MinotaurDecide(mobj_t * actor);
void A_MinotaurCharge(mobj_t * actor);
void A_MinotaurAtk2(mobj_t * actor);
void A_MinotaurAtk3(mobj_t * actor);
void A_MntrFloorFire(mobj_t * actor);
void A_BeastAttack(mobj_t * actor);
void A_HeadAttack(mobj_t * actor);
void A_WhirlwindSeek(mobj_t * actor);
void A_HeadIceImpact(mobj_t * ice);
void A_HeadFireGrow(mobj_t * fire);
void A_SnakeAttack(mobj_t * actor);
void A_SnakeAttack2(mobj_t * actor);
void A_ClinkAttack(mobj_t * actor);
void A_GhostOff(mobj_t * actor);
void A_WizAtk1(mobj_t * actor);
void A_WizAtk2(mobj_t * actor);
void A_WizAtk3(mobj_t * actor);
void A_Scream(mobj_t * actor);
void P_DropItem(mobj_t * source, mobjtype_t type, int special, int chance);
void A_NoBlocking(mobj_t * actor);
void A_Explode(mobj_t * actor);
void A_PodPain(mobj_t * actor);
void A_RemovePod(mobj_t * actor);
void A_MakePod(mobj_t * actor);
void P_Massacre(void);
void A_BossDeath(mobj_t * actor);
void A_ESound(mobj_t * mo);
void A_SpawnTeleGlitter(mobj_t * actor);
void A_SpawnTeleGlitter2(mobj_t * actor);
void A_AccTeleGlitter(mobj_t * actor);
void A_InitKeyGizmo(mobj_t * gizmo);
void A_VolcanoSet(mobj_t * volcano);
void A_VolcanoBlast(mobj_t * volcano);
void A_VolcBallImpact(mobj_t * ball);
void A_SkullPop(mobj_t * actor);
void A_CheckSkullFloor(mobj_t * actor);
void A_CheckSkullDone(mobj_t * actor);
void A_CheckBurnGone(mobj_t * actor);
void A_FreeTargMobj(mobj_t * mo);
void A_AddPlayerCorpse(mobj_t * actor);
void A_FlameSnd(mobj_t * actor);
void A_HideThing(mobj_t * actor);
void A_UnHideThing(mobj_t * actor);


#endif // P_ENEMY_HPP
