//
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
//     Querying servers to find their current status.
//

#include "i_system.hpp"
#include "i_timer.hpp"
#include "m_misc.hpp"
#include "net_common.hpp"
#include "net_defs.hpp"
#include "net_io.hpp"
#include "net_packet.hpp"
#include "net_query.hpp"
#include "net_sdl.hpp"
#include "net_structrw.hpp"

#include "fmt/core.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>
#include <string_view>
#include <vector>

// DNS address of the Internet master server.
constexpr std::string_view MASTER_SERVER_ADDRESS = "master.chocolate-doom.org:2342";

// General magic number for converting Seconds to Milliseconds
constexpr unsigned int Seconds_To_Milliseconds = 1000;

// Time to wait for a response before declaring a timeout.
constexpr unsigned int QUERY_TIMEOUT_MS = 2 * Seconds_To_Milliseconds;

// Time to wait for secure demo signatures before declaring a timeout.
constexpr unsigned int SIGNATURE_TIMEOUT_MS = 5 * Seconds_To_Milliseconds;

// Number of query attempts to make before giving up on a server.
constexpr unsigned int QUERY_MAX_ATTEMPTS = 3;

enum query_target_type_t
{
    QUERY_TARGET_SERVER,    // Normal server target.
    QUERY_TARGET_MASTER,    // The master server.
    QUERY_TARGET_BROADCAST, // Send a broadcast query
    QUERY_TARGET_UNKNOWN    // Undefined
};

enum query_target_state_t
{
    QUERY_TARGET_QUEUED,    // Query not yet sent
    QUERY_TARGET_QUERIED,   // Query sent, waiting response
    QUERY_TARGET_RESPONDED, // Response received
    QUERY_TARGET_NO_RESPONSE
};

struct query_target_t {
    query_target_type_t  type  = QUERY_TARGET_UNKNOWN;
    query_target_state_t state = QUERY_TARGET_NO_RESPONSE;
    net_addr_t *         addr  = nullptr;
    net_querydata_t      data;
    unsigned int         ping_time      = 0;
    unsigned int         query_time     = 0;
    unsigned int         query_attempts = 0;
    bool                 printed        = false;
};

static bool registered_with_master = false;
static bool got_master_response    = false;

static net_context_t *             query_context;
static std::vector<query_target_t> targets;

static bool query_loop_running = false;
static bool printed_header     = false;
static int  last_query_time    = 0;

static char *securedemo_start_message = nullptr;

// Resolve the master server address.

auto NET_Query_ResolveMaster(net_context_t *context) -> net_addr_t *
{
    auto *addr = NET_ResolveAddress(context, MASTER_SERVER_ADDRESS.data());

    if (addr == nullptr)
    {
        // fprintf(stderr, "Warning: Failed to resolve address "
        //                 "for master server: %s\n",
        //     MASTER_SERVER_ADDRESS);
        fmt::print(stderr, "Warning: Failed to resolve address for master server: {}\n", MASTER_SERVER_ADDRESS);
    }

    return addr;
}

// Send a registration packet to the master server to register
// ourselves with the global list.

void NET_Query_AddToMaster(net_addr_t *master_addr)
{
    auto *packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_ADD);
    NET_SendPacket(master_addr, packet);
    NET_FreePacket(packet);
}

// Process a packet received from the master server.

void NET_Query_AddResponse(net_packet_t *packet)
{
    unsigned int result = 0;

    if (!NET_ReadInt16(packet, &result))
    {
        return;
    }

    if (result != 0)
    {
        // Only show the message once.

        if (!registered_with_master)
        {
            fmt::print("Registered with master server at {}\n", MASTER_SERVER_ADDRESS);
            registered_with_master = true;
        }
    }
    else
    {
        // Always show rejections.

        fmt::print("Failed to register with master server at {}\n", MASTER_SERVER_ADDRESS);
    }

    got_master_response = true;
}

auto NET_Query_CheckAddedToMaster(bool *result) -> bool
{
    // Got response from master yet?

    if (!got_master_response)
    {
        return false;
    }

    *result = registered_with_master;
    return true;
}

