
#ifndef __TEST_M_ARGV_HPP__
#define __TEST_M_ARGV_HPP__

#include "../m_argv.hpp"

#include "catch2/catch.hpp"

#include <memory>
#include <span>
#include <string>
#include <vector>

TEST_CASE("{m_argv}", "[c_Arguments]")
{
    char *      charStarStar[] = { "./somefilename.ext", "bull licks", "doom2.wad" };
    c_Arguments testArgs(std::size(charStarStar), &charStarStar[0]);

    SECTION("initialization")
    {
        REQUIRE(&testArgs != nullptr);
    }

    SECTION("at() method with empty args")
    {
        CHECK(testArgs.at(0) == "./somefilename.ext");
    }

    SECTION("size() method with 4 args")
    {
        REQUIRE(testArgs.size() == 3);
    }

    SECTION("importArguments(span<char *>) ")
    {
        REQUIRE(testArgs.size() == 3);
        char *css[] = { "more", "less" };
        testArgs.importArguments(css);
        CHECK(testArgs.size() == 5);
    }


    SECTION("find(const std::string_view findMe, const int num_args) ")
    {
        REQUIRE(testArgs.size() == 3);
        CHECK(testArgs.find("notfound", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("doom2.wad", 0) == 2);
        CHECK(testArgs.find("doom2.wad", 1) == c_Arguments::NotFound);
    }

    SECTION("d_loop.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-privateserver", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-autojoin", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-server", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-drone", 0) == c_Arguments::NotFound);
    }

    SECTION("deh_main.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nocheats", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-deh", 0) == c_Arguments::NotFound);
    }

    SECTION("i_sound.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nosound", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-nomusic", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-nosfx", 0) == c_Arguments::NotFound);
    }

    SECTION("i_video.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nofullscreen", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-fullscreen", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-nomouse", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-window", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-noblit", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-1", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-2", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-3", 0) == c_Arguments::NotFound);
    }

    SECTION("net_server.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-privateserver", 0) == c_Arguments::NotFound);
    }

    SECTION("w_file.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-mmap", 0) == c_Arguments::NotFound);
    }

    SECTION("d_main.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-testcontrols", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-localsearch", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-flipweapons", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-fliplevels", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-deathmatch", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-nomonsters", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-dedicated", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-mergedump", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-altdeath", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-lumpdump", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-respawn", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-devparm", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-search", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-cdrom", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-turbo", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-fast", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-avg", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-dm3", 0) == c_Arguments::NotFound);
    }

    SECTION("d_net.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-solo-net", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-right", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-left", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-avg", 0) == c_Arguments::NotFound);
    }

    SECTION("g_game.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nomonsters", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-solo-net", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-respawn", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-netdemo", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-nodraw", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-fast", 0) == c_Arguments::NotFound);
    }

    SECTION("m_menu.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-opldev", 0) == c_Arguments::NotFound);
    }

    SECTION("p_setup.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-reject_pad_with_ff", 0) == c_Arguments::NotFound);
        CHECK(testArgs.find("-blockmap", 0) == c_Arguments::NotFound);
    }

    SECTION("mode.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-game", 0) == c_Arguments::NotFound);
    }

    // M_ParmExists(?)
}


