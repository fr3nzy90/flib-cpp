// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/binary.hpp>

#include <cstddef>
#include <algorithm>

#include <catch2/catch2.hpp>

namespace
{
  template<class T>
  inline bool reverse_compare(const T* obj1, const T* obj2, const std::size_t size = sizeof(T))
  {
    for (std::size_t i = 0; i < size; ++i)
    {
      if (reinterpret_cast<const uint8_t*>(obj1)[i] != reinterpret_cast<const uint8_t*>(obj2)[size - i - 1])
      {
        return false;
      }
    }
    return true;
  }
}

TEST_CASE("Binary tests - Endianess", "[binary]")
{
  REQUIRE((flib::endianess::native() == flib::endianess::big_byte ||
    flib::endianess::native() == flib::endianess::big_word ||
    flib::endianess::native() == flib::endianess::little_word ||
    flib::endianess::native() == flib::endianess::little_byte));
}

TEST_CASE("Binary tests - Byte swapping", "[binary]")
{
  SECTION("2 byte swap")
  {
    const uint16_t original = 0x0123u;
    auto reversed = original;
    flib::byte_swap(reversed);
    REQUIRE(::reverse_compare(&original, &reversed));
  }
  SECTION("4 byte swap")
  {
    const uint32_t original = 0x01234567ul;
    auto reversed = original;
    flib::byte_swap(reversed);
    REQUIRE(::reverse_compare(&original, &reversed));
  }
  SECTION("8 byte swap")
  {
    const uint64_t original = 0x0123456789abcdefull;
    auto reversed = original;
    flib::byte_swap(reversed);
    REQUIRE(::reverse_compare(&original, &reversed));
  }
  SECTION("Multi byte swap")
  {
    const uint8_t original[] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef };
    uint8_t reversed[sizeof(original)];
    std::copy(&original[0], &original[sizeof(original)], &reversed[0]);
    flib::byte_swap(reversed, reversed + sizeof(reversed));
    REQUIRE(::reverse_compare(&original, &reversed, sizeof(reversed)));
  }
}