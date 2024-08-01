#include <gtest/gtest.h>

import i_swap;

// Test the endian functions.
//  is_big_endian
//  is_little_endian
//  big
//  little
//  SHORT
//  LONG

TEST(Endian, QuickTest)
{
    if constexpr (endian::is_little_endian()) {
        EXPECT_EQ(endian::is_big_endian(), false);
        EXPECT_EQ(endian::is_little_endian(), true);
        EXPECT_EQ(endian::big(0x12345678), 0x78563412);
        EXPECT_EQ(endian::little(0x12345678), 0x12345678);
        EXPECT_EQ(endian::SHORT(0x1234), 0x1234);
        EXPECT_EQ(endian::LONG(0x12345678), 0x12345678);
    }
    else {
        EXPECT_EQ(endian::is_big_endian(), true);
        EXPECT_EQ(endian::is_little_endian(), false);
        EXPECT_EQ(endian::big(0x12345678), 0x12345678);
        EXPECT_EQ(endian::little(0x12345678), 0x78563412);
        EXPECT_EQ(endian::SHORT(0x1234), 0x3412);
        EXPECT_EQ(endian::LONG(0x12345678), 0x78563412);
    }
}