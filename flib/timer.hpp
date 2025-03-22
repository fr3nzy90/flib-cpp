// Copyright © 2019-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <utility>

namespace flib
{
#pragma region API
  class timer
  {
  public:
    using duration_t = std::chrono::nanoseconds;
    using event_t = std::function<void(void)>;

    enum class type
    {
      fixed_delay,
      fixed_rate
    };

  public:
    timer(void) = default;
    timer(const timer&) = delete;
    timer(timer&&) = delete;
    ~timer(void) noexcept;
    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = delete;
    void clear(void);
    void reschedule(void);
    void schedule(event_t p_event, duration_t p_delay, duration_t p_period = {}, type p_type = type::fixed_delay);
    bool scheduled(void) const;

  private:
    using _clock_t = std::chrono::steady_clock;

    enum class _state
    {
      activating,
      active,
      destruct
    };

    struct _executor;

  private:
    bool _condition_check(void) const;
    void _init(_executor& p_executor);
    void _wait(_executor& p_executor);
    void _run(_executor& p_executor);

  private:
    event_t m_event;
    duration_t m_delay{};
    duration_t m_period{};
    type m_type{ type::fixed_delay };
    _state m_state{ _state::destruct };
    _clock_t::time_point m_event_time;
    std::unique_ptr<_executor> m_executor{ std::make_unique<timer::_executor>() };
    std::condition_variable m_condition;
    mutable std::mutex m_condition_mtx;
  };
#pragma endregion

#pragma region IMPLEMENTATION
  struct timer::_executor
  {
    bool m_running{ false };
    std::future<void> m_result;
  };

  inline timer::~timer(void) noexcept
  {
    clear();
    _wait(*m_executor);
  }

  inline void timer::clear(void)
  {
    std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
    m_state = _state::destruct;
    condition_guard.unlock();
    m_condition.notify_all();
  }

  inline void timer::reschedule(void)
  {
    std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
    if (!m_event)
    {
      return;
    }
    m_event_time = _clock_t::now() + m_delay;
    m_state = _state::activating;
    _init(*m_executor);
    condition_guard.unlock();
    m_condition.notify_all();
  }

  inline void timer::schedule(event_t p_event, duration_t p_delay, duration_t p_period, type p_type)
  {
    if (!p_event)
    {
      return;
    }
    std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
    m_event = std::move(p_event);
    m_delay = std::move(p_delay);
    m_period = std::move(p_period);
    m_type = std::move(p_type);
    m_event_time = _clock_t::now() + m_delay;
    m_state = _state::activating;
    _init(*m_executor);
    condition_guard.unlock();
    m_condition.notify_all();
  }

  inline bool timer::scheduled(void) const
  {
    std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
    return _state::active == m_state || _state::activating == m_state;
  }

  inline bool timer::_condition_check(void) const
  {
    return _state::active != m_state;
  }

  inline void timer::_init(_executor& p_executor)
  {
    if (!p_executor.m_running)
    {
      p_executor.m_running = true;
      p_executor.m_result = std::async(std::launch::async, &timer::_run, this, std::ref(p_executor));
    }
  }

  inline void timer::_wait(_executor& p_executor)
  {
    if (p_executor.m_result.valid())
    {
      p_executor.m_result.get();
    }
  }

  inline void timer::_run(_executor& p_executor)
  {
    try
    {
      event_t event;
      _clock_t::time_point event_time;
      std::unique_lock<std::mutex> condition_guard(m_condition_mtx, std::defer_lock);
      auto scheduled_execution = [&]
        {
          if (m_condition.wait_until(condition_guard, event_time, std::bind(&timer::_condition_check, this)))
          {
            return false;
          }
          condition_guard.unlock();
          event();
          condition_guard.lock();
          return !_condition_check();
        };
      condition_guard.lock();
      while (_state::destruct != m_state)
      {
        event = m_event;
        event_time = m_event_time;
        m_state = _state::active;
        if (!scheduled_execution())
        {
          continue;
        }
        if (duration_t{} == m_period)
        {
          m_state = _state::destruct;
          break;
        }
        do
        {
          event_time = (type::fixed_delay == m_type ? _clock_t::now() : event_time) + m_period;
        } while (scheduled_execution());
      }
      p_executor.m_running = false;
    }
    catch (...)
    {
      std::terminate();
    }
  }
#pragma endregion
}