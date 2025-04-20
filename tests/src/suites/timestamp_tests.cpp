// Copyright © 2021-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/timestamp.hpp>

#include <cstdint>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>

#include <catch2/catch2.hpp>

namespace
{
  using microseconds = std::chrono::duration<uint64_t, std::micro>;
  using milliseconds = std::chrono::duration<uint64_t, std::milli>;

  inline uint64_t microseconds_since_epoch(flib::timestamp::time_point_t p_timepoint)
  {
    return std::chrono::duration_cast<microseconds>(p_timepoint.time_since_epoch()).count();
  }

  inline flib::timestamp::time_point_t make_timepoint(uint64_t p_us_since_epoch)
  {
    return flib::timestamp::time_point_t() + ::microseconds(p_us_since_epoch);
  }

  inline flib::timestamp::time_point_t make_timepoint_now()
  {
    return make_timepoint(microseconds_since_epoch(flib::timestamp::time_point_t::clock::now()));
  }

  inline std::string to_string(flib::timestamp::time_point_t p_timepoint)
  {
    return std::to_string(microseconds_since_epoch(p_timepoint));
  }

  inline std::string to_string(flib::timestamp::precision p_precision)
  {
    switch (p_precision)
    {
    case flib::timestamp::precision::seconds:
      return "seconds";
    case flib::timestamp::precision::milliseconds:
      return "milliseconds";
    case flib::timestamp::precision::microseconds:
      return "microseconds";
    default:
      return "unknown";
    }
  }
}

TEST_CASE("Timestamp tests - Sanity check", "[timestamp]")
{
  SECTION("Default construction")
  {
    flib::timestamp::time_point_t timepoint_before = flib::timestamp::time_point_t::clock::now();
    flib::timestamp timestamp;
    flib::timestamp::time_point_t timepoint_after = flib::timestamp::time_point_t::clock::now();
    REQUIRE((timepoint_before <= timestamp.get() && timestamp.get() <= timepoint_after));
  }
  SECTION("Epoch construction")
  {
    flib::timestamp timestamp = flib::timestamp::epoch();
    REQUIRE(flib::timestamp::time_point_t() == timestamp.get());
  }
  SECTION("Construction from custom timestamp")
  {
    flib::timestamp::time_point_t timepoint;
    timepoint += ::microseconds(946684799123456);
    flib::timestamp timestamp(timepoint);
    REQUIRE(timepoint == timestamp.get());
  }
  SECTION("Min precision check")
  {
    REQUIRE(flib::timestamp::precision::min == flib::timestamp::precision::seconds);
  }
  SECTION("Max precision check")
  {
    REQUIRE(flib::timestamp::precision::max == flib::timestamp::precision::microseconds);
  }
  SECTION("Equality check")
  {
    REQUIRE(flib::timestamp::epoch() == flib::timestamp::epoch());
    flib::timestamp::time_point_t timepoint = flib::timestamp::time_point_t::clock::now();
    REQUIRE(flib::timestamp(timepoint) == flib::timestamp(timepoint));
    REQUIRE_FALSE(flib::timestamp(timepoint) == flib::timestamp::epoch());
  }
}

TEST_CASE("Timestamp tests - Set value", "[timestamp]")
{
  flib::timestamp timestamp;
  flib::timestamp::time_point_t timepoint = flib::timestamp::time_point_t::clock::now();
  timestamp.set(timepoint);
  REQUIRE(timepoint == timestamp.get());
}

TEST_CASE("Timestamp tests - General formatting", "[timestamp]")
{
  static const auto check = [](const std::string& p_timestamp)
    {
      static const std::regex regex("^[0-9]{4}-[0-9]{2}-[0-9]{2}"
        "[Tt][0-9]{2}:[0-9]{2}:[0-9]{2}"
        "(\\.[0-9]+)?"
        "([Zz]|([+-][0-9]{2}:[0-9]{2}))$");
      return std::regex_match(p_timestamp, regex);
    };
  flib::timestamp::time_point_t timepoint = GENERATE(
    flib::timestamp::time_point_t(),
    ::make_timepoint(946684799123456),
    ::make_timepoint(946684800000000),
    ::make_timepoint(7258118399123000),
    ::make_timepoint_now());
  flib::timestamp timestamp(timepoint);
  INFO("Timepoint=" << ::to_string(timepoint));
  REQUIRE(check(timestamp.to_string()));
  REQUIRE(check(timestamp.to_string(true, flib::timestamp::precision::seconds)));
  REQUIRE(check(timestamp.to_string(true, flib::timestamp::precision::milliseconds)));
  REQUIRE(check(timestamp.to_string(true, flib::timestamp::precision::microseconds)));
  REQUIRE(check(timestamp.to_string(false, flib::timestamp::precision::seconds)));
  REQUIRE(check(timestamp.to_string(false, flib::timestamp::precision::milliseconds)));
  REQUIRE(check(timestamp.to_string(false, flib::timestamp::precision::microseconds)));
  REQUIRE(check(flib::to_string(timestamp)));
  REQUIRE(check(flib::to_string(timestamp, true, flib::timestamp::precision::seconds)));
  REQUIRE(check(flib::to_string(timestamp, true, flib::timestamp::precision::milliseconds)));
  REQUIRE(check(flib::to_string(timestamp, true, flib::timestamp::precision::microseconds)));
  REQUIRE(check(flib::to_string(timestamp, false, flib::timestamp::precision::seconds)));
  REQUIRE(check(flib::to_string(timestamp, false, flib::timestamp::precision::milliseconds)));
  REQUIRE(check(flib::to_string(timestamp, false, flib::timestamp::precision::microseconds)));
}

