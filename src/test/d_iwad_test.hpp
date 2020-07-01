#ifndef __D_IWAD_TEST_HPP__
#define __D_IWAD_TEST_HPP__

#include "catch2/catch.hpp"
#include "../d_iwad.hpp"

TEST_CASE("d_iwad tests", "env_view") {
    auto test_enviroment_varible_as_string_view = env_view("HOME");

    REQUIRE(test_enviroment_varible_as_string_view.size() > 0);
    REQUIRE(test_enviroment_varible_as_string_view.data() != nullptr);
}

#endif