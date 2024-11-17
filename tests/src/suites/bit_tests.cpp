// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/bit.hpp>

#include <cstddef>
#include <algorithm>

#include <catch2/catch2.hpp>

TEST_CASE("Bit tests - Endianess", "[bit]")
{
  REQUIRE((flib::endian::native() == flib::endian::big_byte ||
    flib::endian::native() == flib::endian::big_word ||
    flib::endian::native() == flib::endian::little_word ||
    flib::endian::native() == flib::endian::little_byte));
}

TEST_CASE("Bit tests - Byte swapping", "[bit]")
{
  SECTION("2 byte swap")
  {
    uint16_t value = 0x0123u;
    const uint16_t reversed = 0x2301u;
    flib::byteswap(value);
    REQUIRE(value == reversed);
  }
  SECTION("4 byte swap")
  {
    uint32_t value = 0x01234567ul;
    const uint32_t reversed = 0x67452301ul;
    flib::byteswap(value);
    REQUIRE(value == reversed);
  }
  SECTION("8 byte swap")
  {
    uint64_t value = 0x0123456789abcdefull;
    const uint64_t reversed = 0xefcdab8967452301ull;
    flib::byteswap(value);
    REQUIRE(value == reversed);
  }
  SECTION("Multi byte swap")
  {
    uint8_t bytes[] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef };
    const uint8_t reversed[] = { 0xef,0xcd,0xab,0x89,0x67,0x45,0x23,0x01 };
    flib::byteswap(bytes, bytes + sizeof(bytes));
    REQUIRE(std::equal(std::cbegin(bytes), std::cend(bytes), std::cbegin(reversed)));
  }
}