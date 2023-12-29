#ifndef P_INTER_HPP
#define P_INTER_HPP

// Include any necessary standard library headers here

// Declare any necessary functions, classes, or variables here
struct player_t;
struct mobj_t;

enum ammotype_t;
enum artitype_t;
enum keytype_t;
enum powertype_t;
enum weapontype_t;

#ifndef boolean
using boolean = bool;
#endif

void P_SetMessage(player_t * player, const char *message, boolean ultmsg);
boolean P_GiveAmmo(player_t * player, ammotype_t ammo, int count);
boolean P_GiveWeapon(player_t * player, weapontype_t weapon);
boolean P_GiveBody(player_t * player, int num);
boolean P_GiveArmor(player_t * player, int armortype);
void P_GiveKey(player_t * player, keytype_t key);
boolean P_GivePower(player_t * player, powertype_t power);
boolean P_GiveArtifact(player_t * player, artitype_t arti, mobj_t * mo);
void P_SetDormantArtifact(mobj_t * arti);
void A_RestoreArtifact(mobj_t * arti);
void P_HideSpecialThing(mobj_t * thing);
void A_RestoreSpecialThing1(mobj_t * thing);
void A_RestoreSpecialThing2(mobj_t * thing);
void P_TouchSpecialThing(mobj_t * special, mobj_t * toucher);
void P_KillMobj(mobj_t * source, mobj_t * target);
void P_MinotaurSlam(mobj_t * source, mobj_t * target);
void P_TouchWhirlwind(mobj_t * target);
boolean P_ChickenMorphPlayer(player_t * player);
boolean P_ChickenMorph(mobj_t * actor);
boolean P_AutoUseChaosDevice(player_t * player);
void P_AutoUseHealth(player_t * player, int saveHealth);
void P_DamageMobj(mobj_t * target, mobj_t * inflictor, mobj_t * source, int damage);

#endif // P_INTER_HPP
