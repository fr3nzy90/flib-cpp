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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>
#include <stdexcept>
#include <tuple>

namespace flib
{
  template<class T>
  class WaitQueue
  {
  public:
    using Duration = std::chrono::milliseconds;
    using Element = T;
    using Priority = uint8_t;

    inline WaitQueue(const bool enabled = true);
    inline WaitQueue(const WaitQueue&) = delete;
    inline WaitQueue(WaitQueue&&) = default;
    inline ~WaitQueue(void) noexcept;
    inline WaitQueue& operator=(const WaitQueue&) = delete;
    inline WaitQueue& operator=(WaitQueue&&) = default;
    inline void Clear(void);
    inline void Disable(void);
    inline void Enable(void);
    inline std::size_t ObjectCount(void) const;
    inline bool IsEmpty(void) const;
    inline bool IsEnabled(void) const;
    inline T Pop(void);
    inline void Push(const T& object, const Priority priority = 0);
    inline void Push(T&& object, const Priority priority = 0);
    inline T WaitedPop(const Duration& timeout = Duration(0));

  private:
    std::atomic_bool mEnabled;
    std::condition_variable mWaitCondition;
    mutable std::recursive_mutex mObjectsAccessLock;
    std::list<std::tuple<T, Priority>> mObjects;
  };
}

// IMPLEMENTATION

template<class T>
flib::WaitQueue<T>::WaitQueue(const bool enabled)
  : mEnabled(enabled)
{
}

template<class T>
flib::WaitQueue<T>::~WaitQueue(void) noexcept
{
  std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
  mObjects.clear();
  mEnabled = false;
  mWaitCondition.notify_all();
}

template<class T>
void flib::WaitQueue<T>::Clear(void)
{
  std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
  mObjects.clear();
}

template<class T>
void flib::WaitQueue<T>::Disable(void)
{
  mEnabled = false;
  mWaitCondition.notify_all();
}

template<class T>
void flib::WaitQueue<T>::Enable(void)
{
  mEnabled = true;
}

template<class T>
std::size_t flib::WaitQueue<T>::ObjectCount(void) const
{
  std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
  return mObjects.size();
}

template<class T>
bool flib::WaitQueue<T>::IsEmpty(void) const
{
  std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
  return mObjects.empty();
}

template<class T>
bool flib::WaitQueue<T>::IsEnabled(void) const
{
  return mEnabled;
}

template<class T>
T flib::WaitQueue<T>::Pop(void)
{
  std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
  if (mObjects.empty())
  {
    throw std::runtime_error("WaitQueue is empty");
  }
  auto object = std::move(std::get<0>(mObjects.front()));
  mObjects.pop_front();
  return object;
}

template<class T>
void flib::WaitQueue<T>::Push(const T& object, const Priority priority)
{
  std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
  mObjects.emplace(0 == priority ? mObjects.cend() : std::upper_bound(mObjects.cbegin(), mObjects.cend(), priority,
    [](const decltype(priority)& value, const typename decltype(mObjects)::value_type& element)
    {
      return value > std::get<1>(element);
    }), object, priority);
  mWaitCondition.notify_one();
}

template<class T>
void flib::WaitQueue<T>::Push(T&& object, const Priority priority)
{
  std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
  mObjects.emplace(0 == priority ? mObjects.cend() : std::upper_bound(mObjects.cbegin(), mObjects.cend(), priority,
    [](const decltype(priority)& value, const typename decltype(mObjects)::value_type& element)
    {
      return value > std::get<1>(element);
    }), std::move(object), priority);
  mWaitCondition.notify_one();
}

template<class T>
T flib::WaitQueue<T>::WaitedPop(const Duration& timeout)
{
  auto timepoint = std::chrono::high_resolution_clock::now() + timeout;
  auto notEmpty = [this]()
  {
    std::lock_guard<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock);
    return !mObjects.empty() || !mEnabled;
  };
  std::mutex waitLock;
  std::unique_lock<decltype(waitLock)> waitGuard(waitLock);
  std::unique_lock<decltype(mObjectsAccessLock)> objectsAccessGuard(mObjectsAccessLock, std::defer_lock);
  while (mEnabled)
  {
    if (Duration(0) == timeout)
    {
      mWaitCondition.wait(waitGuard, notEmpty);
    }
    else
    {
      if (!mWaitCondition.wait_until(waitGuard, timepoint, notEmpty))
      {
        throw std::runtime_error("WaitQueue element retrieval has timed out");
      }
    }
    objectsAccessGuard.lock();
    if (!mObjects.empty())
    {
      auto object = std::move(std::get<0>(mObjects.front()));
      mObjects.pop_front();
      return object;
    }
    objectsAccessGuard.unlock();
  }
  throw std::runtime_error("WaitQueue is disabled");
}