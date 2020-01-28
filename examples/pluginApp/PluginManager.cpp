#include "flib/Plugins.hpp"

#include "PluginManager.hpp"

using namespace flib;

std::tuple<PluginManager::Constructor, PluginManager::Destructor> PluginManager::GetPlugin(const std::string& filepath,
  const std::string& constructorName, const std::string& destructorName)
{
  return plugin::GetPlugin(filepath, constructorName, destructorName);
}