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

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <utility>

#include <flib/pimpl.hpp>

namespace flib
{
  class timer
  {
  public:
    using duration_t = std::chrono::nanoseconds;
    using event_t = std::function<void(void)>;

    enum class type_t
    {
      fixed_delay,
      fixed_rate
    };

    timer(void) = default;
    timer(const timer&) = delete;
    timer(timer&&) = default;
    ~timer(void) noexcept;
    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = default;
    void clear(void);
    void reschedule(void);
    void schedule(event_t event, duration_t delay, duration_t period = {}, type_t type = type_t::fixed_delay);
    bool scheduled(void) const;

  private:
    using _clock_t = std::chrono::steady_clock;

    enum class _state_t
    {
      activating,
      active,
      destruct
    };

    struct _executor;
    struct _storage;

    bool _condition_check(void) const;
    void _init(_executor& executor);
    void _wait(_executor& executor);
    void _run(_executor& executor);

    pimpl<_storage> m_storage;
  };

  // IMPLEMENTATION

  struct timer::_executor
  {
    bool running{ false };
    std::future<void> result;
  };

  struct timer::_storage
  {
    event_t event;
    duration_t delay{};
    duration_t period{};
    type_t type{ type_t::fixed_delay };
    _state_t state{ _state_t::destruct };
    _clock_t::time_point event_time;
    _executor executor;
    std::condition_variable condition;
    mutable std::mutex condition_mtx;
  };

  inline timer::~timer(void) noexcept
  {
    clear();
    _wait(m_storage->executor);
  }

  inline void timer::clear(void)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->state = _state_t::destruct;
    condition_guard.unlock();
    m_storage->condition.notify_all();
  }

  inline void timer::reschedule(void)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    if (!m_storage->event)
    {
      return;
    }
    m_storage->event_time = _clock_t::now() + m_storage->delay;
    m_storage->state = _state_t::activating;
    _init(m_storage->executor);
    condition_guard.unlock();
    m_storage->condition.notify_all();
  }

  inline void timer::schedule(event_t event, duration_t delay, duration_t period, type_t type)
  {
    if (!event)
    {
      return;
    }
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->event = std::move(event);
    m_storage->delay = std::move(delay);
    m_storage->period = std::move(period);
    m_storage->type = std::move(type);
    m_storage->event_time = _clock_t::now() + m_storage->delay;
    m_storage->state = _state_t::activating;
    _init(m_storage->executor);
    condition_guard.unlock();
    m_storage->condition.notify_all();
  }

  inline bool timer::scheduled(void) const
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return _state_t::active == m_storage->state || _state_t::activating == m_storage->state;
  }

  inline bool timer::_condition_check(void) const
  {
    return _state_t::active != m_storage->state;
  }

  inline void timer::_init(_executor& executor)
  {
    if (!executor.running)
    {
      executor.running = true;
      executor.result = std::async(std::launch::async, &timer::_run, this, std::ref(executor));
    }
  }

  inline void timer::_wait(_executor& executor)
  {
    if (executor.result.valid())
    {
      executor.result.get();
    }
  }

  inline void timer::_run(_executor& executor)
  {
    try
    {
      event_t event;
      _clock_t::time_point event_time;
      std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx, std::defer_lock);
      auto scheduled_execution = [&]
      {
        if (m_storage->condition.wait_until(condition_guard, event_time, std::bind(&timer::_condition_check, this)))
        {
          return false;
        }
        condition_guard.unlock();
        event();
        condition_guard.lock();
        return !_condition_check();
      };
      condition_guard.lock();
      while (_state_t::destruct != m_storage->state)
      {
        event = m_storage->event;
        event_time = m_storage->event_time;
        m_storage->state = _state_t::active;
        if (!scheduled_execution())
        {
          continue;
        }
        if (duration_t{} == m_storage->period)
        {
          m_storage->state = _state_t::destruct;
          break;
        }
        do
        {
          event_time = (type_t::fixed_delay == m_storage->type ? _clock_t::now() : event_time) + m_storage->period;
        } while (scheduled_execution());
      }
      executor.running = false;
    }
    catch (...)
    {
      std::terminate();
    }
  }
}