#ifndef __SV_SAVE_HPP__
#define __SV_SAVE_HPP__


extern char *SavePath;

void SV_SaveGame(int slot, const char *description);
void SV_SaveMap(bool savePlayers);
void SV_LoadGame(int slot);
void SV_MapTeleport(int map, int position);
void SV_LoadMap(void);
void SV_InitBaseSlot(void);
void SV_UpdateRebornSlot(void);
void SV_ClearRebornSlot(void);
bool SV_RebornSlotAvailable(void);
int SV_GetRebornSlot(void);


#endif //__SV_SAVE_HPP__