TEST_CASE("Timestamp tests - Precision formatting", "[timestamp]")
{
  static const auto get_offset = [](const std::string& p_timestamp)
    {
      std::tm temp{};
      std::istringstream stream(p_timestamp.substr(p_timestamp.size() - 5));
      stream >> std::get_time(&temp, "%R");
      std::chrono::minutes offset = std::chrono::hours(temp.tm_hour) + std::chrono::minutes(temp.tm_min);
      return '+' == p_timestamp.at(p_timestamp.size() - 6) ? offset : -offset;
    };
  flib::timestamp::time_point_t timepoint;
  flib::timestamp::precision precision;
  std::string utc_timestamp;
  std::tie(timepoint, precision, utc_timestamp) = GENERATE(table<flib::timestamp::time_point_t, flib::timestamp::precision, std::string>({
    // 1970-01-02T00:00:00Z
    { ::make_timepoint(86400000000),      flib::timestamp::precision::seconds,      "1970-01-02T00:00:00Z" },
    { ::make_timepoint(86400000000),      flib::timestamp::precision::milliseconds, "1970-01-02T00:00:00Z" },
    { ::make_timepoint(86400000000),      flib::timestamp::precision::microseconds, "1970-01-02T00:00:00Z" },
    // 1999-12-31T23:59:59.999999Z
    { ::make_timepoint(946684799999999),  flib::timestamp::precision::seconds,      "1999-12-31T23:59:59Z" },
    { ::make_timepoint(946684799999999),  flib::timestamp::precision::milliseconds, "1999-12-31T23:59:59.999Z" },
    { ::make_timepoint(946684799999999),  flib::timestamp::precision::microseconds, "1999-12-31T23:59:59.999999Z" },
    // 2000-01-01T00:00:00Z
    { ::make_timepoint(946684800000000),  flib::timestamp::precision::seconds,      "2000-01-01T00:00:00Z" },
    { ::make_timepoint(946684800000000),  flib::timestamp::precision::milliseconds, "2000-01-01T00:00:00Z" },
    { ::make_timepoint(946684800000000),  flib::timestamp::precision::microseconds, "2000-01-01T00:00:00Z" },
    // 2199-12-31T23:59:59.999999Z
    { ::make_timepoint(7258118399999999), flib::timestamp::precision::seconds,      "2199-12-31T23:59:59Z" },
    { ::make_timepoint(7258118399999999), flib::timestamp::precision::milliseconds, "2199-12-31T23:59:59.999Z" },
    { ::make_timepoint(7258118399999999), flib::timestamp::precision::microseconds, "2199-12-31T23:59:59.999999Z" }}));
  flib::timestamp timestamp(timepoint);
  INFO("    Timepoint=" << ::to_string(timepoint));
  INFO("    Precision=" << ::to_string(precision));
  INFO("UTC timestamp=" << utc_timestamp);
  REQUIRE(timestamp.to_string(true, precision) == utc_timestamp);
  REQUIRE(flib::to_string(timestamp, true, precision) == utc_timestamp);
  REQUIRE_THAT(flib::timestamp(timepoint - get_offset(timestamp.to_string(false))).to_string(false, precision),
    Catch::Matchers::StartsWith(utc_timestamp.substr(0, utc_timestamp.size() - 1)));
  REQUIRE_THAT(flib::to_string(flib::timestamp(timepoint - get_offset(flib::to_string(timestamp, false))), false, precision),
    Catch::Matchers::StartsWith(utc_timestamp.substr(0, utc_timestamp.size() - 1)));
}

