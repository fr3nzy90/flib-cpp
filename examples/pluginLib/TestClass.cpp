#include <iostream>

#include "TestClass.hpp"

#if defined(_WIN32)
#  define API __declspec(dllexport)
#elif defined(__GNUC__)
#  define API __attribute__((visibility("default")))
#else
#  error "Unsupported compiler/platform"
#endif

extern "C" API void* CreatePluginInstance(void)
{
  return new TestClass();
}

extern "C" API void DestroyPluginInstance(void* ptr)
{
  delete static_cast<TestClass*>(ptr);
}

TestClass::TestClass(void)
{
  std::cout << "TestClass\n";
}

TestClass::~TestClass(void)
{
  std::cout << "~TestClass\n";
}

void TestClass::SayHi(void)
{
  std::cout << "Hello\n";
}