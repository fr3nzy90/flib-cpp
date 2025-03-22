// Copyright © 2021-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace flib
{
#pragma region API
  class timestamp
  {
    // RFC 3339 compliant timestamps with precision up to microseconds if supported

  public:
    using time_point_t = std::chrono::system_clock::time_point;

    enum class precision
    {
      seconds,
      milliseconds,
      microseconds,
      min = seconds,
      max = microseconds
    };

  public:
    static timestamp epoch(void);
    static timestamp parse(std::string p_timestamp);

  public:
    explicit timestamp(time_point_t p_timepoint = time_point_t::clock::now());
    time_point_t get(void) const;
    void set(time_point_t p_timepoint = time_point_t::clock::now());
    std::string to_string(bool p_utc = true, precision p_precision = precision::min) const;

  private:
    static void _insert_subseconds(std::string p_subseconds, std::string& p_timestamp);
    static std::string _extract_microseconds(std::string& p_timestamp);
    static std::string _get_timestamp(bool p_utc, const time_point_t& p_timepoint);
    static std::string _get_timestamp_local(const time_point_t& p_timepoint);
    static std::string _get_timestamp_utc(const time_point_t& p_timepoint);
    static std::string _get_subseconds(const time_point_t& p_timepoint, precision p_precision);
    static std::string _get_string(const std::chrono::milliseconds& p_duration);
    static std::string _get_string(const std::chrono::microseconds& p_duration);
    static time_point_t _get_timepoint(const std::string& p_timestamp);
    static time_point_t _get_timepoint_local(const std::string& p_timestamp);
    static time_point_t _get_timepoint_utc(const std::string& p_timestamp);
    static std::chrono::minutes _get_timezone_offset(const std::string& p_timestamp);
    static std::chrono::microseconds _get_microseconds(const std::string& p_duration);

  private:
    time_point_t m_timepoint;
  };
#pragma endregion

#pragma region IMPLEMENTATION
  inline timestamp timestamp::epoch(void)
  {
    return timestamp(time_point_t());
  }

  inline timestamp timestamp::parse(std::string p_timestamp)
  {
    std::string microseconds_digits = _extract_microseconds(p_timestamp);
    time_point_t result = _get_timepoint(p_timestamp);
    result += _get_microseconds(microseconds_digits);
    return timestamp(result);
  }

  inline timestamp::timestamp(time_point_t p_timepoint)
    : m_timepoint{ std::move(p_timepoint) }
  {
  }

  inline timestamp::time_point_t timestamp::get(void) const
  {
    return m_timepoint;
  }

  inline void timestamp::set(time_point_t p_timepoint)
  {
    m_timepoint = std::move(p_timepoint);
  }

  inline std::string timestamp::to_string(bool p_utc, precision p_precision) const
  {
    std::string result = _get_timestamp(p_utc, m_timepoint);
    _insert_subseconds(_get_subseconds(m_timepoint, p_precision), result);
    return result;
  }

  inline void timestamp::_insert_subseconds(std::string p_subseconds, std::string& p_timestamp)
  {
    if (p_subseconds.empty())
    {
      return;
    }
    p_subseconds = p_subseconds.substr(0, p_subseconds.find_last_not_of("0") + 1);
    if (p_subseconds.empty())
    {
      return;
    }
    p_timestamp.insert(19, '.' + p_subseconds);
  }

  inline std::string timestamp::_extract_microseconds(std::string& p_timestamp)
  {
    std::string::size_type index;
    std::string decimals;
    if ('.' == p_timestamp[19] || ',' == p_timestamp[19])
    {
      index = p_timestamp.find_first_of("Z+-", 19);
      decimals = p_timestamp.substr(20, (index < 26 ? index : 26) - 20);
      p_timestamp.erase(19, index - 19);
    }
    if (decimals.empty())
    {
      return "";
    }
    decimals += std::string(6 - decimals.size(), '0');
    index = decimals.find_first_not_of("0");
    if (std::string::npos == index)
    {
      return "";
    }
    return decimals.substr(index);
  }

  inline std::string timestamp::_get_timestamp(bool p_utc, const time_point_t& p_timepoint)
  {
    return p_utc ? _get_timestamp_utc(p_timepoint) : _get_timestamp_local(p_timepoint);
  }

  inline std::string timestamp::_get_timestamp_local(const time_point_t& p_timepoint)
  {
    char buffer[26];
    std::time_t time = time_point_t::clock::to_time_t(p_timepoint);
    std::tm time_components{};
#if defined(_WIN32) && !defined(__CYGWIN__)
    if (0 != localtime_s(&time_components, &time))
    {
      throw std::runtime_error("std::time_t to std::tm conversion error");
    }
#else
    if (0 == localtime_r(&time, &time_components))
    {
      throw std::runtime_error("std::time_t to std::tm conversion error");
    }
#endif
    strftime(buffer, sizeof(buffer), "%FT%T%z", &time_components);
#if defined(_WIN32) && !defined(__CYGWIN__)
    memmove_s(buffer + sizeof(buffer) - 3, 3, buffer + sizeof(buffer) - 4, 3);
#else
    memmove(buffer + sizeof(buffer) - 3, buffer + sizeof(buffer) - 4, 3);
#endif
    buffer[sizeof(buffer) - 4] = ':';
    return buffer;
  }

  inline std::string timestamp::_get_timestamp_utc(const time_point_t& p_timepoint)
  {
    char buffer[21];
    std::time_t time = time_point_t::clock::to_time_t(p_timepoint);
    std::tm time_components{};
#if defined(_WIN32) && !defined(__CYGWIN__)
    if (0 != gmtime_s(&time_components, &time))
    {
      throw std::runtime_error("std::time_t to std::tm conversion error");
    }
#else
    if (0 == gmtime_r(&time, &time_components))
    {
      throw std::runtime_error("std::time_t to std::tm conversion error");
    }
#endif
    strftime(buffer, sizeof(buffer), "%FT%TZ", &time_components);
    return buffer;
  }

  inline std::string timestamp::_get_subseconds(const time_point_t& p_timepoint, precision p_precision)
  {
    switch (p_precision)
    {
    default:
      return "";
    case precision::milliseconds:
      return _get_string(std::chrono::duration_cast<std::chrono::milliseconds>(p_timepoint.time_since_epoch()));
    case precision::microseconds:
      return _get_string(std::chrono::duration_cast<std::chrono::microseconds>(p_timepoint.time_since_epoch()));
    }
  }

  inline std::string timestamp::_get_string(const std::chrono::milliseconds& p_duration)
  {
    char buffer[4];
    snprintf(buffer, sizeof(buffer), "%03u", static_cast<unsigned>(p_duration.count() % 1000));
    return buffer;
  }

  inline std::string timestamp::_get_string(const std::chrono::microseconds& p_duration)
  {
    char buffer[7];
    snprintf(buffer, sizeof(buffer), "%06u", static_cast<unsigned>(p_duration.count() % 1000000));
    return buffer;
  }

  inline timestamp::time_point_t timestamp::_get_timepoint(const std::string& p_timestamp)
  {
    return 'Z' == p_timestamp[19] ? _get_timepoint_utc(p_timestamp) : _get_timepoint_local(p_timestamp);
  }

  inline timestamp::time_point_t timestamp::_get_timepoint_local(const std::string& p_timestamp)
  {
    time_point_t timepoint = _get_timepoint_utc(p_timestamp);
    std::chrono::minutes offset = _get_timezone_offset(p_timestamp.substr(19));
    timepoint -= offset;
    return timepoint;
  }

  inline timestamp::time_point_t timestamp::_get_timepoint_utc(const std::string& p_timestamp)
  {
    std::tm time_components{};
    std::istringstream stream(p_timestamp);
    stream >> std::get_time(&time_components, "%Y-%m-%dT%T");
    time_components.tm_isdst = -1;
    std::time_t time;
#if defined(_WIN32) && !defined(__CYGWIN__)
    time = _mkgmtime(&time_components);
#else
    time = timegm(&time_components);
#endif
    return time_point_t::clock::from_time_t(time);
  }

  inline std::chrono::minutes timestamp::_get_timezone_offset(const std::string& p_offset)
  {
    std::tm time_components{};
    char prefix;
    std::istringstream stream(p_offset);
    stream >> prefix >> std::get_time(&time_components, "%R");
    std::chrono::minutes offset = std::chrono::hours(time_components.tm_hour) + std::chrono::minutes(time_components.tm_min);
    return '+' == prefix ? offset : -offset;
  }

  inline std::chrono::microseconds timestamp::_get_microseconds(const std::string& p_duration)
  {
    return std::chrono::microseconds(p_duration.empty() ? 0 : std::stoul(p_duration));
  }
#pragma endregion
}