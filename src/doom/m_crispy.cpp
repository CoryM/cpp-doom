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
#include <bitset>       // std::bitset

#include "doomstat.hpp"
#include "p_local.hpp" // [crispy] thinkercap
#include "s_sound.hpp"
#include "r_defs.hpp" // [crispy] laserpatch
#include "r_sky.hpp"  // [crispy] R_InitSkyMap()

#include "m_crispy.hpp"

multiitem_t multiitem_bobfactor[to_int(eBobFactor::NUM)] = {
    { to_int(eBobFactor::Full), "full" },
    { to_int(eBobFactor::Mid), "75%" },
    { to_int(eBobFactor::Off), "off" },
};

multiitem_t multiitem_brightmaps[to_int(eBrightmaps::NUM)] = {
    { to_int(eBrightmaps::Off), "none" },
    { to_int(eBrightmaps::Textures), "walls" },
    { to_int(eBrightmaps::Sprites), "items" },
    { to_int(eBrightmaps::Both), "both" },
};

multiitem_t multiitem_centerweapon[to_int(eCenterWeapon::NUM)] = {
    { to_int(eCenterWeapon::Off), "off" },
    { to_int(eCenterWeapon::Center), "centered" },
    { to_int(eCenterWeapon::Bob), "bobbing" },
};

multiitem_t multiitem_coloredhud[to_int(eColoredHud::NUM)] = {
    { to_int(eColoredHud::Off), "off" },
    { to_int(eColoredHud::Bar), "status bar" },
    { to_int(eColoredHud::Text), "hud texts" },
    { to_int(eColoredHud::Both), "both" },
};

multiitem_t multiitem_crosshair[get_max<eCrosshair>()] = {
    { to_int(eCrosshair::Off), "off" },
    { to_int(eCrosshair::Static), "static" },
    { to_int(eCrosshair::Projected), "projected" },
};

multiitem_t multiitem_crosshairtype[] = {
    { -1, "none" },
    { 0, "cross" },
    { 1, "chevron" },
    { 2, "dot" },
};

multiitem_t multiitem_freeaim[get_max<eFreeaim>()] = {
    { to_int(eFreeaim::Auto), "autoaim" },
    { to_int(eFreeaim::Direct), "direct" },
    { to_int(eFreeaim::Both), "both" },
};

multiitem_t multiitem_demotimer[get_max<eDemoTimer>()] = {
    { to_int(eDemoTimer::Off), "off" },
    { to_int(eDemoTimer::Record), "recording" },
    { to_int(eDemoTimer::Playback), "playback" },
    { to_int(eDemoTimer::Both), "both" },
};

multiitem_t multiitem_demotimerdir[] = {
    { 0, "none" },
    { 1, "forward" },
    { 2, "backward" },
};

multiitem_t multiitem_freelook[get_max<eFreelook>()] = {
    { to_int(eFreelook::Off), "off" },
    { to_int(eFreelook::Spring), "spring" },
    { to_int(eFreelook::Lock), "lock" },
};

multiitem_t multiitem_jump[get_max<eJump>()] = {
    { to_int(eJump::Off), "off" },
    { to_int(eJump::Low), "low" },
    { to_int(eJump::High), "high" },
};

multiitem_t multiitem_secretmessage[get_max<eSecretMessage>()] = {
    { to_int(eSecretMessage::Off), "off" },
    { to_int(eSecretMessage::On), "on" },
    { to_int(eSecretMessage::Count), "count" },
};

multiitem_t multiitem_translucency[get_max<eTranslucency>()] = {
    { to_int(eTranslucency::Off), "off" },
    { to_int(eTranslucency::Missile), "projectiles" },
    { to_int(eTranslucency::Item), "items" },
    { to_int(eTranslucency::Both), "both" },
};

multiitem_t multiitem_sndchannels[4] = {
    { 8, "8" },
    { 16, "16" },
    { 32, "32" },
};

multiitem_t multiitem_widescreen[get_max<eWidescreen>()] = {
    { to_int(eWidescreen::Off), "off" },
    { to_int(eWidescreen::Wide), "on, wide HUD" },
    { to_int(eWidescreen::Compact), "on, compact HUD" },
};

multiitem_t multiitem_widgets[get_max<eWidgets>()] = {
    { to_int(eWidgets::Off), "never" },
    { to_int(eWidgets::Automap), "in Automap" },
    { to_int(eWidgets::Always), "always" },
};

