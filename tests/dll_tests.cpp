/*
* MIT License
*
* Copyright (c) 2020 Luka Arnecic
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <flib/dll.hpp>

#include "testing.hpp"

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