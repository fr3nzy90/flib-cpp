// Copyright © 2021-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cstdint>
#include <random>
#include <string>

namespace flib
{
  inline namespace uuid
  {
    enum class version_t
    {
      v4
    };

    std::string generate(version_t version = version_t::v4);

    bool test(const std::string& uuid, version_t version = version_t::v4);

    // IMPLEMENTATION

    inline std::string generate(version_t /*version*/)
    {
      static constexpr char hex[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
      static std::mt19937_64 generator([]()
        {
          return std::random_device()();
        }());
      static std::uniform_int_distribution<uint64_t> distribution;
      uint64_t data[]{ distribution(generator),distribution(generator) };
      std::string uuid("xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx");
      uint8_t i{ 0u };
      for (decltype(uuid)::value_type& symbol : uuid)
      {
        if ('x' == symbol)
        {
          symbol = hex[(data[i / 16] >> 4 * (i % 16)) % 16];
          ++i;
        }
        else if ('y' == symbol)
        {
          symbol = hex[(data[i / 16] >> 4 * (i % 16)) % 4 + 8];
          ++i;
        }
      }
      return uuid;
    }

    inline bool test(const std::string& uuid, version_t /*version*/)
    {                                             
      static const std::string reference("xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx");
      if (uuid.size() != reference.size())
      {
        return false;
      }
      for (decltype(reference)::size_type i = 0; i < reference.size(); ++i)
      {
        if ('x' == reference[i])
        {
          if ((uuid[i] < '0' || '9' < uuid[i]) && (uuid[i] < 'a' || 'f' < uuid[i]) && (uuid[i] < 'A' || 'F' < uuid[i]))
          {
            return false;
          }
        }
        else if ('y' == reference[i])
        {
          if ('8' != uuid[i] && '9' != uuid[i] && 'a' != uuid[i] && 'b' != uuid[i] && 'A' != uuid[i] && 'B' != uuid[i])
          {
            return false;
          }
        }
        else
        {
          if (reference[i] != uuid[i])
          {
            return false;
          }
        }
      }
      return true;
    }
  }
}