extern void AM_ReInit(void);
extern void EnableLoadingDisk(void);
extern void P_SegLengths(boolean contrast_only);
extern void R_ExecuteSetViewSize(void);
extern void R_InitLightTables(void);
extern void I_ReInitGraphics(bReinit reinit);
extern void ST_createWidgets(void);
extern void HU_Start(void);
extern void M_SizeDisplay(int choice);


void M_CrispyToggleAutomapstats([[maybe_unused]] int choice)
{
    crispy->automapstats = wrap_next(crispy->automapstats);
}

void M_CrispyToggleBobfactor([[maybe_unused]] int choice)
{
    crispy->bobfactor = wrap_next(crispy->bobfactor);
}

void M_CrispyToggleBrightmaps([[maybe_unused]] int choice)
{
    crispy->brightmaps = wrap_next(crispy->brightmaps);
}

void M_CrispyToggleCenterweapon([[maybe_unused]] int choice)
{
    crispy->centerweapon = wrap_next(crispy->centerweapon);
}

void M_CrispyToggleColoredblood([[maybe_unused]] int choice)
{
    thinker_t *th;

    if (gameversion == exe_chex)
    {
        return;
    }

    crispy->coloredblood = !crispy->coloredblood;

    // [crispy] switch NOBLOOD flag for Lost Souls
    for (th = thinkercap.next; th && th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mobj = (mobj_t *)th;

            if (mobj->type == MT_SKULL)
            {
                if (crispy->coloredblood)
                {
                    mobj->flags |= MF_NOBLOOD;
                }
                else
                {
                    mobj->flags &= ~MF_NOBLOOD;
                }
            }
        }
    }
}

void M_CrispyToggleColoredhud([[maybe_unused]] int choice)
{
    crispy->coloredhud = wrap_next(crispy->coloredhud);
}

void M_CrispyToggleCrosshair([[maybe_unused]] int choice)
{
    crispy->crosshair = wrap_next(crispy->crosshair);
}

void M_CrispyToggleCrosshairHealth([[maybe_unused]] int choice)
{
    crispy->crosshairhealth = !crispy->crosshairhealth;
}

void M_CrispyToggleCrosshairTarget([[maybe_unused]] int choice)
{
    crispy->crosshairtarget = !crispy->crosshairtarget;
}

void M_CrispyToggleCrosshairtype([[maybe_unused]] int choice)
{
    if (!to_int(crispy->crosshair))
    {
        return;
    }

    crispy->crosshairtype = crispy->crosshairtype + 1;

    if (!laserpatch[crispy->crosshairtype].c)
    {
        crispy->crosshairtype = 0;
    }
}

void M_CrispyToggleDemoBar([[maybe_unused]] int choice)
{
    crispy->demobar = !crispy->demobar;
}

void M_CrispyToggleDemoTimer([[maybe_unused]] int choice)
{
    crispy->demotimer = wrap_next(crispy->demotimer);
}

void M_CrispyToggleDemoTimerDir([[maybe_unused]] int choice)
{
    if (!bit_AND(crispy->demotimer, eDemoTimer::Playback))
    {
        return;
    }

    crispy->demotimerdir = !crispy->demotimerdir;
}

void M_CrispyToggleExtAutomap([[maybe_unused]] int choice)
{
    crispy->extautomap = !crispy->extautomap;
}

void M_CrispyToggleExtsaveg([[maybe_unused]] int choice)
{
    crispy->extsaveg = !crispy->extsaveg;
}

void M_CrispyToggleFlipcorpses([[maybe_unused]] int choice)
{
    if (gameversion == exe_chex)
    {
        return;
    }

    crispy->flipcorpses = !crispy->flipcorpses;
}

void M_CrispyToggleFreeaim([[maybe_unused]] int choice)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->freeaim = wrap_next(crispy->freeaim);

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

static void M_CrispyToggleSkyHook(void)
{
    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

void M_CrispyToggleFreelook([[maybe_unused]] int choice)
{
    crispy->freelook = wrap_next(crispy->freelook);

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleFullsounds([[maybe_unused]] int choice)
{
    int i;

    crispy->soundfull = !crispy->soundfull;

    // [crispy] weapon sound sources
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            players[i].so = Crispy_PlayerSO(i);
        }
    }
}

static void M_CrispyToggleHiresHook(void)
{
    crispy->hires = !crispy->hires;

    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(bReinit::FrameBuffers | bReinit::Textures | bReinit::AspectRatio);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] re-draw bezel
    R_FillBackScreen();
    // [crispy] re-calculate disk icon coordinates
    EnableLoadingDisk();
    // [crispy] re-calculate automap coordinates
    AM_ReInit();
}

