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
#include <cstdint>
#include <mutex>

namespace flib
{
  class sync_guard
  {
  public:
    using duration_t = std::chrono::milliseconds;
    using level_t = uint64_t;

    inline sync_guard(void);
    inline sync_guard(const sync_guard&) = delete;
    inline sync_guard(sync_guard&&) = default;
    inline ~sync_guard(void) noexcept;
    inline sync_guard& operator=(const sync_guard&) = delete;
    inline sync_guard& operator=(sync_guard&&) = default;
    inline bool lock(level_t level = 1, duration_t timeout = duration_t(0));
    inline void reset(void);
    inline std::size_t size(void) const;
    inline void unlock(void);
    inline void unlock_all(void);

  private:
    std::atomic<level_t> m_level;
    std::atomic<std::size_t> m_locks;
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

bool flib::sync_guard::lock(level_t level, duration_t timeout)
{
  auto unlocked = [this, level]()
  {
    return level <= m_level;
  };
  std::mutex condition_mtx;
  std::unique_lock<decltype(condition_mtx)> condition_guard(condition_mtx);
  auto result = true;
  ++m_locks;
  if (duration_t(0) == timeout)
  {
    m_condition.wait(condition_guard, unlocked);
  }
  else
  {
    result = m_condition.wait_for(condition_guard, timeout, unlocked);
  }
  --m_locks;
  return result;
}

void flib::sync_guard::reset(void)
{
  m_level = 0x0000000000000000ull;
}

std::size_t flib::sync_guard::size(void) const
{
  return m_locks;
}

void flib::sync_guard::unlock(void)
{
  ++m_level;
  m_condition.notify_all();
}

void flib::sync_guard::sync_guard::unlock_all(void)
{
  m_level = 0xffffffffffffffffull;
  m_condition.notify_all();
}