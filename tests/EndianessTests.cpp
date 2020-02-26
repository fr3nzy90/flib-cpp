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

#include <algorithm>

#include <catch2/catch.hpp>

#include "flib/Endianess.hpp"

namespace
{
  template<class Type>
  bool ReverseCompare(const Type* obj1, const Type* obj2, const std::size_t size = sizeof(Type))
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

TEST_CASE("Endianess tests - Sanity check", "[Endianess]")
{
  REQUIRE((flib::Endianess::Native() == flib::Endianess::BigByte ||
    flib::Endianess::Native() == flib::Endianess::BigWord ||
    flib::Endianess::Native() == flib::Endianess::LittleWord ||
    flib::Endianess::Native() == flib::Endianess::LittleByte));
}

TEST_CASE("Endianess tests - 2 byte swap", "[Endianess]")
{
  const uint16_t originalValue = 0x0123u;
  auto reversedValue = originalValue;
  flib::ByteSwap(reversedValue);
  REQUIRE(ReverseCompare(&originalValue, &reversedValue));
}

TEST_CASE("Endianess tests - 4 byte swap", "[Endianess]")
{
  const uint32_t originalValue = 0x01234567ul;
  auto reversedValue = originalValue;
  flib::ByteSwap(reversedValue);
  REQUIRE(ReverseCompare(&originalValue, &reversedValue));
}

TEST_CASE("Endianess tests - 8 byte swap", "[Endianess]")
{
  const uint64_t originalValue = 0x0123456789abcdefull;
  auto reversedValue = originalValue;
  flib::ByteSwap(reversedValue);
  REQUIRE(ReverseCompare(&originalValue, &reversedValue));
}

TEST_CASE("Endianess tests - Multi byte swap", "[Endianess]")
{
  const uint8_t originalValue[] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef };
  uint8_t reversedValue[8];
  std::copy(&originalValue[0], &originalValue[8], &reversedValue[0]);
  flib::ByteSwap(reversedValue, 8);
  REQUIRE(ReverseCompare(&originalValue, &reversedValue, 8));
}