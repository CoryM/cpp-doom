
#ifndef __TEST_FMT_HPP__
#define __TEST_FMT_HPP__

#include <string>
#include "catch2/catch.hpp"
#include <fmt/core.h>

TEST_CASE("{fmt}", "[fmt]" ) {

    SECTION( "fmt::format() string insertion" ) {
        std::string s = fmt::format("I'd rather be {1} than {0}.", "right", "happy");

        CHECK(s == "I'd rather be happy than right.");
    }

    SECTION( "fmt::format() number insertion" ) {
        std::string s = fmt::format("{}", 42);

        CHECK(s == "42");
    }
}

#endif //__TEST_FMT_HPP__