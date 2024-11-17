// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

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
    explicit atomic(T desired);
    atomic(const atomic&) = delete;
    atomic(atomic&&) = default;
    ~atomic(void) noexcept = default; // It is only safe to invoke the destructor if all threads have been notified. 
                                      // It is not required that they have exited their respective wait functions: some
                                      // threads may still be waiting to reacquire the associated lock, or may be
                                      // waiting to be scheduled to run after reacquiring it.
    T operator=(T desired);
    atomic& operator=(const atomic&) = delete;
    atomic& operator=(atomic&&) = default;
    operator T(void) const;
    T exchange(T desired);
    T load(void) const;
    void notify_all(void);
    void notify_one(void);
    void store(T desired);
    void wait(predicate_t predicate);
    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& duration, predicate_t predicate);
    template<class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& timeout, predicate_t predicate);

  private:
    struct _storage;

    bool _condition_check(predicate_t predicate) const;

    pimpl<_storage> m_storage;
  };

  // IMPLEMENTATION

  template<class T>
  struct atomic<T>::_storage
  {
    T value{};
    std::condition_variable condition;
    mutable std::mutex condition_mtx;
  };

  template<class T>
  inline atomic<T>::atomic(T desired)
  {
    m_storage->value = std::move(desired);
  }

  template<class T>
  inline T atomic<T>::operator=(T desired)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->value = desired;
    return desired;
  }

  template<class T>
  inline atomic<T>::operator T(void) const
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return m_storage->value;
  }

  template<class T>
  inline T atomic<T>::exchange(T desired)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    auto value = std::move(m_storage->value);
    m_storage->value = std::move(desired);
    return value;
  }

  template<class T>
  inline T atomic<T>::load(void) const
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return m_storage->value;
  }

  template<class T>
  inline void atomic<T>::notify_all(void)
  {
    m_storage->condition.notify_all();
  }

  template<class T>
  inline void atomic<T>::notify_one(void)
  {
    m_storage->condition.notify_one();
  }

  template<class T>
  inline void atomic<T>::store(T desired)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->value = std::move(desired);
  }

  template<class T>
  inline void atomic<T>::wait(predicate_t predicate)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->condition.wait(condition_guard, std::bind(&atomic<T>::_condition_check, this, std::move(predicate)));
  }

  template<class T>
  template<class Rep, class Period>
  inline bool atomic<T>::wait_for(const std::chrono::duration<Rep, Period>& duration, predicate_t predicate)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return m_storage->condition.wait_for(condition_guard, duration,
      std::bind(&atomic<T>::_condition_check, this, std::move(predicate)));
  }

  template<class T>
  template<class Clock, class Duration>
  inline bool atomic<T>::wait_until(const std::chrono::time_point<Clock, Duration>& timeout, predicate_t predicate)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return m_storage->condition.wait_until(condition_guard, timeout,
      std::bind(&atomic<T>::_condition_check, this, std::move(predicate)));
  }

  template<class T>
  inline bool atomic<T>::_condition_check(predicate_t predicate) const
  {
    return predicate(m_storage->value);
  }
}