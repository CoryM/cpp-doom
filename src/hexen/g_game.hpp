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
#ifndef __G_GAME_HPP__
#define __G_GAME_HPP__

#include "../d_event.hpp"
#include "../d_mode.hpp"

#include "h2def.hpp"    // MAXPLAYERS
#include "player.hpp"   // pclass_t

void G_Ticker(void);
void G_DoLoadGame(void);
void G_InitNew(skill_t skill, int episode, int map);
void G_RecordDemo(skill_t skill, int numplayers, int episode, int map, char *name);
void G_DeferedPlayDemo(const char *demo);
void G_TimeDemo(char *name);
void G_LoadGame(int slot);
void G_StartNewInit(void);
void G_SaveGame(int slot, char *description);
void G_DeferredNewGame(skill_t skill);
void G_ScreenShot(void);
void G_Ticker(void);
void G_WorldDone(void);
void G_Completed(int map, int position);
void G_TeleportNewMap(int map, int position);
void G_DeferedInitNew(skill_t skill, int episode, int map);

boolean G_Responder(event_t * ev);

extern boolean playeringame[MAXPLAYERS];
extern pclass_t PlayerClass[MAXPLAYERS];
extern player_t players[MAXPLAYERS];

#endif // __G_GAME_HPP__