TEST_CASE("{m_argv} part2", "[c_Arguments][found]")
{
    char *charStarStar[] = { "-privateserver", "-autojoin", "-server", "-drone", "-nocheats",
        "-deh", "-nosound", "-nomusic", "-nosfx", "-nofullscreen", "-fullscreen", "-nomouse",
        "-window", "-noblit", "-1", "-2", "-3", "-mmap", "-testcontrols", "-localsearch",
        "-flipweapons", "-fliplevels", "-deathmatch", "-nomonsters", "-dedicated", "-mergedump",
        "-altdeath", "-lumpdump", "-respawn", "-devparm", "-search", "-cdrom", "-turbo", "-fast",
        "-avg", "-dm3", "-solo-net", "-right", "-left", "-netdemo",
        "-nodraw", "-opldev", "-reject_pad_with_ff", "-blockmap", "-game" };

    c_Arguments testArgs(std::size(charStarStar), &charStarStar[0]);


    SECTION("d_loop.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-privateserver", 0) == 0);
        CHECK(testArgs.find("-autojoin", 0) == 1);
        CHECK(testArgs.find("-server", 0) == 2);
        CHECK(testArgs.find("-drone", 0) == 3);
    }

    SECTION("deh_main.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nocheats", 0) == 4);
        CHECK(testArgs.find("-deh", 0) == 5);
    }

    SECTION("i_sound.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nosound", 0) == 6);
        CHECK(testArgs.find("-nomusic", 0) == 7);
        CHECK(testArgs.find("-nosfx", 0) == 8);
    }

    SECTION("i_video.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nofullscreen", 0) == 9);
        CHECK(testArgs.find("-fullscreen", 0) == 10);
        CHECK(testArgs.find("-nomouse", 0) == 11);
        CHECK(testArgs.find("-window", 0) == 12);
        CHECK(testArgs.find("-noblit", 0) == 13);
        CHECK(testArgs.find("-1", 0) == 14);
        CHECK(testArgs.find("-2", 0) == 15);
        CHECK(testArgs.find("-3", 0) == 16);
    }

    SECTION("net_server.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-privateserver", 0) == 0);
    }

    SECTION("w_file.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-mmap", 0) == 17);
    }

    SECTION("d_main.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-testcontrols", 0) == 18);
        CHECK(testArgs.find("-localsearch", 0) == 19);
        CHECK(testArgs.find("-flipweapons", 0) == 20);
        CHECK(testArgs.find("-fliplevels", 0) == 21);
        CHECK(testArgs.find("-deathmatch", 0) == 22);
        CHECK(testArgs.find("-nomonsters", 0) == 23);
        CHECK(testArgs.find("-dedicated", 0) == 24);
        CHECK(testArgs.find("-mergedump", 0) == 25);
        CHECK(testArgs.find("-altdeath", 0) == 26);
        CHECK(testArgs.find("-lumpdump", 0) == 27);
        CHECK(testArgs.find("-respawn", 0) == 28);
        CHECK(testArgs.find("-devparm", 0) == 29);
        CHECK(testArgs.find("-search", 0) == 30);
        CHECK(testArgs.find("-cdrom", 0) == 31);
        CHECK(testArgs.find("-turbo", 0) == 32);
        CHECK(testArgs.find("-fast", 0) == 33);
        CHECK(testArgs.find("-avg", 0) == 34);
        CHECK(testArgs.find("-dm3", 0) == 35);
    }

    SECTION("d_net.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-solo-net", 0) == 36);
        CHECK(testArgs.find("-right", 0) == 37);
        CHECK(testArgs.find("-left", 0) == 38);
        CHECK(testArgs.find("-avg", 0) == 34);
    }

    SECTION("g_game.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-nomonsters", 0) == 23);
        CHECK(testArgs.find("-solo-net", 0) == 36);
        CHECK(testArgs.find("-respawn", 0) == 28);
        CHECK(testArgs.find("-netdemo", 0) == 39);
        CHECK(testArgs.find("-nodraw", 0) == 40);
        CHECK(testArgs.find("-fast", 0) == 33);
    }

    SECTION("m_menu.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-opldev", 0) == 41);
    }

    SECTION("p_setup.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-reject_pad_with_ff", 0) == 42);
        CHECK(testArgs.find("-blockmap", 0) == 43);
    }

    SECTION("mode.cpp possible options, expecting not found")
    {
        CHECK(testArgs.find("-game", 0) == 44);
    }

    // M_ParmExists(?)
}

#endif //__TEST_M_ARGV_HPP__
