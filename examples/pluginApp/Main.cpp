#include <iostream>

#include "flib/PluginManager.hpp"
#include "TestAPI.hpp"

using namespace flib;

int main()
{
#if defined(_WIN32)
  auto modulePath = "PluginLib";
#elif defined(__linux__)
  auto modulePath = "PluginLib.so";
#endif
  try
  {
    {
      PluginManager::CreateUniquePlugin<TestAPI>(modulePath)->SayHi();
    }
    {
      PluginManager::CreateSharedPlugin<TestAPI>(modulePath)->SayHi();
    }
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << '\n';
  }
  return 0;
}