// Send a query to the master server.

static void NET_Query_SendMasterQuery(net_addr_t *addr)
{
    auto *packet = NET_NewPacket(4);
    NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_QUERY);
    NET_SendPacket(addr, packet);
    NET_FreePacket(packet);

    // We also send a NAT_HOLE_PUNCH_ALL packet so that servers behind
    // NAT gateways will open themselves up to us.
    packet = NET_NewPacket(4);
    NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_NAT_HOLE_PUNCH_ALL);
    NET_SendPacket(addr, packet);
    NET_FreePacket(packet);
}

// Send a hole punch request to the master server for the server at the
// given address.
void NET_RequestHolePunch(net_context_t *context, net_addr_t *addr)
{
    auto *master_addr = NET_Query_ResolveMaster(context);
    if (master_addr == nullptr)
    {
        return;
    }

    auto *packet = NET_NewPacket(32);
    NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_NAT_HOLE_PUNCH);
    NET_WriteString(packet, NET_AddrToString(addr));
    NET_SendPacket(master_addr, packet);

    NET_FreePacket(packet);
    NET_ReleaseAddress(master_addr);
}

// Given the specified address, find the target associated.  If no
// target is found, and 'create' is true, a new target is created.
static auto GetTargetForAddr(net_addr_t *addr, bool create) -> query_target_t *
{
    for (auto &t : targets)
    {
        if (t.addr == addr)
        {
            return &t;
        }
    }

    if (!create)
    {
        return nullptr;
    }

    targets.emplace_back(query_target_t({ .type = QUERY_TARGET_SERVER,
        .state                                  = QUERY_TARGET_QUEUED,
        .addr                                   = addr,
        .data                                   = {},
        .ping_time                              = 0,
        .query_time                             = 0,
        .query_attempts                         = 0,
        .printed                                = false }));

    NET_ReferenceAddress(addr);

    return &targets.back();
}

static void FreeTargets()
{
    for (auto &t : targets)
    {
        NET_ReleaseAddress(t.addr);
    }
    targets.clear();
}

// Transmit a query packet

static void NET_Query_SendQuery(net_addr_t *addr)
{
    auto *request = NET_NewPacket(10);
    NET_WriteInt16(request, NET_PACKET_TYPE_QUERY);

    if (addr == nullptr)
    {
        NET_SendBroadcast(query_context, request);
    }
    else
    {
        NET_SendPacket(addr, request);
    }

    NET_FreePacket(request);
}

static void NET_Query_ParseResponse(net_addr_t *addr, net_packet_t *packet,
    net_query_callback_t callback,
    void *               user_data)
{
    // Read the header
    unsigned int packet_type = 0;
    if (!NET_ReadInt16(packet, &packet_type)
        || packet_type != NET_PACKET_TYPE_QUERY_RESPONSE)
    {
        return;
    }

    // Read query data
    net_querydata_t querydata;
    if (!NET_ReadQueryData(packet, &querydata))
    {
        return;
    }

    // Find the target that responded.
    auto *target = GetTargetForAddr(addr, false);

    // If the target is not found, it may be because we are doing
    // a LAN broadcast search, in which case we need to create a
    // target for the new responder.

    if (target == nullptr)
    {
        auto *broadcast_target = GetTargetForAddr(nullptr, false);

        // Not in broadcast mode, unexpected response that came out
        // of nowhere. Ignore.

        if (broadcast_target == nullptr || broadcast_target->state != QUERY_TARGET_QUERIED)
        {
            return;
        }

        // Create new target.
        target             = GetTargetForAddr(addr, true);
        target->state      = QUERY_TARGET_QUERIED;
        target->query_time = broadcast_target->query_time;
    }

    if (target->state != QUERY_TARGET_RESPONDED)
    {
        target->state = QUERY_TARGET_RESPONDED;
        memcpy(&target->data, &querydata, sizeof(net_querydata_t));

        // Calculate RTT.
        target->ping_time = I_GetTimeMS() - target->query_time;

        // Invoke callback to signal that we have a new address.
        callback(addr, &target->data, target->ping_time, user_data);
    }
}

