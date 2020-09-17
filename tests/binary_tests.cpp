/*
* MIT License
*
* Copyright (c) 2019 Luka Arnecic
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstddef>

#include <catch2/catch.hpp>

#include <flib/binary.hpp>

#include "testing.hpp"

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