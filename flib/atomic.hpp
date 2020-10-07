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

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <utility>

#include <flib/pimpl.hpp>

namespace flib
{
  template<class T>
  class atomic
  {
  public:
    using predicate_t = std::function<bool(const T&)>;
    using value_t = T;

    atomic(void) = default;
    explicit inline atomic(T desired);
    atomic(const atomic&) = delete;
    atomic(atomic&&) = default;
    ~atomic(void) noexcept = default; // It is only safe to invoke the destructor if all threads have been notified. 
                                      // It is not required that they have exited their respective wait functions: some
                                      // threads may still be waiting to reacquire the associated lock, or may be
                                      // waiting to be scheduled to run after reacquiring it.
    inline T operator=(T desired);
    atomic& operator=(const atomic&) = delete;
    atomic& operator=(atomic&&) = default;
    inline operator T(void) const;
    inline T exchange(T desired);
    inline T load(void) const;
    inline void notify_all(void);
    inline void notify_one(void);
    inline void store(T desired);
    inline void wait(predicate_t predicate);
    template<class Rep, class Period>
    inline bool wait_for(const std::chrono::duration<Rep, Period>& duration, predicate_t predicate);
    template<class Clock, class Duration>
    inline bool wait_until(const std::chrono::time_point<Clock, Duration>& timeout, predicate_t predicate);

  private:
    class _impl;
    pimpl<_impl> m_impl;
  };

  // IMPLEMENTATION

  template<class T>
  class atomic<T>::_impl
  {
  public:
    _impl(void) = default;
    inline _impl(T desired);
    inline T _exchange(T desired);
    inline T _load(void) const;
    inline void _notify_all(void);
    inline void _notify_one(void);
    inline void _store(T desired);
    inline void _wait(predicate_t predicate);
    template<class Rep, class Period>
    inline bool _wait_for(const std::chrono::duration<Rep, Period>& duration, predicate_t predicate);
    template<class Clock, class Duration>
    inline bool _wait_until(const std::chrono::time_point<Clock, Duration>& timeout, predicate_t predicate);

  private:
    inline bool __condition_check(predicate_t predicate) const;

    T m_value;
    std::condition_variable m_condition;
    mutable std::mutex m_condition_mtx;
  };

  template<class T>
  atomic<T>::_impl::_impl(T desired)
    : m_value(std::move(desired))
  {
  }

  template<class T>
  T atomic<T>::_impl::_exchange(T desired)
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    auto value = std::move(m_value);
    m_value = desired;
    return value;
  }

  template<class T>
  T atomic<T>::_impl::_load(void) const
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    return m_value;
  }

  template<class T>
  void atomic<T>::_impl::_notify_all(void)
  {
    m_condition.notify_all();
  }

  template<class T>
  void atomic<T>::_impl::_notify_one(void)
  {
    m_condition.notify_one();
  }

  template<class T>
  void atomic<T>::_impl::_store(T desired)
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    m_value = desired;
  }

  template<class T>
  void atomic<T>::_impl::_wait(predicate_t predicate)
  {
    std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
    m_condition.wait(condition_guard, std::bind(&_impl::__condition_check, this, std::move(predicate)));
  }

  template<class T>
  template<class Rep, class Period>
  bool atomic<T>::_impl::_wait_for(const std::chrono::duration<Rep, Period>& duration, predicate_t predicate)
  {
    std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
    return m_condition.wait_for(condition_guard, duration,
      std::bind(&_impl::__condition_check, this, std::move(predicate)));
  }

  template<class T>
  template<class Clock, class Duration>
  bool atomic<T>::_impl::_wait_until(const std::chrono::time_point<Clock, Duration>& timeout, predicate_t predicate)
  {
    std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
    return m_condition.wait_until(condition_guard, timeout,
      std::bind(&_impl::__condition_check, this, std::move(predicate)));
  }

  template<class T>
  bool atomic<T>::_impl::__condition_check(predicate_t predicate) const
  {
    return predicate(m_value);
  }

  template<class T>
  atomic<T>::atomic(T desired)
    : m_impl(std::move(desired))
  {
  }

  template<class T>
  T atomic<T>::operator=(T desired)
  {
    m_impl->_store(desired);
    return desired;
  }

  template<class T>
  atomic<T>::operator T(void) const
  {
    return m_impl->_load();
  }

  template<class T>
  T atomic<T>::exchange(T desired)
  {
    return m_impl->_exchange(std::move(desired));
  }

  template<class T>
  T atomic<T>::load(void) const
  {
    return m_impl->_load();
  }

  template<class T>
  void atomic<T>::notify_all(void)
  {
    m_impl->_notify_all();
  }

  template<class T>
  void atomic<T>::notify_one(void)
  {
    m_impl->_notify_one();
  }

  template<class T>
  void atomic<T>::store(T desired)
  {
    m_impl->_store(std::move(desired));
  }

  template<class T>
  void atomic<T>::wait(predicate_t predicate)
  {
    m_impl->_wait(std::move(predicate));
  }

  template<class T>
  template<class Rep, class Period>
  bool atomic<T>::wait_for(const std::chrono::duration<Rep, Period>& duration, predicate_t predicate)
  {
    return m_impl->_wait_for(duration, std::move(predicate));
  }

  template<class T>
  template<class Clock, class Duration>
  bool atomic<T>::wait_until(const std::chrono::time_point<Clock, Duration>& timeout, predicate_t predicate)
  {
    return m_impl->_wait_until(timeout, std::move(predicate));
  }
}