// Parse a response packet from the master server.

static void NET_Query_ParseMasterResponse(net_addr_t *master_addr,
    net_packet_t *                                    packet)
{
    // Read the header.  We are only interested in query responses.
    unsigned int packet_type = 0;
    if (!NET_ReadInt16(packet, &packet_type)
        || packet_type != NET_MASTER_PACKET_TYPE_QUERY_RESPONSE)
    {
        return;
    }

    // Read a list of strings containing the addresses of servers
    // that the master knows about.

    for (;;)
    {
        auto *addr_str = NET_ReadString(packet);

        if (addr_str == nullptr)
        {
            break;
        }

        // Resolve address and add to targets list if it is not already
        // there.

        auto *addr = NET_ResolveAddress(query_context, addr_str);
        if (addr != nullptr)
        {
            GetTargetForAddr(addr, true);
            NET_ReleaseAddress(addr);
        }
    }

    // Mark the master as having responded.

    auto *target  = GetTargetForAddr(master_addr, true);
    target->state = QUERY_TARGET_RESPONDED;
}

static void NET_Query_ParsePacket(net_addr_t *addr, net_packet_t *packet,
    net_query_callback_t callback,
    void *               user_data)
{
    // This might be the master server responding.

    auto *target = GetTargetForAddr(addr, false);

    if (target != nullptr && target->type == QUERY_TARGET_MASTER)
    {
        NET_Query_ParseMasterResponse(addr, packet);
    }
    else
    {
        NET_Query_ParseResponse(addr, packet, callback, user_data);
    }
}

static void NET_Query_GetResponse(net_query_callback_t callback,
    void *                                             user_data)
{
    net_addr_t *  addr   = nullptr;
    net_packet_t *packet = nullptr;

    if (NET_RecvPacket(query_context, &addr, &packet))
    {
        NET_Query_ParsePacket(addr, packet, callback, user_data);
        NET_ReleaseAddress(addr);
        NET_FreePacket(packet);
    }
}

// Find a target we have not yet queried and send a query.

static void SendOneQuery()
{
    auto now = I_GetTimeMS();

    // Rate limit - only send one query every 50ms.

    if (now - last_query_time < 50)
    {
        return;
    }

    auto target = std::find_if(targets.begin(), targets.end(),
        [now](auto &t) { return (t.state == QUERY_TARGET_QUEUED
                                 || (t.state == QUERY_TARGET_QUERIED && now - t.query_time > QUERY_TIMEOUT_MS)); });

    if (target == targets.end())
    {
        return;
    }

    switch (target->type)
    {
    case QUERY_TARGET_SERVER:
        NET_Query_SendQuery(target->addr);
        break;
    //
    case QUERY_TARGET_BROADCAST:
        NET_Query_SendQuery(nullptr);
        break;
    //
    case QUERY_TARGET_MASTER:
        NET_Query_SendMasterQuery(target->addr);
        break;
    }

    target->state      = QUERY_TARGET_QUERIED;
    target->query_time = now;
    ++target->query_attempts;

    // int i = 0;
    // for (; i < num_targets; ++i)
    // {
    //     // Not queried yet?
    //     // Or last query timed out without a response?
    //
    //     if (targets[i].state == QUERY_TARGET_QUEUED
    //         || (targets[i].state == QUERY_TARGET_QUERIED
    //             && now - targets[i].query_time > QUERY_TIMEOUT_MS))
    //     {
    //         break;
    //     }
    // }

    // if (i >= num_targets)
    // {
    //     return;
    // }

    // Found a target to query.  Send a query; how to do this depends on
    // the target type.

    // switch (targets[i].type)
    // {
    // case QUERY_TARGET_SERVER:
    //     NET_Query_SendQuery(targets[i].addr);
    //     break;
    //
    // case QUERY_TARGET_BROADCAST:
    //     NET_Query_SendQuery(NULL);
    //     break;
    //
    // case QUERY_TARGET_MASTER:
    //     NET_Query_SendMasterQuery(targets[i].addr);
    //     break;
    // }

    //printf("Queried %s\n", NET_AddrToString(targets[i].addr));
    // targets[i].state      = QUERY_TARGET_QUERIED;
    // targets[i].query_time = now;
    // ++targets[i].query_attempts;

    last_query_time = now;
}

