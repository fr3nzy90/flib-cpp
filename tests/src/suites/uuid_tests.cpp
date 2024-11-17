// Copyright © 2021-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/uuid.hpp>

#include <regex>
#include <string>

#include <catch2/catch2.hpp>

namespace
{
  bool check_uuid_v4(const std::string& uuid)
  {
    static const std::regex regex("^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$", std::regex_constants::icase);
    return std::regex_match(uuid, regex);
  }
}

TEST_CASE("Uuid tests - Formatting", "[uuid]")
{
  SECTION("v4")
  {
    for (auto i = 10; i > 0; --i)
    {
      REQUIRE(::check_uuid_v4(flib::generate()));
    }
  }
}

TEST_CASE("Uuid tests - Test check", "[uuid]")
{
  SECTION("v4")
  {
    for (const auto& uuid : {
      "e8be81a6-c70c-4045-87c5-b7505d0c024f",
      "d4cb4f21-7706-40bd-ba89-f832d5201bf4",
      "9bfc28eb-33cc-4e11-9a44-a69543186ad2",
      "a3c94bdd-8f3c-42b6-a84f-ce4932225064",
      "046ce4f7-8678-4660-8e9d-6de1335714c8",
      "fd476632-c624-49e9-9adc-d8bcaa60c68c",
      "0e54e697-bace-4ffb-a54a-8f65f7d51443",
      "3ac42c0a-58e5-4e02-bfe8-e8e5002a9fb5",
      "33f31336-be42-44e0-bcd8-845db446c9fc",
      "39c2f8c3-70b0-4dc0-a165-a693358df673",
      "4EEEDB2F-BA0B-42B7-AE9F-EDA7A2EBE78C",
      "9A56B17E-09C6-454C-81F6-D729F610851A",
      "977EF5E9-6559-4F08-A217-2E2B5702B0C1",
      "29F9C42D-B2CA-4108-A1AB-3D4A547A8792",
      "F501116E-55A3-4750-8AC9-6154D8FFE279",
      "798F9850-9CA6-4BEC-83D6-AB6939C0A9F2",
      "2EC99398-3C7B-41E9-9B96-4F1E2F991D82",
      "ED9B83DA-33E9-4727-909F-BFD45828370A",
      "0FFDBA42-5CD3-4128-B1E0-1DE75D9DD159",
      "963D4671-055D-42C9-8650-69ECE3DB7826"
      })
    {
      REQUIRE(flib::test(uuid));
    }
  }
}

TEST_CASE("Uuid tests - Generate-test cycle equality", "[uuid]")
{
  SECTION("v4")
  {
    for (auto i = 10; i > 0; --i)
    {
      REQUIRE(flib::test(flib::generate()));
    }
  }
}