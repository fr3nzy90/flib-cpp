// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cstdint>

#if defined(_WIN32)
#  include <cstdlib>
#endif

namespace flib
{
#pragma region API
  // Class for determination of platform byte endianess
  class endian
  {
  public:
    enum reference
      : uint8_t
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

  public:
    // Method for retrieving native platform byte endianess
    //
    // Returns:
    //   native reference byte endianess
    static reference native();
  };

  // Method for inplace reversing bytes of 16-bit unsigned integer
  //
  // Parameters:
  //   p_data - 16-bit unsigned integer
  void byteswap(uint16_t& p_data) noexcept;

  // Method for reversing bytes of 32-bit unsigned integer
  //
  // Parameters:
  //   p_data - 32-bit unsigned integer
  void byteswap(uint32_t& p_data) noexcept;

  // Method for reversing bytes of 64-bit unsigned integer
  //
  // Parameters:
  //   p_data - 64-bit unsigned integer
  void byteswap(uint64_t& p_data) noexcept;

  // Method for inplace reversing bytes of given contiguous memory block
  //
  // Parameters:
  //   p_data_start - Pointer to the first byte of contiguous memory block
  //     p_data_end - Pointer to byte after last byte of contiguous memory block
  void byteswap(uint8_t* p_data_start, uint8_t* p_data_end) noexcept;
#pragma endregion

#pragma region IMPLEMENTATION
  inline endian::reference endian::native(void)
  {
    static const endian::reference result = []()
      {
        uint32_t temp = 0x76543210ul;
        return static_cast<endian::reference>(reinterpret_cast<uint8_t*>(&temp)[0]);
      }();
    return result;
  }

  inline void byteswap(uint16_t& p_data) noexcept
  {
#if   defined(_MSC_VER)
    p_data = _byteswap_ushort(p_data);
#elif defined(__GNUC__)
    p_data = __builtin_bswap16(p_data);
#else
    p_data = p_data >> 8 | p_data << 8;
#endif
  }

  inline void byteswap(uint32_t& p_data) noexcept
  {
#if   defined(_MSC_VER)
    p_data = _byteswap_ulong(p_data);
#elif defined(__GNUC__)
    p_data = __builtin_bswap32(p_data);
#else
    p_data = p_data >> 8 & 0x00ff00fful | p_data << 8 & 0xff00ff00ul;
    p_data = p_data >> 16 | p_data << 16;
#endif
  }

  inline void byteswap(uint64_t& p_data) noexcept
  {
#if   defined(_MSC_VER)
    p_data = _byteswap_uint64(p_data);
#elif defined(__GNUC__)
    p_data = __builtin_bswap64(p_data);
#else
    p_data = p_data >> 8 & 0x00ff00ff00ff00ffull | p_data << 8 & 0xff00ff00ff00ff00ull;
    p_data = p_data >> 16 & 0x0000ffff0000ffffull | p_data << 16 & 0xffff0000ffff0000ull;
    p_data = p_data >> 32 | p_data << 32;
#endif
  }

  inline void byteswap(uint8_t* p_data_start, uint8_t* p_data_end) noexcept
  {
    uint8_t temp;
    for (--p_data_end; p_data_start < p_data_end; ++p_data_start, --p_data_end)
    {
      temp = *p_data_start;
      *p_data_start = *p_data_end;
      *p_data_end = temp;
    }
  }
#pragma endregion
}