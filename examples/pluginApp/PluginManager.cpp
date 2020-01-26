#include "flib/PluginManager.hpp"

#include "PluginManager.hpp"

using namespace flib;

std::tuple<PluginManager::Creator, PluginManager::Deleter> PluginManager::GetPlugin(const std::string& filepath)
{
  return plugin::GetPlugin(filepath);
}