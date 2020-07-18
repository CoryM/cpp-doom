//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//     Main loop code.
//
#include "crispy.hpp"
#include "d_event.hpp"
#include "d_loop.hpp"
#include "d_ticcmd.hpp"
#include "i_system.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"
#include "m_argv.hpp"
#include "m_fixed.hpp"
#include "net_client.hpp"
#include "net_gui.hpp"
#include "net_io.hpp"
#include "net_loop.hpp"
#include "net_query.hpp"
#include "net_sdl.hpp"
#include "net_server.hpp"

#include "fmt/core.h"

#include <array>
#include <cstdlib>

// The complete set of data for a particular tic.

struct ticcmd_set_t {
    std::array<ticcmd_t, NET_MAXPLAYERS> cmds {};
    std::array<bool, NET_MAXPLAYERS>     ingame { false };
};

// Maximum time that we wait in TryRunTics() for netgame data to be
// received before we bail out and render a frame anyway.
// Vanilla Doom used 20 for this value, but we use a smaller value
// instead for better responsiveness of the menu when we're stuck.
constexpr int MAX_NETGAME_STALL_TICS = 5;

//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// recvtic is the latest tic received from the server.
//
// a gametic cannot be run until ticcmds are received for it
// from all players.
//

static std::array<ticcmd_set_t, BACKUPTICS> ticdata {};

// The index of the next tic to be made (with a call to BuildTiccmd).

static int maketic;

// The number of complete tics received from the server so far.

static int recvtic;

// The number of tics that have been run (using RunTic) so far.

int gametic;
int oldleveltime; // [crispy] check if leveltime keeps tickin'

// When set to true, a single tic is run each time TryRunTics() is called.
// This is used for -timedemo mode.

bool singletics = false;

// Index of the local player.

static int localplayer;

// Used for original sync code.

static int skiptics = 0;

// Reduce the bandwidth needed by sampling game input less and transmitting
// less.  If ticdup is 2, sample half normal, 3 = one third normal, etc.

int ticdup;

// Amount to offset the timer for game sync.

fixed_t offsetms;

// Use new client synchronisation code

static bool new_sync = true;

// Callback functions for loop code.

static loop_interface_t *loop_interface = nullptr;

// Current players in the multiplayer game.
// This is distinct from playeringame[] used by the game code, which may
// modify playeringame[] when playing back multiplayer demos.

static std::array<bool, NET_MAXPLAYERS> local_playeringame = {};

// Requested player class "sent" to the server on connect.
// If we are only doing a single player game then this needs to be remembered
// and saved in the game settings.

static int player_class;


// 35 fps clock adjusted by offsetms milliseconds

static auto GetAdjustedTime() -> uint32_t
{
    auto time_ms = I_GetTimeMS();

    if (new_sync)
    {
        // Use the adjustments from net_client.c only if we are
        // using the new sync mode.
        time_ms += (offsetms / FRACUNIT);
    }

    return (time_ms * TICRATE) / TICBase;
}

static auto BuildNewTic() -> bool
{
    int gameticdiv = gametic / ticdup;

    I_StartTic();
    loop_interface->ProcessEvents();

    // Always run the menu

    loop_interface->RunMenu();

    if (drone)
    {
        // In drone mode, do not generate any ticcmds.

        return false;
    }

    if (new_sync)
    {
        // If playing single player, do not allow tics to buffer
        // up very far

        if (!net_client_connected && maketic - gameticdiv > 2)
        {
            return false;
        }

        // Never go more than ~200ms ahead
        constexpr int GameTicsTo200ms = (0.200F * TICRATE) + 1;
        if (maketic - gameticdiv > GameTicsTo200ms)
        {
            return false;
        }
    }
    else
    {
        constexpr int GameTicsTo115ms = static_cast<int>(0.143F * TICRATE);
        if (maketic - gameticdiv >= GameTicsTo115ms)
        {
            return false;
        }
    }

    //printf ("mk:%i ",maketic);
    ticcmd_t cmd = {};
    loop_interface->BuildTiccmd(&cmd, maketic);

    if (net_client_connected)
    {
        NET_CL_SendTiccmd(&cmd, maketic);
    }

    ticdata.at(maketic % BACKUPTICS).cmds.at(localplayer)   = cmd;
    ticdata.at(maketic % BACKUPTICS).ingame.at(localplayer) = true;

    ++maketic;

    return true;
}

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
int lasttime;

void NetUpdate()
{
    // If we are running with singletics (timing a demo), this
    // is all done separately.

    if (singletics)
    {
        return;
    }

    // Run network subsystems

    NET_CL_Run();
    NET_SV_Run();

    // check time
    auto nowtime = GetAdjustedTime() / ticdup;
    auto newtics = nowtime - lasttime;

    lasttime = nowtime;

    if (skiptics <= static_cast<int>(newtics))
    {
        newtics -= skiptics;
        skiptics = 0;
    }
    else
    {
        skiptics -= newtics;
        newtics = 0;
    }

    // build new ticcmds for console player

    for (unsigned int i = 0; i < newtics; i++)
    {
        if (!BuildNewTic())
        {
            break;
        }
    }
}

