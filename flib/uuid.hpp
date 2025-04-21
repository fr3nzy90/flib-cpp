// Copyright © 2021-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <bitset>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace flib
{
#pragma region API
  class uuid
  {
    // RFC 4122 compliant UUID with support for generation of nil, max, v4 and v7 UUIDs

  public:
    using data_t = std::bitset<128>;
    using time_point_t = std::chrono::system_clock::time_point;

  public:
    static uuid generate_nil(void);
    static uuid generate_max(void);
    static uuid generate_v4(void);
    static uuid generate_v7(time_point_t p_timepoint = time_point_t::clock::now());
    static uuid parse(std::string p_uuid);

  public:
    explicit uuid(data_t p_data = {});
    bool operator==(const uuid& p_obj) const;
    bool operator!=(const uuid& p_obj) const;
    data_t get_data(void) const;
    uint8_t get_version(void) const;
    uint8_t get_variant(void) const;
    void set_data(data_t p_data);
    std::string to_string(bool p_uppercase = false) const;
    bool valid(void) const;

  private:
    static void _fill_random(data_t& p_data, data_t& p_set_bits, std::size_t p_start = 0);
    static void _set(data_t& p_data, data_t& p_set_bits, std::size_t p_index, bool p_value);
    static void _set_version(data_t& p_data, data_t& p_set_bits, uint8_t p_version);
    static void _set_variant(data_t& p_data, data_t& p_set_bits, uint8_t p_variant);

  private:
    data_t m_data;
  };

  std::string to_string(const uuid& p_obj);
#pragma endregion

#pragma region IMPLEMENTATION
  inline uuid uuid::generate_nil(void)
  {
    return uuid();
  }

  inline uuid uuid::generate_max(void)
  {
    return uuid(std::move(data_t{}.set()));
  }

  inline uuid uuid::generate_v4(void)
  {
    data_t data;
    data_t set_bits;
    _set_version(data, set_bits, 4u);
    _set_variant(data, set_bits, 2u);
    _fill_random(data, set_bits);
    return uuid(std::move(data));
  }

  inline uuid uuid::generate_v7(time_point_t p_timepoint)
  {
    data_t data;
    data_t set_bits;
    _set_version(data, set_bits, 7u);
    _set_variant(data, set_bits, 2u);
    std::bitset<48> timestamp_data(
      static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::milliseconds>(p_timepoint.time_since_epoch()).count()));
    for (std::size_t i = 0; i < 48; ++i)
    {
      _set(data, set_bits, i, timestamp_data[47 - i]);
    }
    _fill_random(data, set_bits, 48);
    return uuid(std::move(data));
  }

  inline uuid uuid::parse(std::string p_uuid)
  {
    static constexpr char mask[] = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
    static const auto get_digit = [](std::string::value_type symbol)
      {
        switch (symbol)
        {
        case '0':
          return 0u;
        case '1':
          return 1u;
        case '2':
          return 2u;
        case '3':
          return 3u;
        case '4':
          return 4u;
        case '5':
          return 5u;
        case '6':
          return 6u;
        case '7':
          return 7u;
        case '8':
          return 8u;
        case '9':
          return 9u;
        case 'a':
        case 'A':
          return 10u;
        case 'b':
        case 'B':
          return 11u;
        case 'c':
        case 'C':
          return 12u;
        case 'd':
        case 'D':
          return 13u;
        case 'e':
        case 'E':
          return 14u;
        case 'f':
        case 'F':
          return 15u;
        default:
          throw std::runtime_error("Invalid UUID format");
        }
      };
    if (36 != p_uuid.size())
    {
      throw std::runtime_error("Invalid UUID format");
    }
    data_t data;
    for (std::size_t i = 0, j = 0; i < 36; ++i)
    {
      if ('x' == mask[i])
      {
        std::bitset<4> bits = get_digit(p_uuid[i]);
        data[j] = bits[3];
        data[j + 1] = bits[2];
        data[j + 2] = bits[1];
        data[j + 3] = bits[0];
        j += 4;
      }
      else if ('-' != p_uuid[i])
      {
        throw std::runtime_error("Invalid UUID format");
      }
    }
    return uuid(data);
  }

  inline uuid::uuid(data_t p_data)
    : m_data{ std::move(p_data) }
  {
  }

  inline bool uuid::operator==(const uuid& p_obj) const
  {
    return m_data == p_obj.m_data;
  }

  inline bool uuid::operator!=(const uuid& p_obj) const
  {
    return !(*this == p_obj);
  }

  inline uuid::data_t uuid::get_data(void) const
  {
    return m_data;
  }

  inline uint8_t uuid::get_version(void) const
  {
    return static_cast<uint8_t>(m_data[48] * 8u + m_data[49] * 4u + m_data[50] * 2u + m_data[51]);
  }

  inline uint8_t uuid::get_variant(void) const
  {
    return static_cast<uint8_t>(m_data[64] ? (m_data[65] ? (m_data[66] ? 7u : 6u) : 2u) : 0u);
  }

  inline void uuid::set_data(data_t p_data)
  {
    m_data = std::move(p_data);
  }

  inline std::string uuid::to_string(bool p_uppercase) const
  {
    static constexpr char mapping_lower[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
    static constexpr char mapping_upper[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
    const char* mapping = p_uppercase ? mapping_upper : mapping_lower;
    std::string result("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    result.reserve(result.size() + 4);
    for (std::size_t i = 0, j = 0; i < 32; ++i, j += 4)
    {
      result[i] = mapping[m_data[j] * 8 + m_data[j + 1] * 4 + m_data[j + 2] * 2 + m_data[j + 3]];
    }
    result
      .insert(20, "-", 1)
      .insert(16, "-", 1)
      .insert(12, "-", 1)
      .insert(8, "-", 1);
    return result;
  }

  inline bool uuid::valid(void) const
  {
    uint8_t version = get_version();
    switch (version)
    {
    case 0:
      return m_data.none();
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
      return 2 == get_variant();
    case 15:
      return m_data.all();
    default:
      return false;
    }
  }

  inline void uuid::_fill_random(data_t& p_data, data_t& p_set_bits, std::size_t p_start)
  {
    static std::mt19937_64 generator{ std::random_device{}() };
    static std::uniform_int_distribution<uint64_t> distribution;
    std::bitset<64> random_data[] = { distribution(generator), distribution(generator) };
    for (std::size_t i = p_start; i < 128; ++i)
    {
      if (p_set_bits[i])
      {
        continue;
      }
      p_data[i] = random_data[i / 64][i % 64];
      p_set_bits[i] = true;
    }
  }

  inline void uuid::_set(data_t& p_data, data_t& p_set_bits, std::size_t p_index, bool p_value)
  {
    assert((void("UUID bit already set"), !p_set_bits[p_index]));
    p_data[p_index] = p_value;
    p_set_bits[p_index] = true;
  }

  inline void uuid::_set_version(data_t& p_data, data_t& p_set_bits, uint8_t p_version)
  {
    std::bitset<4> version_data(p_version);
    _set(p_data, p_set_bits, 48, version_data[3]);
    _set(p_data, p_set_bits, 49, version_data[2]);
    _set(p_data, p_set_bits, 50, version_data[1]);
    _set(p_data, p_set_bits, 51, version_data[0]);
  }

  inline void uuid::_set_variant(data_t& p_data, data_t& p_set_bits, uint8_t p_variant)
  {
    switch (p_variant)
    {
    case 0u:
      _set(p_data, p_set_bits, 64, false);
      return;
    case 2u:
      _set(p_data, p_set_bits, 64, true);
      _set(p_data, p_set_bits, 65, false);
      return;
    case 6u:
      _set(p_data, p_set_bits, 64, true);
      _set(p_data, p_set_bits, 65, true);
      _set(p_data, p_set_bits, 66, false);
      return;
    case 7u:
      _set(p_data, p_set_bits, 64, true);
      _set(p_data, p_set_bits, 65, true);
      _set(p_data, p_set_bits, 66, true);
      return;
    default:
      throw std::runtime_error("Unsupported UUID variant");
    }
  }

  inline std::string to_string(const uuid& p_obj)
  {
    return p_obj.to_string();
  }
#pragma endregion
}