void M_CrispyToggleHires([[maybe_unused]] int choice)
{
    crispy->post_rendering_hook = M_CrispyToggleHiresHook;
}

void M_CrispyToggleJumping([[maybe_unused]] int choice)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->jump = wrap_next(crispy->jump);

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleLeveltime([[maybe_unused]] int choice)
{
    crispy->leveltime = wrap_next(crispy->leveltime);
}

void M_CrispyToggleMouseLook([[maybe_unused]] int choice)
{
    crispy->mouselook = !crispy->mouselook;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleNeghealth([[maybe_unused]] int choice)
{
    crispy->neghealth = !crispy->neghealth;
}

void M_CrispyToggleOverunder([[maybe_unused]] int choice)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->overunder = !crispy->overunder;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyTogglePitch([[maybe_unused]] int choice)
{
    crispy->pitch = !crispy->pitch;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyTogglePlayerCoords([[maybe_unused]] int choice)
{
    crispy->playercoords = wrap_next(crispy->playercoords);
}

void M_CrispyToggleRecoil([[maybe_unused]] int choice)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->recoil = !crispy->recoil;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleSecretmessage([[maybe_unused]] int choice)
{
    crispy->secretmessage = wrap_next(crispy->secretmessage);
}

void M_CrispyToggleSmoothScaling([[maybe_unused]] int choice)
{
    crispy->smoothscaling = !crispy->smoothscaling;
}

static void M_CrispyToggleSmoothLightingHook(void)
{
    crispy->smoothlight = !crispy->smoothlight;

    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
    // [crispy] re-calculate fake contrast
    P_SegLengths(true);
}

void M_CrispyToggleSmoothLighting([[maybe_unused]] int choice)
{
    crispy->post_rendering_hook = M_CrispyToggleSmoothLightingHook;
}

void M_CrispyToggleSndChannels([[maybe_unused]] int choice)
{
    S_UpdateSndChannels();
}

void M_CrispyToggleSoundfixes([[maybe_unused]] int choice)
{
    crispy->soundfix = !crispy->soundfix;
}

void M_CrispyToggleSoundMono([[maybe_unused]] int choice)
{
    crispy->soundmono = !crispy->soundmono;

    S_UpdateStereoSeparation();
}

void M_CrispyToggleTranslucency([[maybe_unused]] int choice)
{
    crispy->translucency = wrap_next(crispy->translucency);
}

void M_CrispyToggleUncapped([[maybe_unused]] int choice)
{
    crispy->uncapped = !crispy->uncapped;
}

void M_CrispyToggleVsyncHook(void)
{
    crispy->vsync = !crispy->vsync;

    I_ReInitGraphics(bReinit::Renderer | bReinit::Textures | bReinit::AspectRatio);
}

void M_CrispyToggleVsync([[maybe_unused]] int choice)
{
    if (force_software_renderer)
    {
        return;
    }

    crispy->post_rendering_hook = M_CrispyToggleVsyncHook;
}

void M_CrispyToggleWeaponSquat([[maybe_unused]] int choice)
{
    crispy->weaponsquat = !crispy->weaponsquat;
}

void M_CrispyReinitHUDWidgets(void)
{
    if (gamestate == GS_LEVEL && gamemap > 0)
    {
        // [crispy] re-arrange status bar widgets
        ST_createWidgets();
        // [crispy] re-arrange heads-up widgets
        HU_Start();
    }
}

static void M_CrispyToggleWidescreenHook(void)
{
    crispy->widescreen = wrap_next(crispy->widescreen);

    // [crispy] no need to re-init when switching from wide to compact
    if (to_int(crispy->widescreen) == 1 || crispy->widescreen == eWidescreen::Off)
    {
        // [crispy] re-initialize screenSize_min
        M_SizeDisplay(-1);
        // [crispy] re-initialize framebuffers, textures and renderer
        I_ReInitGraphics(bReinit::FrameBuffers | bReinit::Textures | bReinit::AspectRatio);
        // [crispy] re-calculate framebuffer coordinates
        R_ExecuteSetViewSize();
        // [crispy] re-draw bezel
        R_FillBackScreen();
        // [crispy] re-calculate disk icon coordinates
        EnableLoadingDisk();
        // [crispy] re-calculate automap coordinates
        AM_ReInit();
    }

    M_CrispyReinitHUDWidgets();
}

void M_CrispyToggleWidescreen([[maybe_unused]] int choice)
{
    crispy->post_rendering_hook = M_CrispyToggleWidescreenHook;
}
