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
  struct Endianess
  {
  public:
    // Method for retrieving native platform byte endianess
    //
    // Returns:
    //   native reference byte endianess
    inline static uint8_t Native(void);

    enum : uint8_t
    {
      // Big byte - reference byte endianess
      BigByte = 0x76u,
      // Big word - reference byte endianess
      BigWord = 0x54u,
      // Little word - reference byte endianess
      LittleWord = 0x32u,
      // Little byte - reference byte endianess
      LittleByte = 0x10u
    };
  };

  // Method for inplace reversing bytes of 16-bit unsigned integer
  //
  // Parameters:
  //   data - 16-bit unsigned integer
  inline void ByteSwap(uint16_t& data) noexcept;

  // Method for reversing bytes of 32-bit unsigned integer
  //
  // Parameters:
  //   data - 32-bit unsigned integer
  inline void ByteSwap(uint32_t& data) noexcept;

  // Method for reversing bytes of 64-bit unsigned integer
  //
  // Parameters:
  //   data - 64-bit unsigned integer
  inline void ByteSwap(uint64_t& data) noexcept;

  // Method for inplace reversing bytes of given byte array
  //
  // Notes:
  //   - Method assumes that given length and offset are correct, because of this it does not check for memory bounds
  //   - Length does not include offset
  //
  // Parameters:
  //     data - Pointer to byte array
  //   length - Number of bytes to reverse, default is 0
  //   offset - Number of bytes (from the beggining of the byte array) to skip before reversing, default is 0
  inline void ByteSwap(uint8_t* data, const std::size_t length = 0, const std::size_t offset = 0) noexcept;
}

// IMPLEMENTATION

uint8_t flib::Endianess::Native(void)
{
  static uint8_t result = 0;
  if (0 == result)
  {
    uint32_t value = 0x76543210ul;
    result = reinterpret_cast<decltype(result)*>(&value)[0];
  }
  return result;
}

void flib::ByteSwap(uint16_t& data) noexcept
{
#if   defined(_MSC_VER)
  data = _byteswap_ushort(data);
#elif defined(__GNUC__)
  data = __builtin_bswap16(data);
#else
  data = data >> 8 | data << 8;
#endif
}

void flib::ByteSwap(uint32_t& data) noexcept
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

void flib::ByteSwap(uint64_t& data) noexcept
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

void flib::ByteSwap(uint8_t* data, const std::size_t length, const std::size_t offset) noexcept
{
  if (!data)
  {
    return;
  }
  data += offset;
  uint8_t temp;
  for (std::size_t i = 0, m = length / 2; i < m; ++i)
  {
    temp = data[i];
    data[i] = data[length - i - 1];
    data[length - i - 1] = temp;
  }
}