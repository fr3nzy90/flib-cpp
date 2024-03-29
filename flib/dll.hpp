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
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__linux__)
#  include <dlfcn.h>
#endif

namespace flib
{
  inline namespace dll
  {
    // Type definitions for DLL handle
    using dll_t = void*;

    // Function for loading DLL
    // 
    // Notes:
    //   - DLLs maintains reference counter and will be loaded when it first increases from 0
    //   - Function will throw std::runtime_error exception, if library loading fails
    //
    // Parameters:
    //   filepath - path to DLL
    //              Windows notes:
    //                - If the string specifies a full path, the function searches only that path for the  module
    //                - If the string specifies a relative path or a module name without a path, the function uses a
    //                  standard search strategy to find the module
    //                - When specifying a path, be sure to use backslashes \, not forward slashes /
    //                - If the string specifies a module name without a path and the file name extension is omitted, the
    //                  function appends the default library extension .dll to the module name. To prevent the function
    //                  from appending .dll to the module name, include a trailing point character . in the module name
    //                  string
    //              Linux notes:
    //                - If filename contains a slash /, then it is interpreted as a (relative or absolute) pathname.
    //                  Otherwise, the dynamic linker searches for the library in the following order:
    //                    - (ELF only) Using the directories specified in the DT_RPATH dynamic section attribute of
    //                      the binary if present and DT_RUNPATH attribute does not exist. Use of DT_RPATH is deprecated
    //                    - Using the environment variable LD_LIBRARY_PATH. Except if the executable is a
    //                      set-user-ID/set-group-ID binary, in which case it is ignored
    //                    - (ELF only) Using the directories specified in the DT_RUNPATH dynamic section attribute of the
    //                      binary if present
    //                    - From the cache file /etc/ld.so.cache, which contains a compiled list of candidate libraries
    //                      previously found in the augmented library path. If, however, the binary was linked with the
    //                      -z nodeflib linker option, libraries in the default library paths are skipped. Libraries
    //                      installed in hardware capability directories are preferred to other libraries
    //                    - In the default path /lib, and then /usr/lib. If the binary was linked with the -z nodeflib
    //                      linker option, this step is skipped
    //
    // Returns:
    //   DLL handle
    dll_t open_library(const std::string& filepath);

    // Function for unloading DLL
    // 
    // Notes:
    //   - DLLs maintains reference counter and will be unloaded when reference count reaches 0
    //   - Function will throw std::invalid_argument exception if invalid DLL handle is given
    //
    // Parameters:
    //   handle - DLL handle
    void close_library(dll_t handle);

    // Function for retrieving functions defined in dll
    // 
    // Notes:
    //   - DLLs maintains reference counter and will be unloaded when reference count reaches 0
    //   - Function will throw std::invalid_argument exception if invalid DLL handle is given
    //   - Function will throw std::runtime_error exception if library function retrieval fails
    //
    // Template parameters:
    //   T - typename for casting of function handle. There is no checking if pointer can actually be casted
    //
    // Parameters:
    //   handle - DLL handle
    //     name - name of the exported function
    //
    // Returns:
    //   retrieved function
    template<class T>
    std::function<T> get_library_function(dll_t handle, const std::string& name);

    // IMPLEMENTATION

    inline dll_t open_library(const std::string& filepath)
    {
#if defined(_WIN32)
      auto handle = ::LoadLibraryA(filepath.c_str());
      if (!handle)
      {
        throw std::runtime_error("Module loading failed (code: " + std::to_string(::GetLastError()) + ")");
      }
      return handle;
#elif defined(__linux__)
      auto handle = ::dlopen(filepath.c_str(), RTLD_NOW | RTLD_GLOBAL);
      if (!handle)
      {
        throw std::runtime_error("Module loading failed (reason: \"" + std::string(::dlerror()) + "\")");
      }
      return handle;
#else
#  error "Unsupported platform/compiler"
#endif
    }

    inline void close_library(dll_t handle)
    {
#if defined(_WIN32)
      auto module_handle = static_cast<HMODULE>(handle);
      if (!module_handle)
      {
        throw std::invalid_argument("Invalid module handle");
      }
      ::FreeLibrary(module_handle);
#elif defined(__linux__)
      if (!handle)
      {
        throw std::invalid_argument("Invalid module handle");
      }
      ::dlclose(handle);
#else
#  error "Unsupported platform/compiler"
#endif
    }

    template<class T>
    inline std::function<T> get_library_function(dll_t handle, const std::string& name)
    {
#if defined(_WIN32)
      auto module_handle = static_cast<HMODULE>(handle);
      if (!module_handle)
      {
        throw std::invalid_argument("Invalid module handle");
      }
      auto function_handle = ::GetProcAddress(module_handle, name.c_str());
      if (!function_handle)
      {
        throw std::runtime_error("Function retrieval failed (code: " + std::to_string(::GetLastError()) + ")");
      }
      return { reinterpret_cast<T*>(function_handle) };
#elif defined(__linux__)
      if (!handle)
      {
        throw std::invalid_argument("Invalid module handle");
      }
      auto function_handle = ::dlsym(handle, name.c_str());
      if (!function_handle)
      {
        throw std::runtime_error("Function retrieval failed (reason: \"" + std::string(::dlerror()) + "\")");
      }
      return { reinterpret_cast<T*>(function_handle) };
#else
#  error "Unsupported platform/compiler"
#endif
    }
  }
}