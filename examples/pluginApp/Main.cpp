#include <iostream>

#include "PluginManager.hpp"
#include "TestAPI.hpp"

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
      PluginManager::CreateShared<TestAPI>(modulePath)->SayHi();
    }
    {
      PluginManager::CreateUnique<TestAPI>(modulePath)->SayHi();
    }
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << '\n';
  }
  return 0;
}