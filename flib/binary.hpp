// Copyright � 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cstdint>

#if defined(_WIN32)
#  include <cstdlib>
#endif

namespace flib
{
  inline namespace binary
  {
    // Structure for determination of platform byte endianess
    struct endianess
    {
    public:
      // Method for retrieving native platform byte endianess
      //
      // Returns:
      //   native reference byte endianess
      static uint8_t native(void);

      enum
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
    };

    // Method for inplace reversing bytes of 16-bit unsigned integer
    //
    // Parameters:
    //   data - 16-bit unsigned integer
    void byte_swap(uint16_t& data) noexcept;

    // Method for reversing bytes of 32-bit unsigned integer
    //
    // Parameters:
    //   data - 32-bit unsigned integer
    void byte_swap(uint32_t& data) noexcept;

    // Method for reversing bytes of 64-bit unsigned integer
    //
    // Parameters:
    //   data - 64-bit unsigned integer
    void byte_swap(uint64_t& data) noexcept;

    // Method for inplace reversing bytes of given contiguous memory block
    //
    // Parameters:
    //   start - Pointer to the first byte of contiguous memory block
    //     end - Pointer to byte after last byte of contiguous memory block
    void byte_swap(uint8_t* start, uint8_t* end) noexcept;

    // IMPLEMENTATION

    inline uint8_t endianess::native(void)
    {
      static uint8_t result = 0;
      if (0 == result)
      {
        uint32_t value = 0x76543210ul;
        result = reinterpret_cast<uint8_t*>(&value)[0];
      }
      return result;
    }

    inline void byte_swap(uint16_t& data) noexcept
    {
#if   defined(_MSC_VER)
      data = _byteswap_ushort(data);
#elif defined(__GNUC__)
      data = __builtin_bswap16(data);
#else
      data = data >> 8 | data << 8;
#endif
    }

    inline void byte_swap(uint32_t& data) noexcept
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

    inline void byte_swap(uint64_t& data) noexcept
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

    inline void byte_swap(uint8_t* start, uint8_t* end) noexcept
    {
      uint8_t temp;
      for (--end; start < end; ++start, --end)
      {
        temp = *start;
        *start = *end;
        *end = temp;
      }
    }
  }
}