// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <ios>
#include <iostream>

#include <flib/mld.hpp>

int main(int /*argc*/, char* /*argv*/[]) {
  // flags are same as default, they are created only for example
  auto flags = flib::memory_leak_detector::flags::debug_heap_allocations | flib::memory_leak_detector::flags::exit_leak_check;

  // create memory leak detector
  flib::memory_leak_detector::setup(flags);

  // check if memory leak detection is supported
  std::cout << "Memory leak detection supported: " << std::boolalpha << flib::memory_leak_detector::supported() << '\n';

  // problematic allocation number is known and fixed, it is possible to stop on that specific allocation number
  //   => example in commented line below
  // flib::memory_leak_detector::set_allocation_break(85);

  // create a first leak
  auto leaked_1 = new int[3];
  (void)leaked_1;

  // dump report - it should contain only first leak
  flib::memory_leak_detector::dump_leaks();

  // delete first leak
  delete[] leaked_1;

  // create a second leak
  auto leaked_2 = new char[5];
  (void)leaked_2;

  //  dump report will be created on exit - it should only contain second leak
  return 0;
}