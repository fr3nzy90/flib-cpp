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
    using invocation_t = std::weak_ptr<void>;
    using priority_t = uint8_t;
    using size_t = std::size_t;
    using task_t = std::function<void(void)>;
    using thread_id_t = std::thread::id;
    using time_point = std::chrono::steady_clock::time_point;

    struct diagnostics_t
    {
      thread_id_t thread_id;
      bool active;
      time_point task_start;
      time_point task_end;
    };

    using diagnostics_list_t = std::list<diagnostics_t>;

    explicit inline worker(bool enabled = true, size_t executors = 1);
    inline worker(const worker&) = delete;
    inline worker(worker&&) = delete;
    inline ~worker(void) noexcept;
    inline worker& operator=(const worker&) = delete;
    inline worker& operator=(worker&&) = delete;
    inline worker& cancel(const invocation_t& invocation);
    inline worker& clear(void);
    inline diagnostics_list_t diagnostics(void) const;
    inline worker& disable(void);
    inline bool empty(void) const;
    inline worker& enable(void);
    inline bool enabled(void) const;
    inline worker& executors(size_t executors);
    inline size_t executors(void) const;
    inline invocation_t invoke(task_t&& task, priority_t priority = 0);
    inline invocation_t invoke(const task_t& task, priority_t priority = 0);
    inline bool invoked(const invocation_t& invocation) const;
    inline size_t size(void) const;

  protected:
    struct _extended_task_t
    {
      task_t task;
      priority_t priority;
    };

    struct _executor_t
    {
      std::thread thread;
      std::future<void> result;
      std::atomic<decltype(diagnostics_t::task_start)> task_start{ {} };
      std::atomic<decltype(diagnostics_t::task_end)> task_end{ {} };
    };

    inline void _start(void);
    inline void _stop(void);
    inline void _run(_executor_t* executor);

    std::atomic<bool> m_enabled;
    std::condition_variable m_condition;
    std::list<std::shared_ptr<_extended_task_t>> m_tasks;
    mutable std::mutex m_tasks_mtx;
    std::list<_executor_t> m_executors;
    mutable std::mutex m_executors_mtx;
  };
}

// IMPLEMENTATION

flib::worker::worker(bool enabled, size_t executors)
  : m_enabled(false),
  m_executors(executors)
{
  if (enabled)
  {
    _start();
  }
}

flib::worker::~worker(void) noexcept
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  _stop();
}

flib::worker& flib::worker::cancel(const invocation_t& invocation)
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  m_tasks.remove(std::static_pointer_cast<decltype(m_tasks)::value_type::element_type>(invocation.lock()));
  return *this;
}

flib::worker& flib::worker::clear(void)
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  m_tasks.clear();
  return *this;
}

flib::worker::diagnostics_list_t flib::worker::diagnostics(void) const
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  diagnostics_list_t result;
  for (const auto& executor : m_executors)
  {
    result.push_back({
      executor.thread.get_id(),
      executor.result.valid() && std::future_status::ready != executor.result.wait_for(std::chrono::seconds(0)),
      executor.task_start,
      executor.task_end
      });
  }
  return result;
}

flib::worker& flib::worker::disable(void)
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  _stop();
  return *this;
}

bool flib::worker::empty(void) const
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  return m_tasks.empty();
}

flib::worker& flib::worker::enable(void)
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  _start();
  return *this;
}

bool flib::worker::enabled(void) const
{
  return m_enabled;
}

flib::worker& flib::worker::executors(size_t executors)
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  auto enabled = m_enabled.load();
  if (enabled)
  {
    _stop();
  }
  m_executors.resize(executors);
  if (enabled)
  {
    _start();
  }
  return *this;
}

flib::worker::size_t flib::worker::executors(void) const
{
  std::lock_guard<decltype(m_executors_mtx)> executors_guard(m_executors_mtx);
  return static_cast<size_t>(m_executors.size());
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

bool flib::worker::invoked(const invocation_t& invocation) const
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  return m_tasks.cend() != std::find(m_tasks.cbegin(), m_tasks.cend(),
    std::static_pointer_cast<decltype(m_tasks)::value_type::element_type>(invocation.lock()));
}

flib::worker::size_t flib::worker::size(void) const
{
  std::lock_guard<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx);
  return static_cast<size_t>(m_tasks.size());
}

void flib::worker::_start(void)
{
  m_enabled = true;
  for (auto& executor : m_executors)
  {
    if (executor.result.valid() && std::future_status::ready != executor.result.wait_for(std::chrono::seconds(0)))
    {
      continue;
    }
    std::packaged_task<void(_executor_t*)> task(std::bind(&worker::_run, this, std::placeholders::_1));
    executor.result = task.get_future();
    executor.thread = decltype(_executor_t::thread)(std::move(task), &executor);
  }
}

void flib::worker::_stop(void)
{
  for (const auto& executor : m_executors)
  {
    if (std::this_thread::get_id() == executor.thread.get_id())
    {
      throw std::runtime_error("Synchronous invocation of stop on internal thread");
    }
  }
  m_enabled = false;
  for (auto& executor : m_executors)
  {
    if (executor.result.valid() && std::future_status::ready != executor.result.wait_for(std::chrono::seconds(0)))
    {
      do
      {
        m_condition.notify_all();
      } while (std::future_status::timeout == executor.result.wait_for(std::chrono::milliseconds(1)));
    }
    if (executor.thread.joinable())
    {
      executor.thread.join();
    }
  }
}

void flib::worker::_run(_executor_t* executor)
{
  try
  {
    decltype(decltype(m_tasks)::value_type::element_type::task) task;
    std::mutex condition_mtx;
    std::unique_lock<decltype(condition_mtx)> condition_guard(condition_mtx);
    std::unique_lock<decltype(m_tasks_mtx)> tasks_guard(m_tasks_mtx, std::defer_lock);
    while (m_enabled)
    {
      m_condition.wait(condition_guard, [this]
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
      executor->task_start = decltype(diagnostics_t::task_start)::clock::now();
      task();
      executor->task_end = decltype(diagnostics_t::task_end)::clock::now();
      std::this_thread::yield();
    }
  }
  catch (...)
  {
    std::terminate();
  }
}