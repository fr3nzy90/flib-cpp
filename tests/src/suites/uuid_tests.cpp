// Copyright © 2021-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/uuid.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <regex>
#include <string>

#include <catch2/catch2.hpp>

namespace
{
  inline std::string to_upper(const std::string& p_str)
  {
    std::string result(p_str.size(), ' ');
    std::transform(p_str.cbegin(), p_str.cend(), result.begin(), [](std::string::value_type p_char) -> std::string::value_type
      {
        return static_cast<std::string::value_type>(::toupper(static_cast<int>(p_char)));
      });
    return result;
  }
}

TEST_CASE("Uuid tests - Sanity check", "[uuid]")
{
  flib::uuid uuid;
  uint8_t version{ 0u };
  uint8_t variant{ 0u };
  SECTION("nil")
  {
    uuid = GENERATE(
      flib::uuid(),
      flib::uuid::generate_nil());
    version = 0u;
    variant = 0u;
  }
  SECTION("max")
  {
    uuid = flib::uuid::generate_max();
    version = 15u;
    variant = 7u;
  }
  SECTION("v4")
  {
    uuid = flib::uuid::generate_v4();
    version = 4u;
    variant = 2u;
  }
  SECTION("v7")
  {
    uuid = flib::uuid::generate_v7();
    version = 7u;
    variant = 2u;
  }
  INFO("Version=" << version);
  INFO("Variant=" << variant);
  REQUIRE(version == uuid.get_version());
  REQUIRE(variant == uuid.get_variant());
  REQUIRE(uuid.valid());
}

TEST_CASE("Uuid tests - Comparison check", "[uuid]")
{
  SECTION("nil equality")
  {
    REQUIRE(flib::uuid() == flib::uuid::generate_nil());
    REQUIRE(flib::uuid::generate_nil() == flib::uuid::generate_nil());
  }
  SECTION("max equality")
  {
    REQUIRE(flib::uuid::generate_max() == flib::uuid::generate_max());
  }
  SECTION("Inequality")
  {
    REQUIRE(flib::uuid::generate_nil() != flib::uuid::generate_max());
    REQUIRE(flib::uuid::generate_nil() != flib::uuid::generate_v4());
    REQUIRE(flib::uuid::generate_nil() != flib::uuid::generate_v7());
    REQUIRE(flib::uuid::generate_v4() != flib::uuid::generate_v4());
    REQUIRE(flib::uuid::generate_v7() != flib::uuid::generate_v7());
  }
}

TEST_CASE("Uuid tests - Formatting", "[uuid]")
{
  static const auto check = [](const std::string& p_uuid, uint8_t p_version, bool p_uppercase)
    {
      static const std::regex regex_v4_lower("^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");
      static const std::regex regex_v4_upper("^[0-9A-F]{8}-[0-9A-F]{4}-4[0-9A-F]{3}-[89AB][0-9A-F]{3}-[0-9A-F]{12}$");
      static const std::regex regex_v7_lower("^[0-9a-f]{8}-[0-9a-f]{4}-7[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");
      static const std::regex regex_v7_upper("^[0-9A-F]{8}-[0-9A-F]{4}-7[0-9A-F]{3}-[89AB][0-9A-F]{3}-[0-9A-F]{12}$");
      switch (p_version)
      {
      case 0u:
        return "00000000-0000-0000-0000-000000000000" == p_uuid;
      case 4u:
        return std::regex_match(p_uuid, p_uppercase ? regex_v4_upper : regex_v4_lower);
      case 7u:
        return std::regex_match(p_uuid, p_uppercase ? regex_v7_upper : regex_v7_lower);
      case 15u:
        return (p_uppercase ? "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF" : "ffffffff-ffff-ffff-ffff-ffffffffffff") == p_uuid;
      default:
        return false;
      }
    };
  flib::uuid uuid = GENERATE(
    flib::uuid(),
    flib::uuid::generate_nil(),
    flib::uuid::generate_max(),
    flib::uuid::generate_v4(),
    flib::uuid::generate_v7());
  bool uppercase = GENERATE(true, false);
  std::string uuid_str = uuid.to_string(uppercase);
  INFO("     UUID=" << uuid_str);
  INFO("Uppercase=" << std::boolalpha << uppercase);
  REQUIRE(check(uuid_str, uuid.get_version(), uppercase));
}

