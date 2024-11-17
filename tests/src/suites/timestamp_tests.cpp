// Copyright © 2021-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/timestamp.hpp>

#include <cstdint>
#include <chrono>
#include <list>
#include <ratio>
#include <regex>
#include <stdexcept>
#include <string>

#include <catch2/catch2.hpp>

namespace
{
  using microseconds = std::chrono::duration<uint64_t, std::micro>;

  bool check_timestamp(const std::string& timestamp)
  {
    static const std::regex regex("^([0-9]{4})-([0-9]{2})-([0-9]{2})"
      "([Tt]([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.[0-9]+)?)?"
      "(([Zz]|([+-])([0-9]{2}):([0-9]{2})))?$");
    return std::regex_match(timestamp, regex);
  }

  std::chrono::minutes get_offset(const std::string& timestamp)
  {
    std::tm temp{};
    std::istringstream stream(timestamp.substr(timestamp.size() - 5));
    stream >> std::get_time(&temp, "%R");
    auto offset = std::chrono::hours(temp.tm_hour) + std::chrono::minutes(temp.tm_min);
    return '+' == timestamp.at(timestamp.size() - 6) ? offset : -offset;
  }
}

TEST_CASE("Timestamp tests - Formatting", "[timestamp]")
{
  std::chrono::system_clock::time_point timepoint;
  SECTION("1970-01-01 00:00:00")
  {
  }
  SECTION("1999-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(946684799999999);
  }
  SECTION("2000-01-01 00:00:00")
  {
    timepoint += ::microseconds(946684800000000);
  }
  SECTION("2199-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(7258118399999999);
  }
  SECTION("Current time")
  {
    timepoint = std::chrono::system_clock::now();
  }
  REQUIRE(::check_timestamp(flib::generate(timepoint)));
  REQUIRE(::check_timestamp(flib::generate(timepoint, false)));
}

TEST_CASE("Timestamp tests - Generator check", "[timestamp]")
{
  std::chrono::system_clock::time_point timepoint;
  std::string utc_timestamp;
  SECTION("1970-01-02 00:00:00")
  {
    timepoint += ::microseconds(86400000000);
    utc_timestamp = "1970-01-02T00:00:00Z";
  }
  SECTION("1999-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(946684799999999);
    utc_timestamp = "1999-12-31T23:59:59.999999Z";
  }
  SECTION("2000-01-01 00:00:00")
  {
    timepoint += ::microseconds(946684800000000);
    utc_timestamp = "2000-01-01T00:00:00Z";
  }
  SECTION("2199-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(7258118399999999);
    utc_timestamp = "2199-12-31T23:59:59.999999Z";
  }
  REQUIRE(flib::generate(timepoint) == utc_timestamp);
  REQUIRE_THAT(flib::generate(timepoint - ::get_offset(flib::generate(timepoint, false)), false),
    Catch::Matchers::StartsWith(utc_timestamp.substr(0, utc_timestamp.size() - 1)));
}

TEST_CASE("Timestamp tests - Parser check", "[timestamp]")
{
  std::chrono::system_clock::time_point timepoint;
  std::list<std::string> timestamps;
  SECTION("1970-01-01 00:00:00")
  {
    timestamps = {
      "1969-12-31T12:00:00-12:00",
      "1969-12-31T23:00:00-01:00",
      "1969-12-31T23:30:00-00:30",
      "1970-01-01T00:00:00Z",
      "1970-01-01T00:30:00+00:30",
      "1970-01-01T01:00:00+01:00",
      "1970-01-01T12:00:00+12:00"
    };
  }
  SECTION("1999-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(946684799999999);
    timestamps = {
      "1999-12-31T11:59:59.999999-12:00",
      "1999-12-31T22:59:59.999999-01:00",
      "1999-12-31T23:29:59.999999-00:30",
      "1999-12-31T23:59:59.999999Z",
      "2000-01-01T00:29:59.999999+00:30",
      "2000-01-01T00:59:59.999999+01:00",
      "2000-01-01T11:59:59.999999+12:00"
    };
  }
  SECTION("2000-01-01 00:00:00")
  {
    timepoint += ::microseconds(946684800000000);
    timestamps = {
      "1999-12-31T12:00:00-12:00",
      "1999-12-31T23:00:00-01:00",
      "1999-12-31T23:30:00-00:30",
      "2000-01-01T00:00:00Z",
      "2000-01-01T00:30:00+00:30",
      "2000-01-01T01:00:00+01:00",
      "2000-01-01T12:00:00+12:00"
    };
  }
  SECTION("2099-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(7258118399999999);
    timestamps = {
      "2199-12-31T11:59:59.999999-12:00",
      "2199-12-31T22:59:59.999999-01:00",
      "2199-12-31T23:29:59.999999-00:30",
      "2199-12-31T23:59:59.999999Z",
      "2200-01-01T00:29:59.999999+00:30",
      "2200-01-01T00:59:59.999999+01:00",
      "2200-01-01T11:59:59.999999+12:00"
    };
  }
  for (const auto& timestamp : timestamps)
  {
    REQUIRE(flib::parse(timestamp) == timepoint);
  }
}

TEST_CASE("Timestamp tests - Generate-parse cycle equality", "[timestamp]")
{
  std::chrono::system_clock::time_point timepoint;
  SECTION("1970-01-01 00:00:00")
  {
  }
  SECTION("1999-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(946684799999999);
  }
  SECTION("2000-01-01 00:00:00")
  {
    timepoint += ::microseconds(946684800000000);
  }
  SECTION("2199-12-31 23:59:59.999999")
  {
    timepoint += ::microseconds(7258118399999999);
  }
  SECTION("Current time")
  {
    timepoint += std::chrono::duration_cast<::microseconds>(std::chrono::system_clock::now().time_since_epoch());
  }
  REQUIRE(flib::parse(flib::generate(timepoint)) == timepoint);
  REQUIRE(flib::parse(flib::generate(timepoint, false)) == timepoint);
}