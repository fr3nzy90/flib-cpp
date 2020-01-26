/*
* MIT License
*
* Copyright (c) 2020 Luka Arnecic
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__linux__)
#  include <dlfcn.h>
#endif

namespace flib
{
  // Namespace for plugin management. It exposes methods for dynamically loading dynamically linked library (DLL) based
  // plugins. Such DLLs are required to export two functions, which enable construction and destruction of the plugin
  // object and must be of the following form:
  //
  //    extern "C" EXPORT_API void* CreatePluginInstance(void);
  //    extern "C" EXPORT_API void  DestroyPluginInstance(void* ptr);
  //
  // where EXPORT_API macro may be defined as follows:
  //
  //    #if defined(_MSC_VER)
  //    #  define EXPORT_API __declspec(dllexport)
  //    #elif defined(__GNUC__)
  //    #  define EXPORT_API __attribute__((visibility("default")))
  //    #else
  //    #  error "Unsupported compiler/platform"
  //    #endif
  namespace plugin
  {
    // Type definition plugin object constructor.
    using Creator = std::function<void* (void)>;

    // Type definition plugin object destructor (which also decrements DLL usage reference count).
    using Deleter = std::function<void(void*)>;

    // Function for loading DLL and retrieving plugin object constructor and destructor.
    // 
    // Notes:
    //   - DLLs maintains reference counter.
    //   - DLL will be loaded at first usage.
    //   - DLL will be automatically unloaded when last destructor is called.
    //   - Function will throw std::runtime_error exception if any occurs.
    //
    // Parameters:
    //   filepath - path to DLL
    //              Windows notes:
    //                - If the string specifies a full path, the function searches only that path for the module.
    //                - If the string specifies a relative path or a module name without a path, the function uses a
    //                  standard search strategy to find the module.
    //                - When specifying a path, be sure to use backslashes \, not forward slashes /.
    //                - If the string specifies a module name without a path and the file name extension is omitted,
    //                  the function appends the default library extension .dll to the module name. To prevent the
    //                  function from appending .dll to the module name, include a trailing point character . in the
    //                  module name string.
    //              Linux notes:
    //                - If filename contains a slash /, then it is interpreted as a (relative or absolute) pathname.
    //                  Otherwise, the dynamic linker searches for the library in the following order :
    //                    - (ELF only) Using the directories specified in the DT_RPATH dynamic section attribute of the
    //                      binary if present and DT_RUNPATH attribute does not exist. Use of DT_RPATH is deprecated.
    //                    - Using the environment variable LD_LIBRARY_PATH. Except if the executable is a
    //                      set-user-ID/set-group-ID binary, in which case it is ignored.
    //                    - (ELF only) Using the directories specified in the DT_RUNPATH dynamic section attribute of
    //                      the binary if present.
    //                    - From the cache file /etc/ld.so.cache, which contains a compiled list of candidate libraries
    //                      previously found in the augmented library path. If, however, the binary was linked with the
    //                      -z nodeflib linker option, libraries in the default library paths are skipped. Libraries
    //                      installed in hardware capability directories (see below) are preferred to other libraries.
    //                    - In the default path /lib, and then /usr/lib. If the binary was linked with the -z nodeflib
    //                      linker option, this step is skipped.
    //
    // Returns:
    //   Plugin object constructor and destructor.
    inline std::tuple<Creator, Deleter> GetPlugin(const std::string& filepath);

    // Helper function for creating shared smart pointer to plugin instance, which uses functions returned from
    // GetPlugin function for construction and destruction of the plugin object.
    // 
    // Notes:
    //   Same notes apply as for GetPlugin function.
    //
    // Template parameters:
    //   T - typename for casting of plugin instance. There is no checking if pointer can actually be casted.
    //
    // Parameters:
    //   filepath - path to DLL. Same notes apply as for GetPlugin function.
    //
    // Returns:
    //   Shared smart pointer to plugin instance.
    template<class T>
    inline std::shared_ptr<T> CreateShared(const std::string& filepath);

    // Helper function for creating unique smart pointer to plugin instance, which uses functions returned from
    // GetPlugin function for construction and destruction of the plugin object.
    // 
    // Notes:
    //   Same notes apply as for GetPlugin function.
    //
    // Template parameters:
    //   T - typename for casting of plugin instance. There is no checking if pointer can actually be casted.
    //
    // Parameters:
    //   filepath - path to DLL. Same notes apply as for GetPlugin function.
    //
    // Returns:
    //   Unique smart pointer to plugin instance.
    template<class T>
    inline std::unique_ptr<T, Deleter> CreateUnique(const std::string& filepath);
  }
}

// IMPLEMENTATION

std::tuple<flib::plugin::Creator, flib::plugin::Deleter> flib::plugin::GetPlugin(const std::string& filepath)
{
#if defined(_WIN32)
  auto moduleHandle = ::LoadLibraryA(filepath.c_str());
  if (!moduleHandle)
  {
    throw std::runtime_error("Module loading failed (code: " + std::to_string(::GetLastError()) + ")");
  }
  auto create = ::GetProcAddress(moduleHandle, "CreatePluginInstance");
  auto destroy = ::GetProcAddress(moduleHandle, "DestroyPluginInstance");
  if (!create || !destroy)
  {
    throw std::runtime_error("Module functions retrieval failed (code: " + std::to_string(::GetLastError()) + ")");
  }
  return {
    reinterpret_cast<void* (*)(void)>(create),
    [moduleHandle, destroy](void* ptr)
      {
        reinterpret_cast<void(*)(void*)>(destroy)(ptr);
        ::FreeLibrary(moduleHandle);
      }
  };
#elif defined(__linux__)
  auto moduleHandle = ::dlopen(filepath.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!moduleHandle)
  {
    throw std::runtime_error("Module loading failed (reason: \"" + std::string(::dlerror()) + "\")");
  }
  auto create = ::dlsym(moduleHandle, "CreatePluginInstance");
  auto destroy = ::dlsym(moduleHandle, "DestroyPluginInstance");
  if (!create || !destroy)
  {
    throw std::runtime_error("Module functions retrieval failed (reason: \"" + std::string(::dlerror()) + "\")");
  }
  return {
    reinterpret_cast<void* (*)(void)>(create),
    [moduleHandle, destroy](void* ptr)
      {
        reinterpret_cast<void(*)(void*)>(destroy)(ptr);
        ::dlclose(moduleHandle);
      }
  };
#else
#  error "Unsupported platform/compiler"
#endif
}

template<class T>
std::shared_ptr<T> flib::plugin::CreateShared(const std::string& filepath)
{
  auto functions = GetPlugin(filepath);
  return std::shared_ptr<T>(static_cast<T*>(std::get<0>(functions)()), std::get<1>(functions));
}

template<class T>
std::unique_ptr<T, flib::plugin::Deleter> flib::plugin::CreateUnique(const std::string& filepath)
{
  auto functions = GetPlugin(filepath);
  return std::unique_ptr<T, Deleter>(static_cast<T*>(std::get<0>(functions)()), std::get<1>(functions));
}