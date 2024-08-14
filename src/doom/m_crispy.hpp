//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2018 Fabian Greffrath
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
// DESCRIPTION:
//	[crispy] Crispness menu
//

#ifndef __M_CRISPY__
#define __M_CRISPY__

struct multiitem_t
{
    int   value;
    const char *name;
};

extern multiitem_t multiitem_bobfactor[get_max<eBobFactor>()];
extern multiitem_t multiitem_brightmaps[get_max<eBrightmaps>()];
extern multiitem_t multiitem_centerweapon[get_max<eCenterWeapon>()];
extern multiitem_t multiitem_coloredhud[get_max<eColoredHud>()];
extern multiitem_t multiitem_crosshair[get_max<eCrosshair>()];
extern multiitem_t multiitem_crosshairtype[];
extern multiitem_t multiitem_freeaim[get_max<eFreeaim>()];
extern multiitem_t multiitem_demotimer[get_max<eDemoTimer>()];
extern multiitem_t multiitem_demotimerdir[];
extern multiitem_t multiitem_freelook[get_max<eFreelook>()];
extern multiitem_t multiitem_jump[get_max<eJump>()];
extern multiitem_t multiitem_sndchannels[4];
extern multiitem_t multiitem_secretmessage[get_max<eSecretMessage>()];
extern multiitem_t multiitem_translucency[get_max<eTranslucency>()];
extern multiitem_t multiitem_widescreen[get_max<eWidescreen>()];
extern multiitem_t multiitem_widgets[get_max<eWidgets>()];

extern void M_CrispyToggleAutomapstats(int choice);
extern void M_CrispyToggleBobfactor(int choice);
extern void M_CrispyToggleBrightmaps(int choice);
extern void M_CrispyToggleCenterweapon(int choice);
extern void M_CrispyToggleColoredblood(int choice);
extern void M_CrispyToggleColoredhud(int choice);
extern void M_CrispyToggleCrosshair(int choice);
extern void M_CrispyToggleCrosshairHealth(int choice);
extern void M_CrispyToggleCrosshairTarget(int choice);
extern void M_CrispyToggleCrosshairtype(int choice);
extern void M_CrispyToggleDemoBar(int choice);
extern void M_CrispyToggleDemoTimer(int choice);
extern void M_CrispyToggleDemoTimerDir(int choice);
extern void M_CrispyToggleExtAutomap(int choice);
extern void M_CrispyToggleExtsaveg(int choice);
extern void M_CrispyToggleFlipcorpses(int choice);
extern void M_CrispyToggleFreeaim(int choice);
extern void M_CrispyToggleFreelook(int choice);
extern void M_CrispyToggleFullsounds(int choice);
extern void M_CrispyToggleHires(int choice);
extern void M_CrispyToggleJumping(int choice);
extern void M_CrispyToggleLeveltime(int choice);
extern void M_CrispyToggleMouseLook(int choice);
extern void M_CrispyToggleNeghealth(int choice);
extern void M_CrispyToggleOverunder(int choice);
extern void M_CrispyTogglePitch(int choice);
extern void M_CrispyTogglePlayerCoords(int choice);
extern void M_CrispyToggleRecoil(int choice);
extern void M_CrispyToggleSecretmessage(int choice);
extern void M_CrispyToggleSmoothLighting(int choice);
extern void M_CrispyToggleSmoothScaling(int choice);
extern void M_CrispyToggleSndChannels(int choice);
extern void M_CrispyToggleSoundfixes(int choice);
extern void M_CrispyToggleSoundMono(int choice);
extern void M_CrispyToggleTranslucency(int choice);
extern void M_CrispyToggleUncapped(int choice);
extern void M_CrispyToggleVsync(int choice);
extern void M_CrispyToggleWeaponSquat(int choice);
extern void M_CrispyToggleWidescreen(int choice);

#endif
