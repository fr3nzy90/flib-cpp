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
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace flib
{
  template<class T>
  class sync_queue
  {
  public:
    using duration_t = std::chrono::milliseconds;
    using element_t = T;
    using priority_t = uint8_t;
    using size_t = std::size_t;

    inline sync_queue(bool enabled = true);
    inline sync_queue(const sync_queue&) = delete;
    inline sync_queue(sync_queue&&) = delete;
    inline ~sync_queue(void) noexcept;
    inline sync_queue& operator=(const sync_queue&) = delete;
    inline sync_queue& operator=(sync_queue&&) = delete;
    inline sync_queue& clear(void);
    inline sync_queue& disable(void);
    inline bool empty(void) const;
    inline sync_queue& enable(void);
    inline bool enabled(void) const;
    inline element_t pop(duration_t timeout = {});
    inline element_t pop_raw(void);
    inline sync_queue& push(const element_t& object, priority_t priority = 0);
    inline sync_queue& push(element_t&& object, priority_t priority = 0);
    inline size_t size(void) const;

  protected:
    struct _extended_element_t
    {
      element_t element;
      priority_t priority;
    };

    std::atomic<bool> m_enabled;
    std::condition_variable m_condition;
    std::list<_extended_element_t> m_objects;
    mutable std::mutex m_objects_mtx;
  };
}

// IMPLEMENTATION

template<class T>
flib::sync_queue<T>::sync_queue(const bool enabled)
  : m_enabled(enabled)
{
}

template<class T>
flib::sync_queue<T>::~sync_queue(void) noexcept
{
  std::lock_guard<decltype(m_objects_mtx)> objects_guard(m_objects_mtx);
  m_objects.clear();
  m_enabled = false;
  m_condition.notify_all();
}

template<class T>
flib::sync_queue<T>& flib::sync_queue<T>::clear(void)
{
  std::lock_guard<decltype(m_objects_mtx)> objects_guard(m_objects_mtx);
  m_objects.clear();
  return *this;
}

template<class T>
flib::sync_queue<T>& flib::sync_queue<T>::disable(void)
{
  m_enabled = false;
  m_condition.notify_all();
  return *this;
}

template<class T>
bool flib::sync_queue<T>::empty(void) const
{
  std::lock_guard<decltype(m_objects_mtx)> objects_guard(m_objects_mtx);
  return m_objects.empty();
}

template<class T>
flib::sync_queue<T>& flib::sync_queue<T>::enable(void)
{
  m_enabled = true;
  return *this;
}

template<class T>
bool flib::sync_queue<T>::enabled(void) const
{
  return m_enabled;
}

template<class T>
typename flib::sync_queue<T>::element_t flib::sync_queue<T>::pop(duration_t timeout)
{
  auto timepoint = std::chrono::steady_clock::now() + timeout;
  auto condition_check = [this]
  {
    std::lock_guard<decltype(m_objects_mtx)> objects_guard(m_objects_mtx);
    return !m_objects.empty() || !m_enabled;
  };
  std::mutex condition_mtx;
  std::unique_lock<decltype(condition_mtx)> condition_guard(condition_mtx);
  std::unique_lock<decltype(m_objects_mtx)> objects_guard(m_objects_mtx, std::defer_lock);
  while (m_enabled)
  {
    objects_guard.lock();
    if (!m_objects.empty())
    {
      auto object = std::move(m_objects.front().element);
      m_objects.pop_front();
      return object;
    }
    objects_guard.unlock();
    if (decltype(timeout){} == timeout)
    {
      m_condition.wait(condition_guard, condition_check);
    }
    else if (!m_condition.wait_until(condition_guard, timepoint, condition_check))
    {
      throw std::runtime_error("Queue element retrieval has timed out");
    }
  }
  throw std::runtime_error("Queue is disabled");
}

template<class T>
typename flib::sync_queue<T>::element_t flib::sync_queue<T>::pop_raw(void)
{
  std::lock_guard<decltype(m_objects_mtx)> objects_guard(m_objects_mtx);
  if (m_objects.empty())
  {
    throw std::runtime_error("Queue is empty");
  }
  auto object = std::move(m_objects.front().element);
  m_objects.pop_front();
  return object;
}

template<class T>
flib::sync_queue<T>& flib::sync_queue<T>::push(const element_t& object, priority_t priority)
{
  return push(element_t(object), priority);
}

template<class T>
flib::sync_queue<T>& flib::sync_queue<T>::push(element_t&& object, priority_t priority)
{
  std::lock_guard<decltype(m_objects_mtx)> objects_guard(m_objects_mtx);
  m_objects.emplace(0 == priority ? m_objects.cend() : std::upper_bound(m_objects.cbegin(), m_objects.cend(), priority,
    [](decltype(priority) value, const typename decltype(m_objects)::value_type& element)
    {
      return value > element.priority;
    }), _extended_element_t{ std::move(object), priority });
  m_condition.notify_one();
  return *this;
}

template<class T>
typename flib::sync_queue<T>::size_t flib::sync_queue<T>::size(void) const
{
  std::lock_guard<decltype(m_objects_mtx)> objects_guard(m_objects_mtx);
  return static_cast<size_t>(m_objects.size());
}