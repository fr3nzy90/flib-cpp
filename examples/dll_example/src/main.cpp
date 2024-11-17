// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <iostream>

#include <flib/dll.hpp>
#include <flib/mld.hpp>

namespace
{
#pragma region Helpers
  const std::string g_valid_module_path(
#if defined(_WIN32)
    "./test_dll"
#elif defined(__linux__)
    "./test_dll.so"
#endif
  );
  const std::string g_exported_function_name("multiply");

  void print_library(const flib::dll& p_library)
  {
    // print if library data
    std::cout << "Library " << (p_library.loaded() ? "" : "not ") << "loaded" <<
      " on path=\"" << p_library.filepath() << '"' <<
      " with flags=" << p_library.flags() << '\n';
  }

  void get_and_execute(const flib::dll& p_library, std::string p_name, int p_value_1, int p_value_2)
  {
    // retrieve function from dll
    auto func = p_library.get_function<int(int, int)>(p_name);

    // execute function
    std::cout << "Execute function from dll: " << p_value_1 << '*' << p_value_2 << "=" << func(p_value_1, p_value_2) << '\n';
  }
#pragma endregion

#pragma region Examples
  void example_dll_usage(void)
  {
    // create library handle and load it
    flib::dll library(::g_valid_module_path);

    // check if dll is loaded and read loading path/parameters
    ::print_library(library);

    // retrieve and execute function from dll
    ::get_and_execute(library, ::g_exported_function_name, 2, 3);

    // manually unload dll
    library.unload();

    // check if dll is loaded and read loading path/parameters
    ::print_library(library);

    // load dll
    library.load(::g_valid_module_path);

    // check if dll is loaded and read loading path/parameters
    ::print_library(library);

    // retrieve and execute function from dll
    ::get_and_execute(library, ::g_exported_function_name, 3, 5);
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_dll_usage();

  return 0;
}