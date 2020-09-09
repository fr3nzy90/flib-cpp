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
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

namespace flib
{
  class timer
  {
  public:
    using duration_t = std::chrono::milliseconds;
    using event_t = std::function<void(void)>;
    using thread_id_t = std::thread::id;
    using time_point = std::chrono::steady_clock::time_point;

    struct diagnostics_t
    {
      thread_id_t thread_id;
      bool active;
      time_point event_start;
      time_point event_end;
    };

    enum class type_t
    {
      fixed_delay,
      fixed_rate
    };

    inline timer(void);
    inline timer(const timer&) = delete;
    inline timer(timer&&) = delete;
    inline ~timer(void) noexcept;
    inline timer& operator=(const timer&) = delete;
    inline timer& operator=(timer&&) = delete;
    inline timer& cancel(void);
    inline diagnostics_t diagnostics(void) const;
    inline timer& reschedule(void);
    inline timer& schedule(event_t&& event, duration_t&& delay, duration_t&& period = {},
      type_t type = type_t::fixed_delay);
    inline timer& schedule(const event_t& event, const duration_t& delay, const duration_t& period = {},
      type_t type = type_t::fixed_delay);
    inline bool scheduled(void) const;

  protected:
    struct _configuration_t
    {
      event_t event;
      duration_t delay;
      duration_t period;
      type_t type;
    };

    struct _executor_t
    {
      std::thread thread;
      std::future<void> result;
      std::atomic<time_point> event_start{ {} };
      std::atomic<time_point> event_end{ {} };
    };

    inline void _run(_executor_t* executor);

    enum class _state_t
    {
      activating,
      active,
      idle,
      destruct
    };

    std::atomic<_state_t> m_state;
    std::condition_variable m_condition;
    _configuration_t m_configuration;
    mutable std::mutex m_configuration_mtx;
    _executor_t m_executor;
  };
}

// IMPLEMENTATION

flib::timer::timer(void)
  : m_state(_state_t::idle),
  m_configuration{ {}, {}, {}, type_t::fixed_delay }
{
  std::packaged_task<void(_executor_t*)> task(std::bind(&timer::_run, this, std::placeholders::_1));
  m_executor.result = task.get_future();
  m_executor.thread = decltype(_executor_t::thread)(std::move(task), &m_executor);
}

flib::timer::~timer(void) noexcept
{
  m_state = _state_t::destruct;
  if (m_executor.result.valid())
  {
    do
    {
      m_condition.notify_all();
    } while (std::future_status::timeout == m_executor.result.wait_for(std::chrono::milliseconds(1)));
  }
  if (m_executor.thread.joinable())
  {
    m_executor.thread.join();
  }
}

flib::timer& flib::timer::cancel(void)
{
  m_state = _state_t::idle;
  m_condition.notify_all();
  return *this;
}

flib::timer::diagnostics_t flib::timer::diagnostics(void) const
{
  return {
    m_executor.thread.get_id(),
    m_executor.result.valid() && std::future_status::ready != m_executor.result.wait_for(std::chrono::seconds(0)),
    m_executor.event_start,
    m_executor.event_end
  };
}

flib::timer& flib::timer::reschedule(void)
{
  std::unique_lock<decltype(m_configuration_mtx)> configuration_guard(m_configuration_mtx);
  if (!m_configuration.event)
  {
    throw std::runtime_error("Invalid event");
  }
  configuration_guard.unlock();
  m_state = _state_t::activating;
  m_condition.notify_all();
  return *this;
}

flib::timer& flib::timer::schedule(event_t&& event, duration_t&& delay, duration_t&& period, type_t type)
{
  if (!event)
  {
    throw std::invalid_argument("Invalid event");
  }
  m_state = _state_t::idle;
  std::unique_lock<decltype(m_configuration_mtx)> configuration_guard(m_configuration_mtx);
  m_configuration = { std::move(event), std::move(delay), std::move(period), std::move(type) };
  configuration_guard.unlock();
  m_state = _state_t::activating;
  m_condition.notify_all();
  return *this;
}

flib::timer& flib::timer::schedule(const event_t& event, const duration_t& delay, const duration_t& period,
  type_t type)
{
  return schedule(event_t(event), duration_t(delay), duration_t(period), type);
}

bool flib::timer::scheduled(void) const
{
  return _state_t::activating == m_state || _state_t::active == m_state;
}

void flib::timer::_run(_executor_t* executor)
{
  try
  {
    _configuration_t configuration;
    std::mutex condition_mtx;
    std::unique_lock<decltype(condition_mtx)> condition_guard(condition_mtx);
    std::unique_lock<decltype(m_configuration_mtx)> configuration_guard(m_configuration_mtx, std::defer_lock);
    time_point event_time;
    auto scheduled_execution = [&]
    {
      m_condition.wait_until(condition_guard, event_time, [this, &event_time]
        {
          return event_time <= time_point::clock::now() || _state_t::active != m_state;
        }
      );
      if (_state_t::active == m_state)
      {
        executor->event_start = time_point::clock::now();
        configuration.event();
        executor->event_end = time_point::clock::now();
        std::this_thread::yield();
      }
    };
    while (_state_t::destruct != m_state)
    {
      m_condition.wait(condition_guard, [this]
        {
          return _state_t::destruct == m_state || _state_t::activating == m_state;
        }
      );
      if (_state_t::activating != m_state)
      {
        continue;
      }
      m_state = _state_t::active;
      configuration_guard.lock();
      configuration = m_configuration;
      configuration_guard.unlock();
      event_time = time_point::clock::now() + configuration.delay;
      scheduled_execution();
      if (_state_t::active == m_state && decltype(configuration.period){} == configuration.period)
      {
        m_state = _state_t::idle;
      }
      while (_state_t::active == m_state)
      {
        if (type_t::fixed_delay == configuration.type)
        {
          event_time = time_point::clock::now() + configuration.period;
        }
        else
        {
          event_time += configuration.period;
        }
        scheduled_execution();
      }
    }
  }
  catch (...)
  {
    m_state = _state_t::destruct;
    std::terminate();
  }
}