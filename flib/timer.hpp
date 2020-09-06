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

namespace flib
{
  class timer
  {
  public:
    using clock_t = std::chrono::steady_clock;
    using duration_t = std::chrono::milliseconds;
    using event_t = std::function<void(void)>;
    using exception_handler_t = std::function<void(std::exception_ptr)>;

    struct diagnostics_t
    {
      std::thread::id thread_id;
      bool active;
      clock_t::time_point event_start;
      clock_t::time_point event_end;
    };

    enum class type_t
    {
      fixed_delay,
      fixed_rate
    };

    inline timer(void);
    inline timer(const timer&) = delete;
    inline timer(timer&&) = default;
    inline ~timer(void) noexcept;
    inline timer& operator=(const timer&) = delete;
    inline timer& operator=(timer&&) = default;
    inline void cancel(void);
    inline diagnostics_t diagnostics(void) const;
    inline void handle_exceptions(exception_handler_t&& handler);
    inline void handle_exceptions(const exception_handler_t& handler = {});
    inline void reschedule(void);
    inline void schedule(event_t&& event, duration_t&& delay, duration_t&& period = duration_t(0),
      type_t type = type_t::fixed_delay);
    inline void schedule(const event_t& event, const duration_t& delay, const duration_t& period = duration_t(0),
      type_t type = type_t::fixed_delay);
    inline bool scheduled(void) const;

  private:
    struct configuration_t
    {
      event_t event;
      duration_t delay;
      duration_t period;
      type_t type;
    };

    struct executor_t
    {
      std::future<void> thread;
      std::atomic<std::thread::id> thread_id;
      std::atomic<clock_t::time_point> event_start{ clock_t::time_point() };
      std::atomic<clock_t::time_point> event_end{ clock_t::time_point() };
    };

    inline void run(executor_t* executor);

    enum class state_t
    {
      activating,
      active,
      idle,
      destruct
    };

    std::atomic<state_t> m_state;
    std::condition_variable m_condition;
    configuration_t m_configuration;
    mutable std::recursive_mutex m_configuration_mtx;
    executor_t m_executor;
    exception_handler_t m_exception_handler;
    mutable std::mutex m_exception_handler_mtx;
  };
}

// IMPLEMENTATION

flib::timer::timer(void)
  : m_state(state_t::idle),
  m_configuration{ {},duration_t(0),duration_t(0),type_t::fixed_delay }
{
  m_executor.thread = std::async(std::launch::async, &timer::run, this, &m_executor);
}

flib::timer::~timer(void) noexcept
{
  m_state = state_t::destruct;
  if (m_executor.thread.valid())
  {
    do
    {
      m_condition.notify_all();
    } while (std::future_status::timeout == m_executor.thread.wait_for(std::chrono::milliseconds(1)));
  }
}

void flib::timer::cancel(void)
{
  m_state = state_t::idle;
  m_condition.notify_all();
}

flib::timer::diagnostics_t flib::timer::diagnostics(void) const
{
  return {
    m_executor.thread_id,
    m_executor.thread.valid() && std::future_status::ready != m_executor.thread.wait_for(std::chrono::seconds(0)),
    m_executor.event_start,
    m_executor.event_end
  };
}

void flib::timer::handle_exceptions(exception_handler_t&& handler)
{
  std::lock_guard<decltype(m_exception_handler_mtx)> exception_handler_guard(m_exception_handler_mtx);
  m_exception_handler = handler;
}

void flib::timer::handle_exceptions(const exception_handler_t& handler)
{
  handle_exceptions(exception_handler_t(handler));
}

void flib::timer::reschedule(void)
{
  std::unique_lock<decltype(m_configuration_mtx)> configuration_guard(m_configuration_mtx);
  if (!m_configuration.event)
  {
    throw std::runtime_error("Invalid event");
  }
  configuration_guard.unlock();
  m_state = state_t::activating;
  m_condition.notify_all();
}

void flib::timer::schedule(event_t&& event, duration_t&& delay, duration_t&& period, type_t type)
{
  if (!event)
  {
    throw std::invalid_argument("Invalid event");
  }
  m_state = state_t::idle;
  std::unique_lock<decltype(m_configuration_mtx)> configuration_guard(m_configuration_mtx);
  m_configuration = { std::move(event), std::move(delay), std::move(period), std::move(type) };
  configuration_guard.unlock();
  m_state = state_t::activating;
  m_condition.notify_all();
}

void flib::timer::schedule(const event_t& event, const duration_t& delay, const duration_t& period, type_t type)
{
  schedule(event_t(event), duration_t(delay), duration_t(period), type);
}

bool flib::timer::scheduled(void) const
{
  return state_t::activating == m_state || state_t::active == m_state;
}

void flib::timer::run(executor_t* executor)
{
  try
  {
    executor->thread_id = std::this_thread::get_id();
    configuration_t configuration;
    std::mutex condition_mtx;
    std::unique_lock<decltype(condition_mtx)> condition_guard(condition_mtx);
    std::unique_lock<decltype(m_configuration_mtx)> configuration_guard(m_configuration_mtx, std::defer_lock);
    clock_t::time_point event_time;
    auto scheduled_execution = [&]()
    {
      m_condition.wait_until(condition_guard, event_time, [this, &event_time]()
        {
          return event_time <= clock_t::now() || state_t::active != m_state;
        }
      );
      if (state_t::active == m_state)
      {
        executor->event_start = clock_t::now();
        configuration.event();
        executor->event_end = clock_t::now();
        std::this_thread::yield();
      }
    };
    while (state_t::destruct != m_state)
    {
      m_condition.wait(condition_guard, [this]()
        {
          return state_t::destruct == m_state || state_t::activating == m_state;
        }
      );
      if (state_t::activating != m_state)
      {
        continue;
      }
      m_state = state_t::active;
      configuration_guard.lock();
      configuration = m_configuration;
      configuration_guard.unlock();
      event_time = clock_t::now() + configuration.delay;
      scheduled_execution();
      if (state_t::active == m_state && duration_t(0) == configuration.period)
      {
        m_state = state_t::idle;
      }
      while (state_t::active == m_state)
      {
        if (type_t::fixed_delay == configuration.type)
        {
          event_time = clock_t::now() + configuration.period;
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
    m_state = state_t::destruct;
    std::unique_lock<decltype(m_exception_handler_mtx)> exception_handler_guard(m_exception_handler_mtx);
    auto exception_handler = m_exception_handler;
    exception_handler_guard.unlock();
    if (exception_handler)
    {
      exception_handler(std::current_exception());
    }
  }
}