// Time out servers that have been queried and not responded.

static void CheckTargetTimeouts()
{
    auto now = I_GetTimeMS();

    for (auto &t : targets)
    {
        /*
        printf("target %i: state %i, queries %i, query time %i\n",
               i, targets[i].state, targets[i].query_attempts,
               now - targets[i].query_time);
        */

        // We declare a target to be "no response" when we've sent
        // multiple query packets to it (QUERY_MAX_ATTEMPTS) and
        // received no response to any of them.

        if (t.state == QUERY_TARGET_QUERIED
            && t.query_attempts >= QUERY_MAX_ATTEMPTS
            && now - t.query_time > QUERY_TIMEOUT_MS)
        {
            t.state = QUERY_TARGET_NO_RESPONSE;

            if (t.type == QUERY_TARGET_MASTER)
            {
                fmt::print(stderr, "NET_MasterQuery: no response from master server.\n");
            }
        }
    }
}

// If all targets have responded or timed out, returns true.

static auto AllTargetsDone() -> bool
{
    return std::find_if(targets.begin(), targets.end(), [](auto &t) { return (t.state != QUERY_TARGET_RESPONDED
                                                                              && t.state != QUERY_TARGET_NO_RESPONSE); }) == targets.end();

    // for (int i = 0; i < num_targets; ++i)
    // {
    //     if (targets[i].state != QUERY_TARGET_RESPONDED
    //         && targets[i].state != QUERY_TARGET_NO_RESPONSE)
    //     {
    //         return false;
    //     }
    // }
    //
    // return true;
}

// Polling function, invoked periodically to send queries and
// interpret new responses received from remote servers.
// Returns zero when the query sequence has completed and all targets
// have returned responses or timed out.

int NET_Query_Poll(net_query_callback_t callback, void *user_data)
{
    CheckTargetTimeouts();

    // Send a query.  This will only send a single query at once.

    SendOneQuery();

    // Check for a response

    NET_Query_GetResponse(callback, user_data);

    return !AllTargetsDone();
}

// Stop the query loop

static void NET_Query_ExitLoop(void)
{
    query_loop_running = false;
}

// Loop waiting for responses.
// The specified callback is invoked when a new server responds.

static void NET_Query_QueryLoop(net_query_callback_t callback, void *user_data)
{
    query_loop_running = true;

    while (query_loop_running && NET_Query_Poll(callback, user_data))
    {
        // Don't thrash the CPU

        I_Sleep(1);
    }
}

void NET_Query_Init(void)
{
    if (query_context == nullptr)
    {
        query_context = NET_NewContext();
        NET_AddModule(query_context, &net_sdl_module);
        net_sdl_module.InitClient();
    }

    targets.clear();

    printed_header = false;
}

// Callback that exits the query loop when the first server is found.
static void NET_Query_ExitCallback(net_addr_t *addr [[maybe_unused]], net_querydata_t *data [[maybe_unused]],
    unsigned int ping_time [[maybe_unused]], void *user_data [[maybe_unused]])
{
    NET_Query_ExitLoop();
}

// Search the targets list and find a target that has responded.
// If none have responded, returns NULL.

static auto FindFirstResponder() -> query_target_t *
{
    for (auto &t : targets)
    {
        if (t.type == QUERY_TARGET_SERVER
            && t.state == QUERY_TARGET_RESPONDED)
        {
            return &t;
        }
    }

    return nullptr;
}

// Return a count of the number of responses.

static auto GetNumResponses() -> int
{
    return std::count_if(targets.begin(), targets.end(), [](auto &t) { return (t.type == QUERY_TARGET_SERVER && t.state == QUERY_TARGET_RESPONDED); });
}

auto NET_StartLANQuery() -> int
{
    query_target_t *target;

    NET_Query_Init();

    // Add a broadcast target to the list.

    target       = GetTargetForAddr(nullptr, true);
    target->type = QUERY_TARGET_BROADCAST;

    return 1;
}