static void D_Disconnected()
{
    // In drone mode, the game cannot continue once disconnected.

    if (drone)
    {
        S_Error("Disconnected from server in drone mode.");
    }

    // disconnected from server

    fmt::print("Disconnected from server.\n");
}

//
// Invoked by the network engine when a complete set of ticcmds is
// available.
//

void D_ReceiveTic(const ticcmd_t *ticcmds, const bool *players_mask)
{
    // Disconnected from server?

    if (ticcmds == nullptr && players_mask == nullptr)
    {
        D_Disconnected();
        return;
    }

    auto &currentTicData = ticdata.at(recvtic % BACKUPTICS);
    for (int player = 0; player < NET_MAXPLAYERS; ++player)
    {
        if (drone || player != localplayer)
        {
            currentTicData.cmds.at(player)   = ticcmds[player];
            currentTicData.ingame.at(player) = players_mask[player];
        }
    }

    ++recvtic;
}

//
// Start game loop
//
// Called after the screen is set but before the game starts running.
//

void D_StartGameLoop()
{
    lasttime = static_cast<int>(GetAdjustedTime() / ticdup);
}

//
// Block until the game start message is received from the server.
//

static void BlockUntilStart(net_gamesettings_t *settings,
    netgame_startup_callback_t                  callback)
{
    while (!NET_CL_GetSettings(settings))
    {
        NET_CL_Run();
        NET_SV_Run();

        if (!net_client_connected)
        {
            S_Error("Lost connection to server");
        }

        if (callback != nullptr && !callback(net_client_wait_data.ready_players, net_client_wait_data.num_players))
        {
            S_Error("Netgame startup aborted.");
        }
        constexpr uint32_t SleepTimeInMS = 100;
        I_Sleep(SleepTimeInMS);
    }
}

void D_StartNetGame(net_gamesettings_t *settings,
    netgame_startup_callback_t          callback)
{
    offsetms = 0;
    recvtic  = 0;

    settings->consoleplayer     = 0;
    settings->num_players       = 1;
    settings->player_classes[0] = player_class;

    //!
    // @category net
    //
    // Use original network client sync code rather than the improved
    // sync code.
    //
    settings->new_sync = static_cast<int>(!M_ParmExists("-oldsync"));

    //!
    // @category net
    // @arg <n>
    //
    // Send n extra tics in every packet as insurance against dropped
    // packets.
    //

    auto i = M_CheckParmWithArgs("-extratics", 1);

    if (i > 0)
    {
        settings->extratics = atoi(M_GetArgument(i + 1));
    }
    else
    {
        settings->extratics = 1;
    }

    //!
    // @category net
    // @arg <n>
    //
    // Reduce the resolution of the game by a factor of n, reducing
    // the amount of network bandwidth needed.
    //

    i = M_CheckParmWithArgs("-dup", 1);

    if (i > 0)
    {
        settings->ticdup = atoi(M_GetArgument(i + 1));
    }
    else
    {
        settings->ticdup = 1;
    }

    if (net_client_connected)
    {
        // Send our game settings and block until game start is received
        // from the server.

        NET_CL_StartGame(settings);
        BlockUntilStart(settings, callback);

        // Read the game settings that were received.

        NET_CL_GetSettings(settings);
    }

    if (drone)
    {
        settings->consoleplayer = 0;
    }

    // Set the local player and playeringame[] values.

    localplayer = settings->consoleplayer;

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        local_playeringame[i] = i < settings->num_players;
    }

    // Copy settings to global variables.

    ticdup   = settings->ticdup;
    new_sync = settings->new_sync;

    // TODO: Message disabled until we fix new_sync.
    //if (!new_sync)
    //{
    //    printf("Syncing netgames like Vanilla Doom.\n");
    //}
}

