// Copyright © 2021-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace flib
{
  // RFC 3339 compliant timestamps with microsecond precision (if available)
  inline namespace timestamp
  {
    std::string generate(const std::chrono::system_clock::time_point& timepoint = std::chrono::system_clock::now(),
      bool utc = true);

    std::chrono::system_clock::time_point parse(const std::string& timestamp);

    // IMPLEMENTATION

    namespace _impl
    {
      inline std::tm _time_t_tm_utc(std::time_t time)
      {
        std::tm result{};
#if defined(_WIN32) && !defined(__CYGWIN__)
        if (0 == gmtime_s(&result, &time)) return result;
#else
        if (0 != gmtime_r(&time, &result)) return result;
        result.tm_isdst = -1;
        (void)mktime(&result);
#endif
        throw std::runtime_error("Time to tm UTC conversion error");
      }

      inline std::tm _time_t_tm_local(std::time_t time)
      {
        std::tm result{};
#if defined(_WIN32) && !defined(__CYGWIN__)
        if (0 == localtime_s(&result, &time)) return result;
#else
        if (0 != localtime_r(&time, &result)) return result;
#endif
        throw std::runtime_error("Time to tm local conversion error");
      }

      inline std::time_t _tm_time_t_utc(std::tm time)
      {
#if defined(_WIN32) && !defined(__CYGWIN__)
        return _mkgmtime(&time);
#else
        return timegm(&time);
#endif
      }
    }

    inline std::string generate(const std::chrono::system_clock::time_point& timepoint, bool utc)
    {
      auto reference = std::chrono::system_clock::to_time_t(timepoint);
      auto time_struct = utc ? _impl::_time_t_tm_utc(reference) : _impl::_time_t_tm_local(reference);
      std::ostringstream stream;
      stream << std::put_time(&time_struct, "%FT%T");
      {
        auto decimal = std::to_string(
          std::chrono::duration_cast<std::chrono::microseconds>(timepoint.time_since_epoch()).count() % 1000000);
        decimal = std::string(6 - decimal.size(), '0') + decimal;
        auto pos = decimal.find_last_not_of('0');
        decimal = std::string::npos != pos ? '.' + decimal.substr(0, pos + 1) : "";
        stream << decimal;
      }
      if (utc)
      {
        stream << 'Z';
        return stream.str();
      }
      auto tz_diff = std::chrono::duration_cast<std::chrono::minutes>(
        std::chrono::system_clock::from_time_t(_impl::_tm_time_t_utc(time_struct) - reference).time_since_epoch());
      auto tz_prefix = std::chrono::minutes::zero() > tz_diff ? '-' : '+';
      auto tz_hour = std::abs(std::chrono::duration_cast<std::chrono::hours>(tz_diff).count());
      auto tz_min = std::abs(tz_diff.count()) - tz_hour * 60;
      stream << tz_prefix << std::setw(2) << std::setfill('0') << tz_hour << ':' << std::setw(2) << std::setfill('0')
        << tz_min;
      return stream.str();
    }

    inline std::chrono::system_clock::time_point parse(const std::string& timestamp)
    {
      static const auto get_separator_type = [](int ch) -> int
      {
        switch (ch)
        {
        case '.':
          return 1; // decimal;
        case 'Z':
          return 2; // tz_zulu;
        case '+':
          return 3; // tz_positive;
        case '-':
          return 4; // tz_negative;
        default:
          return 0; // other;
        }
      };
      std::tm temp{};
      std::istringstream stream(timestamp);
      stream >> std::get_time(&temp, "%Y-%m-%dT%T");
      temp.tm_isdst = -1;
      auto timepoint = std::chrono::system_clock::from_time_t(_impl::_tm_time_t_utc(temp));
      if (1 == get_separator_type(stream.peek()))
      {
        stream.ignore();
        std::string decimal;
        while (0 == get_separator_type(stream.peek()) && decimal.size() < 6)
        {
          decimal += static_cast<std::string::value_type>(stream.get());
        }
        timepoint += std::chrono::microseconds(std::stoul(decimal + std::string(6 - decimal.size(), '0')));
        while (0 == get_separator_type(stream.peek()))
        {
          stream.ignore();
        }
      }
      if (2 == get_separator_type(stream.peek()))
      {
        return timepoint;
      }
      auto invert = 3 == get_separator_type(stream.get());
      temp = {};
      stream >> std::get_time(&temp, "%R");
      auto offset = std::chrono::hours(temp.tm_hour) + std::chrono::minutes(temp.tm_min);
      timepoint += invert ? -offset : offset;
      return timepoint;
    }
  }
}