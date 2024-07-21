#ifndef __P_SETUP_HPP__
#define __P_SETUP_HPP__

#include "../d_mode.hpp"

void P_Init(void);
// called by startup code

char *P_GetMapName(int map);
void P_SetupLevel(int episode, int map, int playermask, skill_t skill);
int P_GetMapCluster(int map);
int P_TranslateMap(int map);
int P_GetMapCDTrack(int map);
int P_GetMapWarpTrans(int map);
int P_GetMapNextMap(int map);
int P_GetMapSky1Texture(int map);
int P_GetMapSky2Texture(int map);
char *P_GetMapName(int map);
fixed_t P_GetMapSky1ScrollDelta(int map);
fixed_t P_GetMapSky2ScrollDelta(int map);
boolean P_GetMapDoubleSky(int map);
boolean P_GetMapLightning(int map);
boolean P_GetMapFadeTable(int map);
char *P_GetMapSongLump(int map);
void P_PutMapSongLump(int map, char *lumpName);
int P_GetCDStartTrack(void);
int P_GetCDEnd1Track(void);
int P_GetCDEnd2Track(void);
int P_GetCDEnd3Track(void);
int P_GetCDIntermissionTrack(void);
int P_GetCDTitleTrack(void);

#endif // __P_SETUP_HPP__