int NET_StartMasterQuery(void)
{
    net_addr_t *    master;
    query_target_t *target;

    NET_Query_Init();

    // Resolve master address and add to targets list.

    master = NET_Query_ResolveMaster(query_context);

    if (master == NULL)
    {
        return 0;
    }

    target       = GetTargetForAddr(master, true);
    target->type = QUERY_TARGET_MASTER;
    NET_ReleaseAddress(master);

    return 1;
}

// -----------------------------------------------------------------------

static void formatted_printf(int wide, const char *s, ...) PRINTF_ATTR(2, 3);
static void formatted_printf(int wide, const char *s, ...)
{
    va_list args;
    int     i;

    va_start(args, s);
    i = vprintf(s, args);
    va_end(args);

    while (i < wide)
    {
        putchar(' ');
        ++i;
    }
}

static constexpr const char *GameDescription(GameMode_t mode, int mission)
{
    switch (mission)
    {
    case doom:
        if (mode == GameMode_t::shareware)
            return "swdoom";
        else if (mode == GameMode_t::registered)
            return "regdoom";
        else if (mode == GameMode_t::retail)
            return "ultdoom";
        else
            return "doom";
    case doom2:
        return "doom2";
    case pack_tnt:
        return "tnt";
    case pack_plut:
        return "plutonia";
    case pack_chex:
        return "chex";
    case pack_hacx:
        return "hacx";
    case heretic:
        return "heretic";
    case hexen:
        return "hexen";
    case strife:
        return "strife";
    default:
        return "?";
    }
}

static void PrintHeader(void)
{
    int i;

    putchar('\n');
    formatted_printf(5, "Ping");
    formatted_printf(18, "Address");
    formatted_printf(8, "Players");
    puts("Description");

    for (i = 0; i < 70; ++i)
        putchar('=');
    putchar('\n');
}

// Callback function that just prints information in a table.

static void NET_QueryPrintCallback(net_addr_t *addr,
    net_querydata_t *                          data,
    unsigned int                               ping_time,
    void *                                     user_data [[maybe_unused]])
{
    // If this is the first server, print the header.

    if (!printed_header)
    {
        PrintHeader();
        printed_header = true;
    }

    formatted_printf(5, "%4i", ping_time);
    formatted_printf(22, "%s", NET_AddrToString(addr));
    formatted_printf(4, "%i/%i ", data->num_players,
        data->max_players);

    if (data->gamemode != GameMode_t::undetermined)
    {
        printf("(%s) ", GameDescription(data->gamemode, data->gamemission));
    }

    if (data->server_state)
    {
        printf("(game running) ");
    }

    printf("%s\n", data->description);
}

void NET_LANQuery(void)
{
    if (NET_StartLANQuery())
    {
        printf("\nSearching for servers on local LAN ...\n");

        NET_Query_QueryLoop(NET_QueryPrintCallback, NULL);

        printf("\n%i server(s) found.\n", GetNumResponses());
        FreeTargets();
    }
}

void NET_MasterQuery(void)
{
    if (NET_StartMasterQuery())
    {
        printf("\nSearching for servers on Internet ...\n");

        NET_Query_QueryLoop(NET_QueryPrintCallback, NULL);

        printf("\n%i server(s) found.\n", GetNumResponses());
        FreeTargets();
    }
}

void NET_QueryAddress(char *addr_str)
{
    net_addr_t *    addr;
    query_target_t *target;

    NET_Query_Init();

    addr = NET_ResolveAddress(query_context, addr_str);

    if (addr == NULL)
    {
        S_Error(fmt::format("NET_QueryAddress: Host '{}' not found!", addr_str));
    }

    // Add the address to the list of targets.

    target = GetTargetForAddr(addr, true);

    printf("\nQuerying '%s'...\n", addr_str);

    // Run query loop.

    NET_Query_QueryLoop(NET_Query_ExitCallback, NULL);

    // Check if the target responded.

    if (target->state == QUERY_TARGET_RESPONDED)
    {
        NET_QueryPrintCallback(addr, &target->data, target->ping_time, NULL);
        NET_ReleaseAddress(addr);
        FreeTargets();
    }
    else
    {
        S_Error(fmt::format("No response from '{}'", addr_str));
    }
}

