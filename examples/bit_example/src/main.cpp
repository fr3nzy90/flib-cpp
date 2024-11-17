// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include <flib/bit.hpp>
#include <flib/mld.hpp>

namespace
{
#pragma region Helpers
  const std::string g_hex_prefix = "";
  const std::string g_hex_string_deliminator = " ";

  std::string to_string(flib::endian::reference p_value)
  {
    switch (p_value)
    {
    case flib::endian::big_byte:
      return "big_byte";
    case flib::endian::big_word:
      return "big_word";
    case flib::endian::little_word:
      return "little_word";
    case flib::endian::little_byte:
      return "little_byte";
    default:
      return "unknown";
    }
  }

  std::string to_hex(uint8_t p_byte)
  {
    static constexpr char map[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    return std::string() + map[(p_byte & 0xf0) >> 4] + map[(p_byte & 0x0f)];
  }

  std::string to_hex_byte_string(const uint8_t* p_data_start, const uint8_t* p_data_end)
  {
    std::string result;
    for (auto it = p_data_start; it != p_data_end; ++it)
    {
      if (!result.empty())
      {
        result += g_hex_string_deliminator;
      }
      result += g_hex_prefix + ::to_hex(*it);
    }
    return result;
  }

  template<class T>
  std::string to_hex_byte_string(T p_value)
  {
    const uint8_t* ref = reinterpret_cast<uint8_t*>(&p_value);
    return ::to_hex_byte_string(ref, ref + sizeof(T));
  }

  void print_swapped_bytes(const uint8_t* p_original_start, const uint8_t* p_original_end, const uint8_t* p_reversed_start,
    const uint8_t* p_reversed_end)
  {
    std::cout << "Original " << (p_original_end - p_original_start) << "-byte array: " <<
      ::to_hex_byte_string(p_original_start, p_original_end) << '\n';
    std::cout << "Reversed " << (p_reversed_end - p_reversed_start) << "-byte array: " <<
      ::to_hex_byte_string(p_reversed_start, p_reversed_end) << '\n';
  }

  template<class T>
  void print_swapped_values(T p_original_value, T p_reversed_value)
  {
    std::cout << "Original " << sizeof(T) << "-byte value: " << ::to_hex_byte_string(p_original_value) << '\n';
    std::cout << "Reversed " << sizeof(T) << "-byte value: " << ::to_hex_byte_string(p_reversed_value) << '\n';
  }
#pragma endregion

#pragma region Examples
  void example_endianess(void)
  {
    // print native endianess
    std::cout << "Native endianess: " << ::to_string(flib::endian::native()) << '\n';
  }

  void example_2byte_swap(void)
  {
    // prepare data
    const uint16_t original = 0x0011u;
    uint16_t reversed = original;

    // reverse value bytes
    flib::byteswap(reversed);

    // print results
    ::print_swapped_values(original, reversed);
  }

  void example_4byte_swap(void)
  {
    // prepare data
    const uint32_t original = 0x00112233ul;
    uint32_t reversed = original;

    // reverse value bytes
    flib::byteswap(reversed);

    // print results
    ::print_swapped_values(original, reversed);
  }

  void example_8byte_swap(void)
  {
    // prepare data
    const uint64_t original = 0x0011223344556677ull;
    uint64_t reversed = original;

    // reverse value bytes
    flib::byteswap(reversed);

    // print results
    ::print_swapped_values(original, reversed);
  }

  void example_multibyte_swap(void)
  {
    // prepare data
    const uint8_t bytes[] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef };
    uint8_t reversed[sizeof(bytes)];
    std::copy(&bytes[0], &bytes[sizeof(bytes)], &reversed[0]);

    // reverse bytes
    flib::byteswap(reversed, reversed + sizeof(reversed));

    // print results
    ::print_swapped_bytes(bytes, bytes + sizeof(bytes), reversed, reversed + sizeof(reversed));
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_endianess();
  ::example_2byte_swap();
  ::example_4byte_swap();
  ::example_8byte_swap();
  ::example_multibyte_swap();

  return 0;
}