auto D_InitNetGame(net_connect_data_t *connect_data) -> bool
{
    bool        result = false;
    net_addr_t *addr   = nullptr;

    // Call D_QuitNetGame on exit:

    I_AtExit(D_QuitNetGame, true);

    player_class = connect_data->player_class;

    //!
    // @category net
    //
    // Start a multiplayer server, listening for connections.
    //

    if (M_CheckParm("-server") > 0
        || M_CheckParm("-privateserver") > 0)
    {
        NET_SV_Init();
        NET_SV_AddModule(&net_loop_server_module);
        NET_SV_AddModule(&net_sdl_module);
        NET_SV_RegisterWithMaster();

        net_loop_client_module.InitClient();
        addr = net_loop_client_module.ResolveAddress(NULL);
        NET_ReferenceAddress(addr);
    }
    else
    {
        //!
        // @category net
        //
        // Automatically search the local LAN for a multiplayer
        // server and join it.
        //

        int i = M_CheckParm("-autojoin");

        if (i > 0)
        {
            addr = NET_FindLANServer();

            if (addr == nullptr)
            {
                S_Error("No server found on local LAN");
            }
        }

        //!
        // @arg <address>
        // @category net
        //
        // Connect to a multiplayer server running on the given
        // address.
        //

        i = M_CheckParmWithArgs("-connect", 1);

        if (i > 0)
        {
            net_sdl_module.InitClient();
            addr = net_sdl_module.ResolveAddress(M_GetArgument(i + 1));
            NET_ReferenceAddress(addr);

            if (addr == nullptr)
            {
                //I_Error("Unable to resolve '%s'\n", myargv[i + 1]);
                S_Error(fmt::format("Unable to resolve {}\n", M_GetArgument(i + 1)));
            }
        }
    }

    if (addr != nullptr)
    {
        if (M_CheckParm("-drone") > 0)
        {
            connect_data->drone = true;
        }

        if (!NET_CL_Connect(addr, connect_data))
        {
            S_Error(fmt::format("D_InitNetGame: Failed to connect to {}:\n{}\n",
                NET_AddrToString(addr), net_client_reject_reason));
        }

        fmt::print("D_InitNetGame: Connected to {}\n", NET_AddrToString(addr));
        NET_ReleaseAddress(addr);

        // Wait for launch message received from server.

        NET_WaitForLaunch();

        result = true;
    }

    return result;
}


//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame(void)
{
    NET_SV_Shutdown();
    NET_CL_Disconnect();
}

static int GetLowTic(void)
{
    int lowtic;

    lowtic = maketic;

    if (net_client_connected)
    {
        if (drone || recvtic < lowtic)
        {
            lowtic = recvtic;
        }
    }

    return lowtic;
}

static int frameon;
static int frameskip[4];
static int oldnettics;

static void OldNetSync(void)
{
    unsigned int i;
    int          keyplayer = -1;

    frameon++;

    // ideally maketic should be 1 - 3 tics above lowtic
    // if we are consistently slower, speed up time

    for (i = 0; i < NET_MAXPLAYERS; i++)
    {
        if (local_playeringame[i])
        {
            keyplayer = i;
            break;
        }
    }

    if (keyplayer < 0)
    {
        // If there are no players, we can never advance anyway

        return;
    }

    if (localplayer == keyplayer)
    {
        // the key player does not adapt
    }
    else
    {
        if (maketic <= recvtic)
        {
            lasttime--;
            // printf ("-");
        }

        frameskip[frameon & 3] = oldnettics > recvtic;
        oldnettics             = maketic;

        if (frameskip[0] && frameskip[1] && frameskip[2] && frameskip[3])
        {
            skiptics = 1;
            // printf ("+");
        }
    }
}

// Returns true if there are players in the game:

static bool PlayersInGame(void)
{
    bool         result = false;
    unsigned int i;

    // If we are connected to a server, check if there are any players
    // in the game.

    if (net_client_connected)
    {
        for (i = 0; i < NET_MAXPLAYERS; ++i)
        {
            result = result || local_playeringame[i];
        }
    }

    // Whether single or multi-player, unless we are running as a drone,
    // we are in the game.

    if (!drone)
    {
        result = true;
    }

    return result;
}

// When using ticdup, certain values must be cleared out when running
// the duplicate ticcmds.

static void TicdupSquash(ticcmd_set_t *set)
{
    ticcmd_t *   cmd;
    unsigned int i;

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        cmd           = &set->cmds[i];
        cmd->chatchar = 0;
        if (cmd->buttons & BT_SPECIAL)
            cmd->buttons = 0;
    }
}

// When running in single player mode, clear all the ingame[] array
// except the local player.

static void SinglePlayerClear(ticcmd_set_t *set)
{
    for (int i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (i != localplayer)
        {
            set->ingame[i] = false;
        }
    }
}


//
// TryRunTics
//

