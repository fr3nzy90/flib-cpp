// Copyright � 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <cstdint>
#include <ios>
#include <iostream>
#include <string>

#include <flib/flags.hpp>
#include <flib/mld.hpp>

namespace
{
#pragma region Helpers
  enum class test_flags
    : uint8_t
  {
    option_1 = 0b0001,
    option_2 = 0b0010,
    option_3 = 0b0100,
    option_4 = 0b1000
  };

  void print_flags(::test_flags p_flags)
  {
    std::cout << std::boolalpha
      << "option_1: " << flib::is_flag_set(p_flags, ::test_flags::option_1) << '\n'
      << "option_2: " << flib::is_flag_set(p_flags, ::test_flags::option_2) << '\n'
      << "option_3: " << flib::is_flag_set(p_flags, ::test_flags::option_3) << '\n'
      << "option_4: " << flib::is_flag_set(p_flags, ::test_flags::option_4) << '\n';
  }
#pragma endregion

#pragma region Examples
  void example_flags_operators(void)
  {
    // enable access to operators
    using namespace flib;

    // combine option_1 and option_2
    auto combined_flags = ::test_flags::option_1 | ::test_flags::option_2;

    // add option_4
    combined_flags |= ::test_flags::option_4;
    // alternative way of previous operation
    // combined_flags = combined_flags | ::test_flags::option_4;

    // print flags
    ::print_flags(combined_flags);

    // add option_3
    combined_flags |= ::test_flags::option_3;
    // remove option_4
    combined_flags &= ~::test_flags::option_4;
    // alternative way of previous operation
    // combined_flags = combined_flags & ~::test_flags::option_4;

    // print flags
    ::print_flags(combined_flags);

    // add option_4 and remove option_3
    combined_flags ^= ::test_flags::option_3 | ::test_flags::option_4;
    // alternative way of previous operation
    // combined_flags = combined_flags ^ (::test_flags::option_3 | ::test_flags::option_4);

    // print flags
    ::print_flags(combined_flags);
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_flags_operators();

  return 0;
}