#include "plugin.hpp"

using namespace testing;

#if defined(_WIN32)
#  define API __declspec(dllexport)
#elif defined(__GNUC__)
#  define API __attribute__((visibility("default")))
#else
#  error "Unsupported compiler/platform"
#endif

extern "C" API void* create(void)
{
  return new plugin("plugin");
}

extern "C" API void destroy(void* ptr)
{
  delete static_cast<plugin*>(ptr);
}

extern "C" API int multiply(int a, int b)
{
  return a * b;
}

plugin::plugin(const std::string& module)
  : m_module(module)
{
}

std::string plugin::name(void) const
{
  return m_module;
}