#ifndef __D_IWAD_TEST_HPP__
#define __D_IWAD_TEST_HPP__

#include "gtest/gtest.h"
#include "../d_iwad.hpp"

TEST(d_iwad_tests, env_view)
{
    auto test_enviroment_varible_as_string_view = env_view("HOME");
    
    EXPECT_GT(test_enviroment_varible_as_string_view.size(), 0);
    EXPECT_NE(test_enviroment_varible_as_string_view.data(), nullptr);
}

#endif