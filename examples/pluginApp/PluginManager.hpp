#pragma once

#include <functional>
#include <memory>
#include <string>
#include <tuple>

// Wrapper
class PluginManager
{
public:
  using Constructor = std::function<void* (void)>;
  using Destructor = std::function<void(void*)>;

  static constexpr auto defaultConstructorName = "CreatePluginInstance";
  static constexpr auto defaultDestructorName = "DestroyPluginInstance";

  template<class T>
  static inline std::shared_ptr<T> CreateShared(const std::string& filepath,
    const std::string& constructorName = defaultConstructorName,
    const std::string& destructorName = defaultDestructorName);

  template<class T>
  static inline std::unique_ptr<T, Destructor> CreateUnique(const std::string& filepath,
    const std::string& constructorName = defaultConstructorName,
    const std::string& destructorName = defaultDestructorName);

private:
  static std::tuple<Constructor, Destructor> GetPlugin(const std::string& filepath, const std::string& constructorName,
    const std::string& destructorName);
};

// IMPLEMENTATION

template<class T>
std::shared_ptr<T> PluginManager::CreateShared(const std::string& filepath, const std::string& constructorName,
  const std::string& destructorName)
{
  auto functions = GetPlugin(filepath, constructorName, destructorName);
  return std::shared_ptr<T>(static_cast<T*>(std::get<0>(functions)()), std::get<1>(functions));
}

template<class T>
std::unique_ptr<T, PluginManager::Destructor> PluginManager::CreateUnique(const std::string& filepath,
  const std::string& constructorName, const std::string& destructorName)
{
  auto functions = GetPlugin(filepath, constructorName, destructorName);
  return std::unique_ptr<T, Destructor>(static_cast<T*>(std::get<0>(functions)()), std::get<1>(functions));
}