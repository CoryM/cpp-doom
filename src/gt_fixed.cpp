#include <gtest/gtest.h>

import m_fixed;

constexpr fixed_t fSmall = 1;
constexpr fixed_t fLarge = 0x100000;
constexpr fixed_t fZero = 0.0 * FRACUNIT;
constexpr fixed_t fHalf = 0.5 * FRACUNIT;
constexpr fixed_t fOne = 1.0 * FRACUNIT;
constexpr fixed_t fTwo = 2.0 * FRACUNIT;
constexpr fixed_t fFour = 4.0 * FRACUNIT;


// Test the FixedMul function.
TEST(FixedMul, Test1)
{
    EXPECT_EQ(FixedMul(fOne, fOne), fOne);
    EXPECT_EQ(FixedMul(fOne, fTwo), fTwo);
    EXPECT_EQ(FixedMul(fTwo, fOne), fTwo);
    EXPECT_EQ(FixedMul(fTwo, fTwo), fFour);
}

// Test the FixedDiv function.
TEST(FixedDiv, Test1)
{
    EXPECT_EQ(FixedDiv(fOne, fOne), fOne);
    EXPECT_EQ(FixedDiv(fTwo, fOne), fTwo);
    EXPECT_EQ(FixedDiv(fTwo, fTwo), fOne);
    EXPECT_EQ(FixedDiv(fOne, fTwo), fHalf);
}

// Test the FixedDiv function with a division by zero.
TEST(FixedDiv, DivZero)
{
    EXPECT_EQ(FixedDiv(fOne, fZero), std::numeric_limits<int32_t>::max());
    EXPECT_EQ(FixedDiv(fZero, fOne), fZero);
}

// Test the FixedDiv function with a division by a number that is too large.
TEST(FixedDiv, DivLargeSmall)
{
    EXPECT_EQ(FixedDiv(fLarge, fSmall), std::numeric_limits<int32_t>::max());
    EXPECT_EQ(FixedDiv(fLarge, -fSmall), std::numeric_limits<int32_t>::min());
    EXPECT_EQ(FixedDiv(-fLarge, fSmall), std::numeric_limits<int32_t>::min());
    EXPECT_EQ(FixedDiv(-fLarge, -fSmall), std::numeric_limits<int32_t>::max());
}

// Test the FixedDiv function with a division by a negative number.
TEST(FixedDiv, Signed)
{
    EXPECT_EQ(FixedDiv(fOne, -fOne), -fOne);
    EXPECT_EQ(FixedDiv(-fOne, fOne), -fOne);
    EXPECT_EQ(FixedDiv(-fOne, -fOne), fOne);
} 
