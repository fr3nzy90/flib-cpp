// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cstdint>

#if !defined(FLIB_NO_MEMORY_LEAK_DETECTION) & defined(_DEBUG) & defined(_MSC_VER)
#  define FLIB_MEMORY_LEAK_DETECTION
#  define _CRTDBG_MAP_ALLOC
#  include <stdlib.h>
#  include <crtdbg.h>
#  if !defined(FLIB_NO_MEMORY_LEAK_DETECTION_NEW)
#    define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#  endif
#endif

#include <flib/flags.hpp>

namespace flib
{
#pragma region API
  // Optional macros:
  //  - FLIB_NO_MEMORY_LEAK_DETECTION ....... disables memory leak detection
  //  - FLIB_NO_MEMORY_LEAK_DETECTION_NEW ... disables definition of c++ new operator for more detailed memory dump

  class memory_leak_detector
  {
    // Memory leak detection is only available for Microsoft Visual Studio compiler and debug mode
    // Please refer to the following documentation for memory leak detection:
    //   - for Windows platform refer
    //     https://learn.microsoft.com/en-us/cpp/c-runtime-library/find-memory-leaks-using-the-crt-library

  public:
    enum class flags
      : uint16_t
    {
      debug_heap_allocations = 0b000000001, // Refer to: _CRTDBG_ALLOC_MEM_DF
      delay_free_memory      = 0b000000010, // Refer to: _CRTDBG_DELAY_FREE_MEM_DF
      exit_leak_check        = 0b000000100, // Refer to: _CRTDBG_LEAK_CHECK_DF
      check_crt_types        = 0b000001000, // Refer to: _CRTDBG_CHECK_CRT_DF
      check_manual           = 0b000010000, // Refer to: _CRTDBG_CHECK_DEFAULT_DF
      check_every_16         = 0b000100000, // Refer to: _CRTDBG_CHECK_EVERY_16_DF
      check_every_128        = 0b001000000, // Refer to: _CRTDBG_CHECK_EVERY_128_DF
      check_every_1024       = 0b010000000, // Refer to: _CRTDBG_CHECK_EVERY_1024_DF
      check_always           = 0b100000000  // Refer to: _CRTDBG_CHECK_ALWAYS_DF
    };

    static constexpr flags s_default_flags = flags::debug_heap_allocations | flags::exit_leak_check;

  public:
    static void dump_leaks(void);
    static void set_allocation_break(long p_number = -1);
    static void setup(flags p_flags = s_default_flags);
    static bool supported(void);
  };
#pragma endregion

#pragma region IMPLEMENTATION
#if defined(FLIB_MEMORY_LEAK_DETECTION)
  inline void memory_leak_detector::dump_leaks(void)
  {
    _CrtDumpMemoryLeaks();
  }

  inline void memory_leak_detector::set_allocation_break(long p_number)
  {
    _CrtSetBreakAlloc(p_number);
  }

  inline void memory_leak_detector::setup(flags p_flags)
  {
    int internal_flags = 0;
    if (flib::is_flag_set(p_flags, flags::debug_heap_allocations))
    {
      internal_flags |= _CRTDBG_ALLOC_MEM_DF;
    }
    if (flib::is_flag_set(p_flags, flags::delay_free_memory))
    {
      internal_flags |= _CRTDBG_DELAY_FREE_MEM_DF;
    }
    if (flib::is_flag_set(p_flags, flags::exit_leak_check))
    {
      internal_flags |= _CRTDBG_LEAK_CHECK_DF;
    }
    if (flib::is_flag_set(p_flags, flags::check_crt_types))
    {
      internal_flags |= _CRTDBG_CHECK_CRT_DF;
    }
    if (flib::is_flag_set(p_flags, flags::check_manual))
    {
      internal_flags |= _CRTDBG_CHECK_DEFAULT_DF;
    }
    if (flib::is_flag_set(p_flags, flags::check_every_16))
    {
      internal_flags |= _CRTDBG_CHECK_EVERY_16_DF;
    }
    if (flib::is_flag_set(p_flags, flags::check_every_128))
    {
      internal_flags |= _CRTDBG_CHECK_EVERY_128_DF;
    }
    if (flib::is_flag_set(p_flags, flags::check_every_1024))
    {
      internal_flags |= _CRTDBG_CHECK_EVERY_1024_DF;
    }
    if (flib::is_flag_set(p_flags, flags::check_always))
    {
      internal_flags |= _CRTDBG_CHECK_ALWAYS_DF;
    }
    _CrtSetDbgFlag(internal_flags);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  }

  inline bool memory_leak_detector::supported(void)
  {
    return true;
  }
#else
  inline void memory_leak_detector::dump_leaks(void)
  {
  }

  inline void memory_leak_detector::set_allocation_break(long)
  {
  }

  inline void memory_leak_detector::setup(flags)
  {
  }

  inline bool memory_leak_detector::supported(void)
  {
    return false;
  }
#endif
#pragma endregion
}