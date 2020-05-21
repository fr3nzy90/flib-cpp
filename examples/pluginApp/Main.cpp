#include <functional>
#include <iostream>

#include "PluginManager.hpp"
#include "TestAPI.hpp"

void Execute(const std::function<void(void)>& func)
{
  try
  {
    func();
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << '\n';
  }
}

int main()
{
#if defined(_WIN32)
  auto modulePath = "PluginLib";
#elif defined(__linux__)
  auto modulePath = "PluginLib.so";
#endif
  Execute([modulePath]()
    {
      PluginManager::CreateShared<TestAPI>(modulePath)->SayHi();
    }
  );
  Execute([modulePath]()
    {
      PluginManager::CreateUnique<TestAPI>(modulePath)->SayHi();
    }
  );
  Execute([modulePath]()
    {
      PluginManager::CreateUnique<TestAPI>(modulePath, "CreatePluginInstance2")->SayHi();
    }
  );
  Execute([modulePath]()
    {
      PluginManager::CreateUnique<TestAPI>(modulePath, "CreatePluginInstance2", "DestroyPluginInstance2")->SayHi();
    }
  );
  Execute([modulePath]()
    {
      PluginManager::CreateUnique<TestAPI>(modulePath, "InvalidFunction")->SayHi();
    }
  );
  Execute([modulePath]()
    {
      PluginManager::CreateUnique<TestAPI>(modulePath, "CreatePluginInstance", "InvalidFunction")->SayHi();
    }
  );
  return 0;
}