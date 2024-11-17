// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#if defined(_WIN32)
#  define API __declspec(dllexport)
#elif defined(__GNUC__)
#  define API __attribute__((visibility("default")))
#else
#  error "Unsupported compiler/platform"
#endif

extern "C"
{
  API int multiply(int a, int b)
  {
    return a * b;
  }
}