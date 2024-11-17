// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <cstdint>

#include <flib/flags.hpp>

#include <catch2/catch2.hpp>

namespace
{
  enum class test_flags
    : uint8_t
  {
    option_1       = 0b00000001,
    option_2       = 0b00000010,
    option_3       = 0b00000100,
    option_4       = 0b00001000,
    option_5       = 0b00010000,
    option_6       = 0b00100000,
    option_7       = 0b01000000,
    option_8       = 0b10000000,
    // test values
    option_none    = 0b00000000,
    option_12      = 0b00000011,
    option_34      = 0b00001100,
    option_123     = 0b00000111,
    option_124     = 0b00001011,
    option_1234    = 0b00001111,
    option_2345678 = 0b11111110,
    option_all     = 0b11111111
  };
}

TEST_CASE("Flags tests - Flag setting", "[flags]")
{
  SECTION("Check if one flag set")
  {
    constexpr auto test_value = ::test_flags::option_1;
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_1));
  }
  SECTION("Check if multiple flags set")
  {
    constexpr auto test_value = ::test_flags::option_2345678;
    REQUIRE(!flib::is_flag_set(test_value, ::test_flags::option_1));
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_2));
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_3));
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_4));
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_5));
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_6));
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_7));
    REQUIRE(flib::is_flag_set(test_value, ::test_flags::option_8));
  }
}

TEST_CASE("Flags tests - Biwise NOT", "[flags]")
{
  using namespace flib;
  SECTION("Negate unset flag")
  {
    constexpr auto test_value = ~static_cast<::test_flags>(0);
    REQUIRE(test_value == ::test_flags::option_all);
  }
  SECTION("Negate specific value")
  {
    constexpr auto test_value = ~::test_flags::option_1;
    REQUIRE(test_value == ::test_flags::option_2345678);
  }
}

TEST_CASE("Flags tests - Biwise AND", "[flags]")
{
  using namespace flib;
  SECTION("AND (a & b)")
  {
    constexpr auto test_value = ::test_flags::option_1 & ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_none);
  }
  SECTION("AND (a & a)")
  {
    constexpr auto test_value = ::test_flags::option_1 & ::test_flags::option_1;
    REQUIRE(test_value == ::test_flags::option_1);
  }
  SECTION("AND (ab & b)")
  {
    constexpr auto test_value = ::test_flags::option_12 & ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_2);
  }
  SECTION("AND (abc & abd)")
  {
    constexpr auto test_value = ::test_flags::option_123 & ::test_flags::option_124;
    REQUIRE(test_value == ::test_flags::option_12);
  }
  SECTION("Assignment AND (a &= b)")
  {
    constexpr auto test_value = ::test_flags::option_1 & ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_none);
  }
  SECTION("Assignment AND (a &= a)")
  {
    constexpr auto test_value = ::test_flags::option_1 & ::test_flags::option_1;
    REQUIRE(test_value == ::test_flags::option_1);
  }
  SECTION("Assignment AND (ab &= b)")
  {
    constexpr auto test_value = ::test_flags::option_12 & ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_2);
  }
  SECTION("Assignment AND (abc &= abd)")
  {
    constexpr auto test_value = ::test_flags::option_123 & ::test_flags::option_124;
    REQUIRE(test_value == ::test_flags::option_12);
  }
}

TEST_CASE("Flags tests - Biwise OR", "[flags]")
{
  using namespace flib;
  SECTION("OR (a | b)")
  {
    constexpr auto test_value = ::test_flags::option_1 | ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_12);
  }
  SECTION("OR (a | a)")
  {
    constexpr auto test_value = ::test_flags::option_1 | ::test_flags::option_1;
    REQUIRE(test_value == ::test_flags::option_1);
  }
  SECTION("OR (ab | b)")
  {
    constexpr auto test_value = ::test_flags::option_12 | ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_12);
  }
  SECTION("OR (abc | abd)")
  {
    constexpr auto test_value = ::test_flags::option_123 | ::test_flags::option_124;
    REQUIRE(test_value == ::test_flags::option_1234);
  }
  SECTION("Assignment OR (a |= b)")
  {
    constexpr auto test_value = ::test_flags::option_1 | ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_12);
  }
  SECTION("Assignment OR (a |= a)")
  {
    constexpr auto test_value = ::test_flags::option_1 | ::test_flags::option_1;
    REQUIRE(test_value == ::test_flags::option_1);
  }
  SECTION("Assignment OR (ab |= b)")
  {
    constexpr auto test_value = ::test_flags::option_12 | ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_12);
  }
  SECTION("Assignment OR (abc |= abd)")
  {
    constexpr auto test_value = ::test_flags::option_123 | ::test_flags::option_124;
    REQUIRE(test_value == ::test_flags::option_1234);
  }
}

TEST_CASE("Flags tests - Biwise XOR", "[flags]")
{
  using namespace flib;
  SECTION("XOR (a ^ b)")
  {
    constexpr auto test_value = ::test_flags::option_1 ^ ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_12);
  }
  SECTION("XOR (a ^ a)")
  {
    constexpr auto test_value = ::test_flags::option_1 ^ ::test_flags::option_1;
    REQUIRE(test_value == ::test_flags::option_none);
  }
  SECTION("XOR (ab ^ b)")
  {
    constexpr auto test_value = ::test_flags::option_12 ^ ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_1);
  }
  SECTION("XOR (abc ^ abd)")
  {
    constexpr auto test_value = ::test_flags::option_123 ^ ::test_flags::option_124;
    REQUIRE(test_value == ::test_flags::option_34);
  }
  SECTION("Assignment XOR (a ^= b)")
  {
    constexpr auto test_value = ::test_flags::option_1 ^ ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_12);
  }
  SECTION("Assignment XOR (a ^= a)")
  {
    constexpr auto test_value = ::test_flags::option_1 ^ ::test_flags::option_1;
    REQUIRE(test_value == ::test_flags::option_none);
  }
  SECTION("Assignment XOR (ab ^= b)")
  {
    constexpr auto test_value = ::test_flags::option_12 ^ ::test_flags::option_2;
    REQUIRE(test_value == ::test_flags::option_1);
  }
  SECTION("Assignment XOR (abc ^= abd)")
  {
    constexpr auto test_value = ::test_flags::option_123 ^ ::test_flags::option_124;
    REQUIRE(test_value == ::test_flags::option_34);
  }
}