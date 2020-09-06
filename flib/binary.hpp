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

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace flib
{
  // Structure for determination of platform byte endianess
  struct endianess
  {
  public:
    // Method for retrieving native platform byte endianess
    //
    // Returns:
    //   native reference byte endianess
    inline static uint8_t native(void);

    enum : uint8_t
    {
      // Big byte - reference byte endianess
      big_byte = 0x76u,
      // Big word - reference byte endianess
      big_word = 0x54u,
      // Little word - reference byte endianess
      little_word = 0x32u,
      // Little byte - reference byte endianess
      little_byte = 0x10u
    };
  };

  // Method for inplace reversing bytes of 16-bit unsigned integer
  //
  // Parameters:
  //   data - 16-bit unsigned integer
  inline void byte_swap(uint16_t& data) noexcept;

  // Method for reversing bytes of 32-bit unsigned integer
  //
  // Parameters:
  //   data - 32-bit unsigned integer
  inline void byte_swap(uint32_t& data) noexcept;

  // Method for reversing bytes of 64-bit unsigned integer
  //
  // Parameters:
  //   data - 64-bit unsigned integer
  inline void byte_swap(uint64_t& data) noexcept;

  // Method for inplace reversing bytes of given contiguous memory block
  //
  // Parameters:
  //   start - Pointer to the first byte of contiguous memory block
  //     end - Pointer to byte after last byte of contiguous memory block
  inline void byte_swap(uint8_t* start, uint8_t* end) noexcept;
}

// IMPLEMENTATION

uint8_t flib::endianess::native(void)
{
  static uint8_t result = 0;
  if (0 == result)
  {
    uint32_t value = 0x76543210ul;
    result = reinterpret_cast<decltype(result)*>(&value)[0];
  }
  return result;
}

void flib::byte_swap(uint16_t& data) noexcept
{
#if   defined(_MSC_VER)
  data = _byteswap_ushort(data);
#elif defined(__GNUC__)
  data = __builtin_bswap16(data);
#else
  data = data >> 8 | data << 8;
#endif
}

void flib::byte_swap(uint32_t& data) noexcept
{
#if   defined(_MSC_VER)
  data = _byteswap_ulong(data);
#elif defined(__GNUC__)
  data = __builtin_bswap32(data);
#else
  data = data >> 8 & 0x00ff00fful | data << 8 & 0xff00ff00ul;
  data = data >> 16 | data << 16;
#endif
}

void flib::byte_swap(uint64_t& data) noexcept
{
#if   defined(_MSC_VER)
  data = _byteswap_uint64(data);
#elif defined(__GNUC__)
  data = __builtin_bswap64(data);
#else
  data = data >> 8 & 0x00ff00ff00ff00ffull | data << 8 & 0xff00ff00ff00ff00ull;
  data = data >> 16 & 0x0000ffff0000ffffull | data << 16 & 0xffff0000ffff0000ull;
  data = data >> 32 | data << 32;
#endif
}

void flib::byte_swap(uint8_t* start, uint8_t* end) noexcept
{
  uint8_t temp;
  for (--end; start < end; ++start, --end)
  {
    temp = *start;
    *start = *end;
    *end = temp;
  }
}