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
  class SyncLock
  {
  public:
    using Duration = std::chrono::milliseconds;
    using Level = uint64_t;

    enum class ReleaseReason
    {
      NotLocked,
      Release,
      Timeout
    };

    inline SyncLock(void);
    inline SyncLock(const SyncLock&) = delete;
    inline SyncLock(SyncLock&&) = default;
    inline ~SyncLock(void) noexcept;
    inline SyncLock& operator=(const SyncLock&) = delete;
    inline SyncLock& operator=(SyncLock&&) = default;
    inline ReleaseReason Acquire(const Level level = 1, const Duration& timeout = Duration(0));
    inline std::size_t LockCount(void) const;
    inline void Release(void);
    inline void ReleaseAll(void);
    inline void Reset(void);

  private:
    std::atomic<Level> mLevel;
    std::atomic<std::size_t> mLocks;
    std::condition_variable mWaitCondition;
  };
}

// IMPLEMENTATION

flib::SyncLock::SyncLock(void)
  : mLevel(0x0000000000000000ull),
  mLocks(0)
{
}

flib::SyncLock::~SyncLock(void) noexcept
{
  ReleaseAll();
}

flib::SyncLock::ReleaseReason flib::SyncLock::Acquire(const Level level, const Duration& timeout)
{
  auto unlocked = [this, level]()
  {
    return level <= mLevel;
  };
  if (unlocked())
  {
    return ReleaseReason::NotLocked;
  }
  std::mutex waitLock;
  std::unique_lock<decltype(waitLock)> waitGuard(waitLock);
  auto result = ReleaseReason::Release;
  ++mLocks;
  if (Duration(0) == timeout)
  {
    mWaitCondition.wait(waitGuard, unlocked);
  }
  else
  {
    result = mWaitCondition.wait_for(waitGuard, timeout, unlocked) ? ReleaseReason::Release : ReleaseReason::Timeout;
  }
  --mLocks;
  return result;
}

std::size_t flib::SyncLock::LockCount(void) const
{
  return mLocks;
}

void flib::SyncLock::Release(void)
{
  ++mLevel;
  mWaitCondition.notify_all();
}

void flib::SyncLock::SyncLock::ReleaseAll(void)
{
  mLevel = 0xffffffffffffffffull;
  mWaitCondition.notify_all();
}

void flib::SyncLock::Reset(void)
{
  mLevel = 0x0000000000000000ull;
}