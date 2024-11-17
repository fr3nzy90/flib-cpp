// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__linux__)
#  include <dlfcn.h>
#endif

namespace flib
{
#pragma region API
  class dll
  {
    // Please refer to the following documentation for DLL (un)loading behaviour, file searching algorithm and/or DLL module loading flags:
    //   - for Windows platform refer to LoadLibraryExA function in libloaderapi.h header
    //     (https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexa)
    //   - for Linux platform refer to dlopen function in dlfcn.h header
    //     (https://linux.die.net/man/3/dlopen)

  public:
    static constexpr int64_t s_default_flags =
#if defined(_WIN32)      // Windows
      NULL;
#elif defined(__linux__) // Linux
      RTLD_NOW | RTLD_GLOBAL;
#else
#  error "Unsupported platform/compiler"
#endif

  public:
    dll(void) = default;
    dll(const std::string& p_filepath, int64_t p_flags = s_default_flags);
    dll(const dll&) = delete;
    dll(dll&&) = delete;
    ~dll(void);
    dll& operator=(const dll&) = delete;
    dll& operator=(dll&&) = delete;
    std::string filepath(void) const;
    int64_t flags(void) const;
    template<class T>
    std::function<T> get_function(const std::string& p_name) const;
    void load(const std::string& p_filepath, int64_t p_flags = s_default_flags);
    bool loaded(void) const;
    void unload(void);

  private:
    template<class T>
    std::function<T> _get_function_unchecked(const std::string& p_name) const;
    void _load_unchecked(const std::string& p_filepath, int64_t p_flags);
    void _unload_unchecked(void);

  private:
#if defined(_WIN32)
    HMODULE m_handle_win{ NULL };
#elif defined(__linux__)
    void* m_handle_unix{ nullptr };
#else
#  error "Unsupported platform/compiler"
#endif
    std::string m_filepath;
    int64_t m_flags{ 0ll };
    bool m_loaded{ false };
  };
#pragma endregion

#pragma region IMPLEMENTATION
  inline dll::dll(const std::string& p_filepath, int64_t p_flags)
  {
    _load_unchecked(p_filepath, p_flags);
  }

  inline dll::~dll(void)
  {
    if (!loaded())
    {
      return;
    }
    _unload_unchecked();
  }

  inline std::string dll::filepath(void) const
  {
    return m_filepath;
  }

  inline int64_t dll::flags(void) const
  {
    return m_flags;
  }

  template<class T>
  inline std::function<T> dll::get_function(const std::string& p_name) const
  {
    if (!loaded())
    {
      throw std::runtime_error("DLL not loaded");
    }
    return _get_function_unchecked<T>(p_name);
  }

  inline void dll::load(const std::string& p_filepath, int64_t p_flags)
  {
    if (loaded())
    {
      throw std::runtime_error("DLL already loaded");
    }
    _load_unchecked(p_filepath, std::move(p_flags));
  }

  inline bool dll::loaded(void) const
  {
    return m_loaded;
  }

  inline void dll::unload(void)
  {
    if (!loaded())
    {
      return;
    }
    _unload_unchecked();
  }

  template<class T>
  inline std::function<T>  dll::_get_function_unchecked(const std::string& p_name) const
  {
#if defined(_WIN32)
    assert(m_handle_win);
    auto result = ::GetProcAddress(m_handle_win, p_name.c_str());
    if (NULL == result)
    {
      throw std::runtime_error("Function retrieval from module failed (code: " + std::to_string(::GetLastError()) + ")");
    }
    return { reinterpret_cast<T*>(result) };
#elif defined(__linux__)
    assert(m_handle_unix);
    ::dlerror();
    auto result = ::dlsym(m_handle_unix, p_name.c_str());
    auto error = ::dlerror();
    if (NULL != error)
    {
      throw std::runtime_error("Function retrieval from module failed (reason: \"" + std::string(error) + "\")");
    }
    return NULL != result ? std::function<T>{ reinterpret_cast<T*>(result) } : std::function<T>{};
#else
#  error "Unsupported platform/compiler"
#endif
  }

  inline void dll::_load_unchecked(const std::string& p_filepath, int64_t p_flags)
  {
    m_filepath = p_filepath;
    m_flags = p_flags;
#if defined(_WIN32)
    m_handle_win = ::LoadLibraryExA(p_filepath.c_str(), NULL, static_cast<DWORD>(p_flags));
    if (NULL == m_handle_win)
    {
      throw std::runtime_error("Module loading failed (code: " + std::to_string(::GetLastError()) + ")");
    }
#elif defined(__linux__)
    ::dlerror();
    m_handle_unix = ::dlopen(p_filepath.c_str(), static_cast<int>(p_flags));
    auto error = ::dlerror();
    if (NULL == m_handle_unix)
    {
      throw std::runtime_error("Module loading failed (reason: \"" + std::string(NULL != error ? error : "unknown") + "\")");
    }
#else
#  error "Unsupported platform/compiler"
#endif
    m_loaded = true;
  }

  inline void dll::_unload_unchecked(void)
  {
#if defined(_WIN32)
    assert(m_handle_win);
    auto result = ::FreeLibrary(m_handle_win);
    if (0 == result)
    {
      throw std::runtime_error("Module unloading failed (code: " + std::to_string(::GetLastError()) + ")");
    }
#elif defined(__linux__)
    assert(m_handle_unix);
    ::dlerror();
    auto result = ::dlclose(m_handle_unix);
    auto error = ::dlerror();
    if (0 != result)
    {
      throw std::runtime_error("Module unloading failed (reason: \"" + std::string(NULL != error ? error : "unknown") + "\")");
    }
#else
#  error "Unsupported platform/compiler"
#endif
    m_loaded = false;
  }
#pragma endregion
}