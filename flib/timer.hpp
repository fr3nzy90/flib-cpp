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
    ~timer(void) noexcept = default;
    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = default;
    inline void clear(void);
    inline void reschedule(void);
    inline void schedule(event_t event, duration_t delay, duration_t period = {}, type_t type = type_t::fixed_delay);
    inline bool scheduled(void) const;

  private:
    class _impl;
    pimpl<_impl> m_impl;
  };

  // IMPLEMENTATION

  class timer::_impl
  {
  public:
    inline _impl(void);
    inline ~_impl(void) noexcept;
    inline void _clear(void);
    inline void _reschedule(void);
    inline void _schedule(event_t event, duration_t delay, duration_t period, type_t type);
    inline bool _scheduled(void) const;

  private:
    using __clock_t = std::chrono::steady_clock;

    enum class __state_t
    {
      activating,
      active,
      destruct
    };

    struct __executor_t
    {
      bool running = false;
      std::future<void> result;
    };

    inline bool __condition_check(void) const;
    inline void __init(__executor_t& executor);
    inline void __wait(__executor_t& executor);
    inline void __run(__executor_t& executor);

    event_t m_event;
    duration_t m_delay;
    duration_t m_period;
    type_t m_type;
    __state_t m_state;
    __clock_t::time_point m_event_time;
    __executor_t m_executor;
    std::condition_variable m_condition;
    mutable std::mutex m_condition_mtx;
  };

  timer::_impl::_impl(void)
    : m_delay{},
    m_period{},
    m_type{ type_t::fixed_delay },
    m_state{ __state_t::destruct }
  {
  }

  timer::_impl::~_impl(void) noexcept
  {
    _clear();
    __wait(m_executor);
  }

  void timer::_impl::_clear(void)
  {
    {
      std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
      m_state = __state_t::destruct;
    }
    m_condition.notify_all();
  }

  void timer::_impl::_reschedule(void)
  {
    {
      std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
      if (!m_event)
      {
        return;
      }
      m_event_time = __clock_t::now() + m_delay;
      m_state = __state_t::activating;
      __init(m_executor);
    }
    m_condition.notify_all();
  }

  void timer::_impl::_schedule(event_t event, duration_t delay, duration_t period, type_t type)
  {
    if (!event)
    {
      return;
    }
    {
      std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
      m_event = std::move(event);
      m_delay = std::move(delay);
      m_period = std::move(period);
      m_type = std::move(type);
      m_event_time = __clock_t::now() + m_delay;
      m_state = __state_t::activating;
      __init(m_executor);
    }
    m_condition.notify_all();
  }

  bool timer::_impl::_scheduled(void) const
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    return __state_t::active == m_state || __state_t::activating == m_state;
  }

  bool timer::_impl::__condition_check(void) const
  {
    return __state_t::active != m_state;
  }

  void timer::_impl::__init(__executor_t& executor)
  {
    if (!executor.running)
    {
      executor.running = true;
      executor.result = std::async(std::launch::async, &_impl::__run, this, std::ref(executor));
    }
  }

  void timer::_impl::__wait(__executor_t& executor)
  {
    if (executor.result.valid())
    {
      executor.result.get();
    }
  }

  void timer::_impl::__run(__executor_t& executor)
  {
    try
    {
      event_t event;
      __clock_t::time_point event_time;
      std::unique_lock<std::mutex> condition_guard(m_condition_mtx, std::defer_lock);
      auto scheduled_execution = [&]
      {
        if (m_condition.wait_until(condition_guard, event_time, std::bind(&_impl::__condition_check, this)))
        {
          return false;
        }
        condition_guard.unlock();
        event();
        condition_guard.lock();
        return !__condition_check();
      };
      condition_guard.lock();
      while (__state_t::destruct != m_state)
      {
        event = m_event;
        event_time = m_event_time;
        m_state = __state_t::active;
        if (!scheduled_execution())
        {
          continue;
        }
        if (duration_t{} == m_period)
        {
          m_state = __state_t::destruct;
          break;
        }
        do
        {
          event_time = (type_t::fixed_delay == m_type ? __clock_t::now() : event_time) + m_period;
        } while (scheduled_execution());
      }
      executor.running = false;
    }
    catch (...)
    {
      std::terminate();
    }
  }

  void timer::clear(void)
  {
    m_impl->_clear();
  }

  void timer::reschedule(void)
  {
    m_impl->_reschedule();
  }

  void timer::schedule(event_t event, duration_t delay, duration_t period, type_t type)
  {
    m_impl->_schedule(std::move(event), std::move(delay), std::move(period), std::move(type));
  }

  bool timer::scheduled(void) const
  {
    return m_impl->_scheduled();
  }
}