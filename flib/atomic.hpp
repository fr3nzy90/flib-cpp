// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <utility>

namespace flib
{
#pragma region API
  template<class T>
  class atomic
  {
  public:
    using predicate_t = std::function<bool(const T&)>;
    using value_t = T;

  public:
    atomic(void) = default;
    explicit atomic(T p_desired);
    atomic(const atomic&) = delete;
    atomic(atomic&&) = delete;
    virtual ~atomic(void) noexcept;
    virtual T operator=(T p_desired);
    virtual atomic& operator=(const atomic&) = delete;
    virtual atomic& operator=(atomic&&) = delete;
    virtual operator T(void) const;
    virtual T exchange(T p_desired);
    virtual bool is_lock_free(void);
    virtual T load(void) const;
    virtual void notify_all(void);
    virtual void notify_one(void);
    virtual void store(T p_desired);
    virtual void wait(predicate_t p_predicate);
    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& p_duration, predicate_t p_predicate);
    template<class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& p_timepoint, predicate_t p_predicate);

  protected:
    virtual bool _condition_check(predicate_t p_predicate) const;

  protected:
    T m_value{};
    volatile bool m_destruct{ false };
    std::condition_variable m_condition;
    mutable std::mutex m_mtx;
  };
#pragma endregion

#pragma region IMPLEMENTATION
  template<class T>
  inline atomic<T>::atomic(T p_desired)
    : m_value(std::move(p_desired))
  {
  }

  template<class T>
  inline atomic<T>::~atomic(void) noexcept
  {
    m_destruct = true;
    m_condition.notify_all();
  }

  template<class T>
  inline T atomic<T>::operator=(T p_desired)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_value = p_desired;
    return p_desired;
  }

  template<class T>
  inline atomic<T>::operator T(void) const
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return m_value;
  }

  template<class T>
  inline T atomic<T>::exchange(T p_desired)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    auto value = std::move(m_value);
    m_value = std::move(p_desired);
    return value;
  }

  template<class T>
  inline bool atomic<T>::is_lock_free(void)
  {
    return false;
  }

  template<class T>
  inline T atomic<T>::load(void) const
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return m_value;
  }

  template<class T>
  inline void atomic<T>::notify_all(void)
  {
    m_condition.notify_all();
  }

  template<class T>
  inline void atomic<T>::notify_one(void)
  {
    m_condition.notify_one();
  }

  template<class T>
  inline void atomic<T>::store(T p_desired)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_value = std::move(p_desired);
  }

  template<class T>
  inline void atomic<T>::wait(predicate_t p_predicate)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_condition.wait(guard, std::bind(&atomic<T>::_condition_check, this, std::move(p_predicate)));
  }

  template<class T>
  template<class Rep, class Period>
  inline bool atomic<T>::wait_for(const std::chrono::duration<Rep, Period>& p_duration, predicate_t p_predicate)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return m_condition.wait_for(guard, p_duration, std::bind(&atomic<T>::_condition_check, this, std::move(p_predicate)));
  }

  template<class T>
  template<class Clock, class Duration>
  inline bool atomic<T>::wait_until(const std::chrono::time_point<Clock, Duration>& p_timepoint, predicate_t p_predicate)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return m_condition.wait_until(guard, p_timepoint, std::bind(&atomic<T>::_condition_check, this, std::move(p_predicate)));
  }

  template<class T>
  inline bool atomic<T>::_condition_check(predicate_t p_predicate) const
  {
    return m_destruct || p_predicate(m_value);
  }
#pragma endregion
}