#pragma once

#include <functional>
#include <memory>
#include <string>
#include <tuple>

// Wrapper
class PluginManager
{
public:
  using Creator = std::function<void* (void)>;
  using Deleter = std::function<void(void*)>;

  template<class T>
  static inline std::shared_ptr<T> CreateShared(const std::string& filepath);

  template<class T>
  static inline std::unique_ptr<T, Deleter> CreateUnique(const std::string& filepath);

private:
  static std::tuple<Creator, Deleter> GetPlugin(const std::string& filepath);
};

// IMPLEMENTATION

template<class T>
std::shared_ptr<T> PluginManager::CreateShared(const std::string& filepath)
{
  auto functions = GetPlugin(filepath);
  return std::shared_ptr<T>(static_cast<T*>(std::get<0>(functions)()), std::get<1>(functions));
}

template<class T>
std::unique_ptr<T, PluginManager::Deleter> PluginManager::CreateUnique(const std::string& filepath)
{
  auto functions = GetPlugin(filepath);
  return std::unique_ptr<T, Deleter>(static_cast<T*>(std::get<0>(functions)()), std::get<1>(functions));
}