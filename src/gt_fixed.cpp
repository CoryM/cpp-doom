#include <gtest/gtest.h>

import m_fixed;

constexpr fixed_t fSmall = 1;
constexpr fixed_t fLarge = 0x100000;
constexpr fixed_t fZero = 0.0 * FRACUNIT;
constexpr fixed_t fHalf = 0.5 * FRACUNIT;
constexpr fixed_t fOne = 1.0 * FRACUNIT;
constexpr fixed_t fTwo = 2.0 * FRACUNIT;
constexpr fixed_t fFour = 4.0 * FRACUNIT;
constexpr fixed_t fMax = std::numeric_limits<int32_t>::max();
constexpr fixed_t fMin = std::numeric_limits<int32_t>::min();

// Test Fixed
TEST(Fixed, QuickTest) {
    EXPECT_EQ(FRACBITS, 16);
    EXPECT_EQ(FRACUNIT, 0x10000);
    EXPECT_EQ(FRACUNIT, 65536);
}


// Test the FixedMul function.
TEST(Fixed, FixedMul)
{
    EXPECT_EQ(FixedMul(fOne, fOne), fOne);
    EXPECT_EQ(FixedMul(fOne, fTwo), fTwo);
    EXPECT_EQ(FixedMul(fTwo, fOne), fTwo);
    EXPECT_EQ(FixedMul(fTwo, fTwo), fFour);

    // Multiplication by zero
    EXPECT_EQ(FixedMul(fOne, fZero), fZero);
    EXPECT_EQ(FixedMul(fZero, fOne), fZero);

    // Multiplication by a negative number.
    EXPECT_EQ(FixedMul(fOne, -fOne), -fOne);
    EXPECT_EQ(FixedMul(-fOne, fOne), -fOne);
    EXPECT_EQ(FixedMul(-fOne, -fOne), fOne);
    EXPECT_EQ(FixedMul(fOne, -fTwo), -fTwo);

    // Multiplication by extremes. (Rollover test)
    if constexpr (SafeFixed) {
        EXPECT_EQ(FixedMul(fMax, fMax), fMax);
        EXPECT_EQ(FixedMul(fMax, fMin), fMin);
        EXPECT_EQ(FixedMul(fMin, fMax), fMin);
        EXPECT_EQ(FixedMul(fMin, fMin), fMax);
    }
    else {
        EXPECT_EQ(FixedMul(fMax, fMax), -65536);
        EXPECT_EQ(FixedMul(fMax, fMin), 32768);
        EXPECT_EQ(FixedMul(fMin, fMax), 32768);
        EXPECT_EQ(FixedMul(fMin, fMin), 0);
    }
}

// Test the FixedDiv function.
TEST(Fixed, FixedDiv)
{
    EXPECT_EQ(FixedDiv(fOne, fOne), fOne);
    EXPECT_EQ(FixedDiv(fTwo, fOne), fTwo);
    EXPECT_EQ(FixedDiv(fTwo, fTwo), fOne);
    EXPECT_EQ(FixedDiv(fOne, fTwo), fHalf);
    
    // Division by zero
    EXPECT_EQ(FixedDiv(fOne, fZero), std::numeric_limits<int32_t>::max());
    EXPECT_EQ(FixedDiv(fZero, fOne), fZero);

    // Division by a negative number.
    EXPECT_EQ(FixedDiv(fOne, -fOne), -fOne);
    EXPECT_EQ(FixedDiv(-fOne, fOne), -fOne);
    EXPECT_EQ(FixedDiv(-fOne, -fOne), fOne);
    EXPECT_EQ(FixedDiv(fOne, -fTwo), -fHalf);

    // Division by extremes.
    EXPECT_EQ(FixedDiv(fLarge, fSmall), std::numeric_limits<int32_t>::max());
    EXPECT_EQ(FixedDiv(fLarge, -fSmall), std::numeric_limits<int32_t>::min());
    EXPECT_EQ(FixedDiv(-fLarge, fSmall), std::numeric_limits<int32_t>::min());
    EXPECT_EQ(FixedDiv(-fLarge, -fSmall), std::numeric_limits<int32_t>::max());
}