TEST_CASE("Uuid tests - Parsing", "[uuid]")
{
  SECTION("Valid UUIDs")
  {
    std::string uuid_str;
    uint8_t version{ 0u };
    uint8_t variant{ 0u };
    std::unique_ptr<flib::uuid> uuid_ref;
    bool uppercase = GENERATE(true, false);
    SECTION("nil")
    {
      uuid_str = "00000000-0000-0000-0000-000000000000";
      version = 0u;
      variant = 0u;
      uuid_ref.reset(new flib::uuid(flib::uuid::generate_nil()));
    }
    SECTION("max")
    {
      uuid_str = "ffffffff-ffff-ffff-ffff-ffffffffffff";
      version = 15u;
      variant = 7u;
      uuid_ref.reset(new flib::uuid(flib::uuid::generate_max()));
    }
    SECTION("v4")
    {
      uuid_str = GENERATE(
        "e8be81a6-c70c-4045-87c5-b7505d0c024f",
        "d4cb4f21-7706-40bd-ba89-f832d5201bf4",
        "9bfc28eb-33cc-4e11-9a44-a69543186ad2",
        "a3c94bdd-8f3c-42b6-a84f-ce4932225064",
        "046ce4f7-8678-4660-8e9d-6de1335714c8");
      version = 4u;
      variant = 2u;
    }
    SECTION("v7")
    {
      uuid_str = GENERATE(
        "e8be81a6-c70c-7045-87c5-b7505d0c024f",
        "d4cb4f21-7706-70bd-ba89-f832d5201bf4",
        "9bfc28eb-33cc-7e11-9a44-a69543186ad2",
        "a3c94bdd-8f3c-72b6-a84f-ce4932225064",
        "046ce4f7-8678-7660-8e9d-6de1335714c8");
      version = 7u;
      variant = 2u;
    }
    if (uppercase)
    {
      uuid_str = ::to_upper(uuid_str);
    }
    INFO("   UUID=" << uuid_str);
    INFO("Version=" << version);
    INFO("Variant=" << variant);
    flib::uuid uuid = flib::uuid::parse(uuid_str);
    REQUIRE(version == uuid.get_version());
    REQUIRE(variant == uuid.get_variant());
    REQUIRE((!uuid_ref || *uuid_ref.get() == uuid));
    REQUIRE(uuid.valid());
  }

  SECTION("Invalid UUIDs with good format")
  {
    std::string uuid = GENERATE(
      "00000000-0000-0000-1000-000000000000",
      "00000000-0000-1000-0000-000000000000",
      "00000000-0000-2000-0000-000000000000",
      "00000000-0000-3000-0000-000000000000",
      "e8be81a6-c70c-4045-07c5-b7505d0c024f",
      "00000000-0000-5000-0000-000000000000",
      "00000000-0000-6000-0000-000000000000",
      "e8be81a6-c70c-7045-07c5-b7505d0c024f",
      "00000000-0000-8000-0000-000000000000",
      "00000000-0000-9000-0000-000000000000",
      "00000000-0000-a000-0000-000000000000",
      "00000000-0000-b000-0000-000000000000",
      "00000000-0000-c000-0000-000000000000",
      "00000000-0000-d000-0000-000000000000",
      "00000000-0000-e000-0000-000000000000",
      "ffffffff-ffff-ffff-efff-ffffffffffff");
    bool uppercase = GENERATE(true, false);
    if (uppercase)
    {
      uuid = ::to_upper(uuid);
    }
    REQUIRE(!flib::uuid::parse(uuid).valid());
  }
  SECTION("Unsupported format")
  {
    std::string uuid = GENERATE(
      "00000000-0000-0000-0000-0000000000000",
      "00000000x0000-0000-0000-000000000000",
      "x0000000-0000-0000-0000-000000000000");
    REQUIRE_THROWS_MATCHES(flib::uuid::parse(uuid), std::runtime_error, Catch::Matchers::Message("Invalid UUID format"));
  }
}

TEST_CASE("Uuid tests - to_string->parse cycle equality", "[uuid]")
{
  flib::uuid uuid = GENERATE(
    flib::uuid::generate_nil(),
    flib::uuid::generate_max(),
    flib::uuid::generate_v4(),
    flib::uuid::generate_v7());
  bool uppercase = GENERATE(true, false);
  std::string uuid_str = uuid.to_string(uppercase);
  INFO("UUID=" << uuid_str);
  REQUIRE(uuid == flib::uuid::parse(uuid_str));
}

TEST_CASE("Uuid tests - parse->to_string cycle equality", "[uuid]")
{
  static const auto check = [](const std::string& p_str1, const std::string& p_str2)
    {
      if (p_str1.size() != p_str2.size())
      {
        return false;
      }
      for (std::size_t i = 0; i < p_str1.size(); ++i)
      {
        if (std::tolower(static_cast<unsigned char>(p_str1.at(i))) != std::tolower(static_cast<unsigned char>(p_str2.at(i))))
        {
          return false;
        }
      }
      return true;
    };
  std::string uuid = GENERATE(
    "00000000-0000-0000-0000-000000000000",
    "ffffffff-ffff-ffff-ffff-ffffffffffff",
    "e8be81a6-c70c-4045-87c5-b7505d0c024f",
    "e8be81a6-c70c-7045-87c5-b7505d0c024f");
  bool uppercase = GENERATE(true, false);
  if (uppercase)
  {
    uuid = ::to_upper(uuid);
  }
  INFO("UUID=" << uuid);
  REQUIRE(check(uuid, flib::uuid::parse(uuid).to_string()));
}