void TryRunTics(void)
{
    int        i;
    int        lowtic;
    int        entertic;
    static int oldentertics;
    int        realtics;
    int        availabletics;
    int        counts;

    // [AM] If we've uncapped the framerate and there are no tics
    //      to run, return early instead of waiting around.
    extern int leveltime;
#define return_early (crispy->uncapped && counts == 0 && leveltime > oldleveltime && screenvisible)

    // get real tics
    entertic     = I_GetTime() / ticdup;
    realtics     = entertic - oldentertics;
    oldentertics = entertic;

    // in singletics mode, run a single tic every time this function
    // is called.

    if (singletics)
    {
        BuildNewTic();
    }
    else
    {
        NetUpdate();
    }

    lowtic = GetLowTic();

    availabletics = lowtic - gametic / ticdup;

    // decide how many tics to run

    if (new_sync)
    {
        counts = availabletics;

        // [AM] If we've uncapped the framerate and there are no tics
        //      to run, return early instead of waiting around.
        if (return_early)
            return;
    }
    else
    {
        // decide how many tics to run
        if (realtics < availabletics - 1)
            counts = realtics + 1;
        else if (realtics < availabletics)
            counts = realtics;
        else
            counts = availabletics;

        // [AM] If we've uncapped the framerate and there are no tics
        //      to run, return early instead of waiting around.
        if (return_early)
            return;

        if (counts < 1)
            counts = 1;

        if (net_client_connected)
        {
            OldNetSync();
        }
    }

    if (counts < 1)
        counts = 1;

    // wait for new tics if needed
    while (!PlayersInGame() || lowtic < gametic / ticdup + counts)
    {
        NetUpdate();

        lowtic = GetLowTic();

        if (lowtic < gametic / ticdup)
        {
            S_Error("TryRunTics: lowtic < gametic");
        }

        // Still no tics to run? Sleep until some are available.
        if (lowtic < gametic / ticdup + counts)
        {
            // If we're in a netgame, we might spin forever waiting for
            // new network data to be received. So don't stay in here
            // forever - give the menu a chance to work.
            if (I_GetTime() / ticdup - entertic >= MAX_NETGAME_STALL_TICS)
            {
                return;
            }

            I_Sleep(1);
        }
    }

    // run the count * ticdup dics
    while (counts--)
    {
        if (!PlayersInGame())
        {
            return;
        }

        ticcmd_set_t *set = &ticdata.at((gametic / ticdup) % BACKUPTICS);

        if (!net_client_connected)
        {
            SinglePlayerClear(set);
        }

        for (i = 0; i < ticdup; i++)
        {
            if (gametic / ticdup > lowtic)
            {
                S_Error("gametic>lowtic");
            }

            std::copy(set->ingame.begin(), set->ingame.end(), &local_playeringame[0]);

            loop_interface->RunTic(&set->cmds[0], &set->ingame[0]);
            gametic++;

            // modify command for duplicated tics
            TicdupSquash(set);
        }

        NetUpdate(); // check for new console commands
    }
}

void D_RegisterLoopCallbacks(loop_interface_t *i)
{
    loop_interface = i;
}

// TODO: Move nonvanilla demo functions into a dedicated file.
#include "m_misc.hpp"
#include "w_wad.hpp"

static bool StrictDemos(void)
{
    //!
    // @category demo
    //
    // When recording or playing back demos, disable any extensions
    // of the vanilla demo format - record demos as vanilla would do,
    // and play back demos as vanilla would do.
    //
    return M_ParmExists("-strictdemos");
}

// If the provided conditional value is true, we're trying to record
// a demo file that will include a non-vanilla extension. The function
// will return true if the conditional is true and it's allowed to use
// this extension (no extensions are allowed if -strictdemos is given
// on the command line). A warning is shown on the console using the
// provided string describing the non-vanilla expansion.
bool D_NonVanillaRecord(bool conditional, const char *feature)
{
    if (!conditional || StrictDemos())
    {
        return false;
    }

    printf("Warning: Recording a demo file with a non-vanilla extension "
           "(%s). Use -strictdemos to disable this extension.\n",
        feature);

    return true;
}

// Returns true if the given lump number corresponds to data from a .lmp
// file, as opposed to a WAD.
static bool IsDemoFile(int lumpnum)
{
    const auto lower = S_ForceLowercase(lumpinfo[lumpnum]->wad_file->path);
    return M_StringEndsWith(lower.c_str(), ".lmp");
    ;
}

// If the provided conditional value is true, we're trying to play back
// a demo that includes a non-vanilla extension. We return true if the
// conditional is true and it's allowed to use this extension, checking
// that:
//  - The -strictdemos command line argument is not provided.
//  - The given lumpnum identifying the demo to play back identifies a
//    demo that comes from a .lmp file, not a .wad file.
//  - Before proceeding, a warning is shown to the user on the console.
bool D_NonVanillaPlayback(bool conditional, int lumpnum,
    const char *feature)
{
    if (!conditional || StrictDemos())
    {
        return false;
    }

    if (!IsDemoFile(lumpnum))
    {
        printf("Warning: WAD contains demo with a non-vanilla extension "
               "(%s)\n",
            feature);
        return false;
    }

    printf("Warning: Playing back a demo file with a non-vanilla extension "
           "(%s). Use -strictdemos to disable this extension.\n",
        feature);

    return true;
}