TEST_CASE("Timestamp tests - Parsing check", "[timestamp]")
{
  flib::timestamp::time_point_t timepoint;
  std::string timestamp;
  std::tie(timepoint, timestamp) = GENERATE(table<flib::timestamp::time_point_t, std::string>({
    // precision edgecases
    { flib::timestamp::time_point_t(),   "1970-01-01T00:00:00.Z" },
    { flib::timestamp::time_point_t(),   "1970-01-01T00:00:00.0Z" },
    // 1970-01-02T00:00:00Z
    { flib::timestamp::time_point_t(),   "1969-12-31T12:00:00-12:00" },
    { flib::timestamp::time_point_t(),   "1969-12-31T23:00:00-01:00" },
    { flib::timestamp::time_point_t(),   "1969-12-31T23:30:00-00:30" },
    { flib::timestamp::time_point_t(),   "1970-01-01T00:00:00Z" },
    { flib::timestamp::time_point_t(),   "1970-01-01T00:30:00+00:30" },
    { flib::timestamp::time_point_t(),   "1970-01-01T01:00:00+01:00" },
    { flib::timestamp::time_point_t(),   "1970-01-01T12:00:00+12:00" },
    // 1999-12-31T23:59:59.999999Z
    { ::make_timepoint(946684799999999), "1999-12-31T11:59:59.999999-12:00" },
    { ::make_timepoint(946684799999999), "1999-12-31T22:59:59.999999-01:00" },
    { ::make_timepoint(946684799999999), "1999-12-31T23:29:59.999999-00:30" },
    { ::make_timepoint(946684799999999), "1999-12-31T23:59:59.999999Z" },
    { ::make_timepoint(946684799999999), "2000-01-01T00:29:59.999999+00:30" },
    { ::make_timepoint(946684799999999), "2000-01-01T00:59:59.999999+01:00" },
    { ::make_timepoint(946684799999999), "2000-01-01T11:59:59.999999+12:00" },
    // 2000-01-01T00:00:00Z
    { ::make_timepoint(946684800000000), "1999-12-31T12:00:00-12:00" },
    { ::make_timepoint(946684800000000), "1999-12-31T23:00:00-01:00" },
    { ::make_timepoint(946684800000000), "1999-12-31T23:30:00-00:30" },
    { ::make_timepoint(946684800000000), "2000-01-01T00:00:00Z" },
    { ::make_timepoint(946684800000000), "2000-01-01T00:30:00+00:30" },
    { ::make_timepoint(946684800000000), "2000-01-01T01:00:00+01:00" },
    { ::make_timepoint(946684800000000), "2000-01-01T12:00:00+12:00" },
    // 2199-12-31T23:59:59.999999Z
    { ::make_timepoint(7258118399999999), "2199-12-31T11:59:59.999999-12:00" },
    { ::make_timepoint(7258118399999999), "2199-12-31T22:59:59.999999-01:00" },
    { ::make_timepoint(7258118399999999), "2199-12-31T23:29:59.999999-00:30" },
    { ::make_timepoint(7258118399999999), "2199-12-31T23:59:59.999999Z" },
    { ::make_timepoint(7258118399999999), "2200-01-01T00:29:59.999999+00:30" },
    { ::make_timepoint(7258118399999999), "2200-01-01T00:59:59.999999+01:00" },
    { ::make_timepoint(7258118399999999), "2200-01-01T11:59:59.999999+12:00" }}));
  INFO("Timepoint=" << ::to_string(timepoint));
  INFO("Timestamp=" << timestamp);
  REQUIRE(flib::timestamp::parse(timestamp).get() == timepoint);
}

TEST_CASE("Timestamp tests - to_string-parse cycle check", "[timestamp]")
{
  bool utc = GENERATE(true, false);
  flib::timestamp::time_point_t timepoint = GENERATE(
    flib::timestamp::time_point_t(),
    ::make_timepoint(86400000000),
    ::make_timepoint(946684799999999),
    ::make_timepoint(946684799123456),
    ::make_timepoint(946684800000000),
    ::make_timepoint(7258118399123000),
    ::make_timepoint(7258118399999999),
    ::make_timepoint_now());
  flib::timestamp timestamp(timepoint);
  INFO("Timepoint=" << ::to_string(timepoint));
  flib::timestamp parsed_timestamp_utc = flib::timestamp::parse(timestamp.to_string(utc, flib::timestamp::precision::max));
  flib::timestamp parsed_timestamp_local = flib::timestamp::parse(flib::to_string(timestamp, utc, flib::timestamp::precision::max));
  REQUIRE(timepoint == parsed_timestamp_utc.get());
  REQUIRE(timestamp == parsed_timestamp_utc);
  REQUIRE(timepoint == parsed_timestamp_local.get());
  REQUIRE(timestamp == parsed_timestamp_local);
}

TEST_CASE("Timestamp tests - parse-to_string cycle check", "[timestamp]")
{
  std::string timestamp = GENERATE(
    "1970-01-01T00:00:00Z",
    "1999-12-31T23:59:59.999999Z",
    "2000-01-01T00:00:00Z",
    "2199-12-31T23:59:59.999999Z");
  INFO("UTC timestamp=" << timestamp);
  REQUIRE(timestamp == flib::timestamp::parse(timestamp).to_string(true, flib::timestamp::precision::max));
  REQUIRE(timestamp == flib::to_string(flib::timestamp::parse(timestamp), true, flib::timestamp::precision::max));
}