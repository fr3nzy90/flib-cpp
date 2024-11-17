// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/dll.hpp>

#include <string>

#include <catch2/catch2.hpp>

namespace
{
  const std::string g_valid_module_path(
#if defined(_WIN32)
    "./test_dll"
#elif defined(__linux__)
    "./test_dll.so"
#endif
  );
  const std::string g_invalid_module_path(
#if defined(_WIN32)
    "./test_invalid_dll"
#elif defined(__linux__)
    "./test_invalid_dll.so"
#endif
  );
  constexpr int64_t g_custom_flags =
#if defined(_WIN32)      // Windows
    LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
#elif defined(__linux__) // Linux
    RTLD_NOW | RTLD_LOCAL;
#else
#  error "Unsupported platform/compiler"
#endif
  const std::string g_valid_exported_function_name("multiply");
  const std::string g_invalid_exported_function_name("invalid_multiply");
}

TEST_CASE("Dynamic-link library tests - Sanity check", "[dll]")
{
  SECTION("Default construction")
  {
    flib::dll library;
    REQUIRE(false == library.loaded());
    REQUIRE("" == library.filepath());
    REQUIRE(0ll == library.flags());
  }
  SECTION("Construction with valid module and default flags")
  {
    flib::dll library(::g_valid_module_path);
    REQUIRE(true == library.loaded());
    REQUIRE(::g_valid_module_path == library.filepath());
    REQUIRE(flib::dll::s_default_flags == library.flags());
  }
  SECTION("Construction with valid module and custom flags")
  {
    flib::dll library(::g_valid_module_path, ::g_custom_flags);
    REQUIRE(true == library.loaded());
    REQUIRE(::g_valid_module_path == library.filepath());
    REQUIRE(::g_custom_flags == library.flags());
  }
  SECTION("Construction with invalid module and default flag")
  {
    REQUIRE_THROWS_MATCHES(flib::dll(::g_invalid_module_path), std::runtime_error,
      Catch::Matchers::MessageMatches(Catch::Matchers::StartsWith("Module loading failed")));
  }
  SECTION("Construction with invalid module and custom flag")
  {
    REQUIRE_THROWS_MATCHES(flib::dll(::g_invalid_module_path, ::g_custom_flags), std::runtime_error,
      Catch::Matchers::MessageMatches(Catch::Matchers::StartsWith("Module loading failed")));
  }
}

TEST_CASE("Dynamic-link library tests - Module loading", "[dll]")
{
  SECTION("Loading valid module into unloaded container with default flags")
  {
    flib::dll library;
    REQUIRE(false == library.loaded());
    REQUIRE_NOTHROW(library.load(::g_valid_module_path));
    REQUIRE(true == library.loaded());
    REQUIRE(::g_valid_module_path == library.filepath());
    REQUIRE(flib::dll::s_default_flags == library.flags());
  }
  SECTION("Loading valid module into unloaded container with custom flags")
  {
    flib::dll library;
    REQUIRE(false == library.loaded());
    REQUIRE_NOTHROW(library.load(::g_valid_module_path, ::g_custom_flags));
    REQUIRE(true == library.loaded());
    REQUIRE(::g_valid_module_path == library.filepath());
    REQUIRE(::g_custom_flags == library.flags());
  }
  SECTION("Loading loaded container with default flags")
  {
    flib::dll library(::g_valid_module_path);
    REQUIRE(true == library.loaded());
    REQUIRE_THROWS_MATCHES(library.load(::g_invalid_module_path), std::runtime_error, Catch::Matchers::Message("DLL already loaded"));
    REQUIRE(true == library.loaded());
    REQUIRE(::g_valid_module_path == library.filepath());
    REQUIRE(flib::dll::s_default_flags == library.flags());
  }
  SECTION("Loading loaded container with custom flags")
  {
    flib::dll library(::g_valid_module_path);
    REQUIRE(true == library.loaded());
    REQUIRE_THROWS_MATCHES(library.load(::g_invalid_module_path, ::g_custom_flags), std::runtime_error,
      Catch::Matchers::Message("DLL already loaded"));
    REQUIRE(true == library.loaded());
    REQUIRE(::g_valid_module_path == library.filepath());
    REQUIRE(flib::dll::s_default_flags == library.flags());
  }
  SECTION("Loading invalid module into unloaded container with default flags")
  {
    flib::dll library;
    REQUIRE(false == library.loaded());
    REQUIRE_THROWS_MATCHES(library.load(::g_invalid_module_path), std::runtime_error,
      Catch::Matchers::MessageMatches(Catch::Matchers::StartsWith("Module loading failed")));
    REQUIRE(false == library.loaded());
    REQUIRE(::g_invalid_module_path == library.filepath());
    REQUIRE(flib::dll::s_default_flags == library.flags());
  }
  SECTION("Loading invalid module into unloaded container with custom flags")
  {
    flib::dll library;
    REQUIRE(false == library.loaded());
    REQUIRE_THROWS_MATCHES(library.load(::g_invalid_module_path, ::g_custom_flags), std::runtime_error,
      Catch::Matchers::MessageMatches(Catch::Matchers::StartsWith("Module loading failed")));
    REQUIRE(false == library.loaded());
    REQUIRE(::g_invalid_module_path == library.filepath());
    REQUIRE(::g_custom_flags == library.flags());
  }
}

TEST_CASE("Dynamic-link library tests - Module unloading", "[dll]")
{
  SECTION("Unloading loaded container")
  {
    flib::dll library(::g_valid_module_path);
    REQUIRE(true == library.loaded());
    REQUIRE_NOTHROW(library.unload());
    REQUIRE(false == library.loaded());
  }
  SECTION("Unloading unloaded container")
  {
    flib::dll library;
    REQUIRE(false == library.loaded());
    REQUIRE_NOTHROW(library.unload());
    REQUIRE(false == library.loaded());
  }
}

TEST_CASE("Dynamic-link library tests - Exported functions", "[dll]")
{
  SECTION("Retrieving exported function from loaded container")
  {
    flib::dll library(::g_valid_module_path);
    auto func = library.get_function<int(int, int)>(::g_valid_exported_function_name);
    REQUIRE(6 == func(2, 3));
  }
  SECTION("Retrieving exported function from unloaded container")
  {
    flib::dll library;
    REQUIRE_THROWS_MATCHES(library.get_function<int(int, int)>(::g_valid_exported_function_name), std::runtime_error,
      Catch::Matchers::Message("DLL not loaded"));
  }
  SECTION("Retrieving invalid function")
  {
    flib::dll library(::g_valid_module_path);
    REQUIRE_THROWS_MATCHES(library.get_function<int(int, int)>(::g_invalid_exported_function_name), std::runtime_error,
      Catch::Matchers::MessageMatches(Catch::Matchers::StartsWith("Function retrieval from module failed")));
  }
}