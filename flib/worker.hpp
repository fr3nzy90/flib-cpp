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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace flib
{
  class worker
  {
  public:
    using clock_t = std::chrono::steady_clock;
    using exception_handler_t = std::function<void(std::exception_ptr)>;
    using invocation_t = std::weak_ptr<void>;
    using priority_t = uint8_t;
    using task_t = std::function<void(void)>;

    struct diagnostics_t
    {
      std::thread::id thread_id;
      bool active;
      clock_t::time_point task_start;
      clock_t::time_point task_end;
    };

    explicit inline worker(bool enabled = true, std::size_t executors = 1);
    inline worker(const worker&) = delete;
    inline worker(worker&&) = default;
    inline ~worker(void) noexcept;
    inline worker& operator=(const worker&) = delete;
    inline worker& operator=(worker&&) = default;
    inline void cancel(const invocation_t& token);
    inline void clear(void);
    inline std::list<diagnostics_t> diagnostics(void) const;
    inline void disable(void);
    inline bool empty(void) const;
    inline void enable(void);
    inline bool enabled(void) const;
    inline void executors(std::size_t executors);
    inline std::size_t executors(void) const;
    inline void handle_exceptions(exception_handler_t&& handler);
    inline void handle_exceptions(const exception_handler_t& handler = {});
    inline invocation_t invoke(task_t&& task, priority_t priority = 0);
    inline invocation_t invoke(const task_t& task, priority_t priority = 0);
    inline std::size_t size(void) const;

  private:
    struct extended_task_t
    {
      task_t task;
      priority_t priority;
    };

    struct executor_t
    {
      std::future<void> thread;
      std::atomic<std::thread::id> thread_id;
      std::atomic<clock_t::time_point> task_start{ {} };
      std::atomic<clock_t::time_point> task_end{ {} };
    };

    inline void start(void);
    inline void stop(bool async);
    inline void run(executor_t* executor);

    std::atomic<bool> m_enabled;
    std::condition_variable m_condition;
    std::list<std::shared_ptr<extended_task_t>> m_tasks;
    mutable std::mutex m_tasks_mtx;
    std::list<executor_t> m_executors;
    mutable std::mutex m_executors_mtx;
    exception_handler_t m_exception_handler;
    mutable std::mutex m_exception_handler_mtx;
  };
}

// IMPLEMENTATION

flib::worker::worker(bool enabled, std::size_t executors)
  : m_enabled(false),
  m_executors(executors)
{
  if (enabled)
  {
    start();
  }
}

flib::worker::~worker(void) noexcept
{
  stop(false);
}

void flib::worker::cancel(const invocation_t& token)
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  m_tasks.remove(std::static_pointer_cast<decltype(m_tasks)::value_type::element_type>(token.lock()));
}

void flib::worker::clear(void)
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  m_tasks.clear();
}

std::list<flib::worker::diagnostics_t> flib::worker::diagnostics(void) const
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  std::list<diagnostics_t> result;
  for (const auto& executor : m_executors)
  {
    result.push_back({
      executor.thread_id,
      executor.thread.valid() && std::future_status::ready != executor.thread.wait_for(std::chrono::seconds(0)),
      executor.task_start,
      executor.task_end
      });
  }
  return result;
}

void flib::worker::disable(void)
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  stop(true);
}

bool flib::worker::empty(void) const
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  return m_tasks.empty();
}

void flib::worker::enable(void)
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  start();
}

bool flib::worker::enabled(void) const
{
  return m_enabled;
}

void flib::worker::executors(std::size_t executors)
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  auto enabled = m_enabled.load();
  if (enabled)
  {
    stop(false);
  }
  m_executors.resize(executors);
  if (enabled)
  {
    start();
  }
}

std::size_t flib::worker::executors(void) const
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  return m_executors.size();
}

void flib::worker::handle_exceptions(exception_handler_t&& handler)
{
  std::lock_guard<decltype(m_exception_handler_mtx)> exception_handler_guard(m_exception_handler_mtx);
  m_exception_handler = handler;
}

void flib::worker::handle_exceptions(const exception_handler_t& handler)
{
  handle_exceptions(exception_handler_t(handler));
}

flib::worker::invocation_t flib::worker::invoke(task_t&& task, priority_t priority)
{
  if (!task)
  {
    throw std::invalid_argument("Invalid task");
  }
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  auto result = m_tasks.emplace(0 == priority ? m_tasks.cend() : std::upper_bound(m_tasks.cbegin(), m_tasks.cend(),
    priority, [](decltype(priority) value, const decltype(m_tasks)::value_type& element)
    {
      return value > element->priority;
    }), std::make_shared<decltype(m_tasks)::value_type::element_type>(
      decltype(m_tasks)::value_type::element_type{ std::move(task), std::move(priority) }));
  m_condition.notify_one();
  return *result;
}

flib::worker::invocation_t flib::worker::invoke(const task_t& task, priority_t priority)
{
  return invoke(task_t(task), priority);
}

std::size_t flib::worker::size(void) const
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  return m_tasks.size();
}

void flib::worker::start(void)
{
  m_enabled = true;
  for (auto& executor : m_executors)
  {
    if (!executor.thread.valid() || std::future_status::ready == executor.thread.wait_for(std::chrono::seconds(0)))
    {
      executor.thread = std::async(std::launch::async, &worker::run, this, &executor);
    }
  }
}

void flib::worker::stop(bool async)
{
  if (!async)
  {
    for (const auto& executor : m_executors)
    {
      if (std::this_thread::get_id() == executor.thread_id)
      {
        throw std::runtime_error("Synchronous invocation of stop on internal thread");
      }
    }
  }
  m_enabled = false;
  for (const auto& executor : m_executors)
  {
    if (std::this_thread::get_id() == executor.thread_id)
    {
      continue;
    }
    if (executor.thread.valid() && std::future_status::timeout == executor.thread.wait_for(std::chrono::seconds(0)))
    {
      do
      {
        m_condition.notify_all();
      } while (std::future_status::timeout == executor.thread.wait_for(std::chrono::milliseconds(1)));
    }
  }
}

void flib::worker::run(executor_t* executor)
{
  try
  {
    executor->thread_id = std::this_thread::get_id();
    task_t task;
    std::mutex condition_mtx;
    std::unique_lock<decltype(condition_mtx)> condition_guard(condition_mtx);
    std::unique_lock<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx, std::defer_lock);
    while (m_enabled)
    {
      m_condition.wait(condition_guard, [this]()
        {
          return !m_enabled || !empty();
        }
      );
      tasks_guard.lock();
      if (!m_enabled || m_tasks.empty())
      {
        tasks_guard.unlock();
        continue;
      }
      task = m_tasks.front()->task;
      m_tasks.pop_front();
      tasks_guard.unlock();
      executor->task_start = clock_t::now();
      task();
      executor->task_end = clock_t::now();
      std::this_thread::yield();
    }
  }
  catch (...)
  {
    std::unique_lock<decltype(m_exception_handler_mtx)> exception_handler_guard(m_exception_handler_mtx);
    auto exception_handler = m_exception_handler;
    exception_handler_guard.unlock();
    if (exception_handler)
    {
      exception_handler(std::current_exception());
    }
  }
}