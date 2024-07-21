#ifndef __R_MAIN_HPP__
#define __R_MAIN_HPP__

#include "player.hpp"

void R_Init(void);
void R_RenderPlayerView(player_t * player);
void R_SetViewSize(int blocks, int detail);

extern boolean setsizeneeded;

#endif // __R_MAIN_HPP__