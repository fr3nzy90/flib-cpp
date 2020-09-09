/*
* MIT License
*
* Copyright (c) 2019 Luka Arnecic
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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>

namespace flib
{
  class sync_guard
  {
  public:
    using duration_t = std::chrono::milliseconds;
    using level_t = uint64_t;
    using size_t = std::size_t;

    enum class result_t
    {
      normal,
      timeout
    };

    inline sync_guard(void);
    inline sync_guard(const sync_guard&) = delete;
    inline sync_guard(sync_guard&&) = delete;
    inline ~sync_guard(void) noexcept;
    inline sync_guard& operator=(const sync_guard&) = delete;
    inline sync_guard& operator=(sync_guard&&) = delete;
    inline level_t level(void) const;
    inline result_t lock(level_t level = 1, duration_t timeout = {});
    inline sync_guard& reset(void);
    inline size_t size(void) const;
    inline sync_guard& unlock(void);
    inline sync_guard& unlock_all(void);

  protected:
    std::atomic<level_t> m_level;
    std::atomic<size_t> m_locks;
    std::condition_variable m_condition;
  };
}

// IMPLEMENTATION

flib::sync_guard::sync_guard(void)
  : m_level(0x0000000000000000ull),
  m_locks(0)
{
}

flib::sync_guard::~sync_guard(void) noexcept
{
  unlock_all();
}

flib::sync_guard::level_t flib::sync_guard::level(void) const
{
  return m_level;
}

flib::sync_guard::result_t flib::sync_guard::lock(level_t level, duration_t timeout)
{
  auto unlocked = [this, level]
  {
    return level <= m_level;
  };
  std::mutex condition_mtx;
  std::unique_lock<decltype(condition_mtx)> condition_guard(condition_mtx);
  auto result = result_t::normal;
  ++m_locks;
  if (decltype(timeout){} == timeout)
  {
    m_condition.wait(condition_guard, unlocked);
  }
  else
  {
    result = m_condition.wait_for(condition_guard, timeout, unlocked) ? result_t::normal : result_t::timeout;
  }
  --m_locks;
  return result;
}

flib::sync_guard& flib::sync_guard::reset(void)
{
  m_level = 0x0000000000000000ull;
  return *this;
}

flib::sync_guard::size_t flib::sync_guard::size(void) const
{
  return m_locks;
}

flib::sync_guard& flib::sync_guard::unlock(void)
{
  ++m_level;
  m_condition.notify_all();
  return *this;
}

flib::sync_guard& flib::sync_guard::sync_guard::unlock_all(void)
{
  m_level = 0xffffffffffffffffull;
  m_condition.notify_all();
  return *this;
}