// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/dll.hpp>

#include <string>

#include <catch2/catch2.hpp>

namespace
{
  const std::string module_path(
#if defined(_WIN32)
    "./test_dll"
#elif defined(__linux__)
    "./test_dll.so"
#endif
  );
}

TEST_CASE("Dynamic-link library tests - Sanity check", "[dll]")
{
  SECTION("Existing module")
  {
    auto library = flib::open_library(::module_path);
    flib::close_library(library);
  }
  SECTION("Non-existing module")
  {
    REQUIRE_THROWS_MATCHES(flib::open_library(::module_path + ".xyz"), std::runtime_error,
      Catch::Matchers::MessageMatches(Catch::Matchers::StartsWith("Module loading failed")));
    REQUIRE_THROWS_MATCHES(flib::get_library_function<void(void)>(nullptr, "invalid"), std::invalid_argument,
      Catch::Matchers::Message("Invalid module handle"));
    REQUIRE_THROWS_MATCHES(flib::close_library(nullptr), std::invalid_argument,
      Catch::Matchers::Message("Invalid module handle"));
  }
}

TEST_CASE("Dynamic-link library tests - Exported functions", "[dll]")
{
  SECTION("Exported function")
  {
    auto library = flib::open_library(::module_path);
    auto func = flib::get_library_function<int(int, int)>(library, "multiply");
    REQUIRE(6 == func(2, 3));
    flib::close_library(library);
  }
  SECTION("Invalid function")
  {
    auto library = flib::open_library(::module_path);
    REQUIRE_THROWS_MATCHES(flib::get_library_function<void(void)>(library, "invalid"), std::runtime_error,
      Catch::Matchers::MessageMatches(Catch::Matchers::StartsWith("Function retrieval failed")));
    flib::close_library(library);
  }
}