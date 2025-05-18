// Copyright © 2020-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <utility>

namespace flib
{
#pragma region API
  class atomic_base
  {
  public:
    // Clock definition - enforced monotonic steady_clock
    using clock_t = std::chrono::steady_clock;
    // Duration type definition - enforced minimum us duration unit
    using duration_t = std::chrono::microseconds;

  protected:
    atomic_base(void) = default;
  };

  template<class T>
  class atomic
    : public atomic_base
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
    virtual bool wait(predicate_t p_predicate);
    virtual bool wait_for(const duration_t& p_duration, predicate_t p_predicate);
    virtual bool wait_until(const clock_t::time_point& p_timepoint, predicate_t p_predicate);

  protected:
    using _mutex_t = std::mutex;
    using _waiting_strategy_t = std::function<bool(std::unique_lock<_mutex_t>&)>;

  protected:
    virtual bool _condition_check(predicate_t p_predicate) const;
    virtual bool _wait(_waiting_strategy_t p_strategy);

  protected:
    T m_value{};
    bool m_destruct{ false };
    uint64_t m_wait_count{ 0ull };
    std::condition_variable m_condition;
    mutable _mutex_t m_mtx;
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
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_destruct = true;
    guard.unlock();
    m_condition.notify_all();
    guard.lock();
    m_condition.wait(guard, [this] { return 0ull == m_wait_count; });
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
  inline bool atomic<T>::wait(predicate_t p_predicate)
  {
    return _wait([this, predicate = std::move(p_predicate)](std::unique_lock<decltype(m_mtx)>& p_guard)
      {
        m_condition.wait(p_guard, std::bind(&atomic<T>::_condition_check, this, std::move(predicate)));
        return predicate(m_value);
      });
  }

  template<class T>
  inline bool atomic<T>::wait_for(const duration_t& p_duration, predicate_t p_predicate)
  {
    return _wait([this, &p_duration, predicate = std::move(p_predicate)](std::unique_lock<decltype(m_mtx)>& p_guard)
      {
        return m_condition.wait_for(p_guard, p_duration, std::bind(&atomic<T>::_condition_check, this, std::move(predicate)))
          && predicate(m_value);
      });
  }

  template<class T>
  inline bool atomic<T>::wait_until(const clock_t::time_point& p_timepoint, predicate_t p_predicate)
  {
    return _wait([this, &p_timepoint, predicate = std::move(p_predicate)](std::unique_lock<decltype(m_mtx)>& p_guard)
      {
        return m_condition.wait_until(p_guard, p_timepoint, std::bind(&atomic<T>::_condition_check, this, std::move(predicate)))
          && predicate(m_value);
      });
  }

  template<class T>
  inline bool atomic<T>::_condition_check(predicate_t p_predicate) const
  {
    return m_destruct || p_predicate(m_value);
  }

  template<class T>
  inline bool atomic<T>::_wait(_waiting_strategy_t p_strategy)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    ++m_wait_count;
    bool result = p_strategy(guard);
    --m_wait_count;
    guard.unlock();
    m_condition.notify_all();
    return result;
  }
#pragma endregion
}