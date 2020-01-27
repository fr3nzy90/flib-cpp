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
  return new TestClass("CreatePluginInstance");
}

extern "C" API void* CreatePluginInstance2(void)
{
  return new TestClass("CreatePluginInstance2");
}

extern "C" API void DestroyPluginInstance(void* ptr)
{
  delete static_cast<TestClass*>(ptr);
}

extern "C" API void DestroyPluginInstance2(void* ptr)
{
  delete static_cast<TestClass*>(ptr);
}

TestClass::TestClass(const std::string& name)
  : mName(name)
{
  std::cout << "TestClass via \"" + mName + "\"\n";
}

TestClass::~TestClass(void)
{
  std::cout << "~TestClass via \"" + mName + "\"\n";
}

void TestClass::SayHi(void)
{
  std::cout << "SayHi via \"" + mName + "\"\n";
}