net_addr_t *NET_FindLANServer(void)
{
    query_target_t *target;
    query_target_t *responder;
    net_addr_t *    result;

    NET_Query_Init();

    // Add a broadcast target to the list.

    target       = GetTargetForAddr(NULL, true);
    target->type = QUERY_TARGET_BROADCAST;

    // Run the query loop, and stop at the first target found.

    NET_Query_QueryLoop(NET_Query_ExitCallback, NULL);

    responder = FindFirstResponder();

    if (responder != NULL)
    {
        result = responder->addr;
        NET_ReferenceAddress(result);
    }
    else
    {
        result = NULL;
    }

    FreeTargets();
    return result;
}

// Block until a packet of the given type is received from the given
// address.

static net_packet_t *BlockForPacket(net_addr_t *addr, unsigned int packet_type,
    unsigned int timeout_ms)
{
    net_packet_t *packet;
    net_addr_t *  packet_src;
    unsigned int  read_packet_type;
    unsigned int  start_time;

    start_time = I_GetTimeMS();

    while (I_GetTimeMS() < static_cast<int>(start_time + timeout_ms))
    {
        if (!NET_RecvPacket(query_context, &packet_src, &packet))
        {
            I_Sleep(20);
            continue;
        }

        // Caller doesn't need additional reference.
        NET_ReleaseAddress(packet_src);

        if (packet_src == addr
            && NET_ReadInt16(packet, &read_packet_type)
            && packet_type == read_packet_type)
        {
            return packet;
        }

        NET_FreePacket(packet);
    }

    // Timeout - no response.

    return NULL;
}

// Query master server for secure demo start seed value.

bool NET_StartSecureDemo(prng_seed_t seed)
{
    net_packet_t *request, *response;
    net_addr_t *  master_addr;
    char *        signature;
    bool          result;

    NET_Query_Init();
    master_addr = NET_Query_ResolveMaster(query_context);

    // Send request packet to master server.

    request = NET_NewPacket(10);
    NET_WriteInt16(request, NET_MASTER_PACKET_TYPE_SIGN_START);
    NET_SendPacket(master_addr, request);
    NET_FreePacket(request);

    // Block for response and read contents.
    // The signed start message will be saved for later.

    response = BlockForPacket(master_addr,
        NET_MASTER_PACKET_TYPE_SIGN_START_RESPONSE,
        SIGNATURE_TIMEOUT_MS);

    result = false;

    if (response != NULL)
    {
        if (NET_ReadPRNGSeed(response, seed))
        {
            signature = NET_ReadString(response);

            if (signature != NULL)
            {
                securedemo_start_message = M_StringDuplicate(signature);
                result                   = true;
            }
        }

        NET_FreePacket(response);
    }

    return result;
}

// Query master server for secure demo end signature.

char *NET_EndSecureDemo(sha1_digest_t demo_hash)
{
    net_packet_t *request, *response;
    net_addr_t *  master_addr;
    char *        signature;

    master_addr = NET_Query_ResolveMaster(query_context);

    // Construct end request and send to master server.

    request = NET_NewPacket(10);
    NET_WriteInt16(request, NET_MASTER_PACKET_TYPE_SIGN_END);
    NET_WriteSHA1Sum(request, demo_hash);
    NET_WriteString(request, securedemo_start_message);
    NET_SendPacket(master_addr, request);
    NET_FreePacket(request);

    // Block for response. The response packet simply contains a string
    // with the ASCII signature.

    response = BlockForPacket(master_addr,
        NET_MASTER_PACKET_TYPE_SIGN_END_RESPONSE,
        SIGNATURE_TIMEOUT_MS);

    if (response == NULL)
    {
        return NULL;
    }

    signature = NET_ReadString(response);

    NET_FreePacket(response);

    return signature;
}
