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
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <tuple>

namespace flib
{
  class Executor
  {
  public:
    using Clock = std::chrono::steady_clock;
    using Priority = uint8_t;
    using Task = std::function<void(void)>;
    using Token = std::weak_ptr<void>;
    using WorkerDiagnostics = std::tuple<
      bool              /*active*/,
      Clock::time_point /*taskStart*/,
      Clock::time_point /*taskEnd*/
    >;

    explicit inline Executor(const bool enabled = true, const std::size_t workerCount = 1);
    inline Executor(const Executor&) = delete;
    inline Executor(Executor&&) = default;
    inline ~Executor(void) noexcept;
    inline Executor& operator=(const Executor&) = delete;
    inline Executor& operator=(Executor&&) = default;
    inline void Cancel(const Token& token);
    inline void Clear(void);
    inline std::list<WorkerDiagnostics> Diagnostics(void) const;
    inline void Disable(void);
    inline void Enable(void);
    inline Token Invoke(const Task& task, const Priority priority = 0);
    inline Token Invoke(Task&& task, const Priority priority = 0);
    inline bool IsEmpty(void) const;
    inline bool IsEnabled(void) const;
    inline void SetWorkerCount(const std::size_t workerCount);
    inline std::size_t TaskCount(void) const;
    inline std::size_t WorkerCount(void) const;

  private:
    struct Worker
    {
      std::future<void> thread;
      std::atomic<Clock::time_point> taskStart{ Clock::time_point() };
      std::atomic<Clock::time_point> taskEnd{ Clock::time_point() };
    };

    inline void UpdateState(const bool enabled);
    inline void AsyncProcess(Worker* worker);

    const std::chrono::milliseconds mDestructionTimeout;
    std::atomic<bool> mEnabled;
    std::condition_variable mWaitCondition;
    std::list<std::shared_ptr<std::tuple<Task, Priority>>> mTasks;
    mutable std::recursive_mutex mTasksAccessLock;
    std::list<Worker> mWorkers;
    mutable std::recursive_mutex mWorkersAccessLock;
  };
}

// IMPLEMENTATION

flib::Executor::Executor(const bool enabled, const std::size_t workerCount)
  : mDestructionTimeout(std::chrono::milliseconds(50)),
  mWorkers(workerCount)
{
  UpdateState(enabled);
}

flib::Executor::~Executor(void) noexcept
{
  UpdateState(false);
}

void flib::Executor::Cancel(const Token& token)
{
  std::lock_guard<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock);
  mTasks.remove(std::static_pointer_cast<decltype(mTasks)::value_type::element_type>(token.lock()));
}

void flib::Executor::Clear(void)
{
  std::lock_guard<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock);
  mTasks.clear();
}

std::list<flib::Executor::WorkerDiagnostics> flib::Executor::Diagnostics(void) const
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  std::list<flib::Executor::WorkerDiagnostics> result;
  for (auto& worker : mWorkers)
  {
    result.push_back({
      worker.thread.valid() && std::future_status::ready != worker.thread.wait_for(std::chrono::seconds(0)),
      worker.taskStart,
      worker.taskEnd
      });
  }
  return result;
}

void flib::Executor::Disable(void)
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  UpdateState(false);
}

void flib::Executor::Enable(void)
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  UpdateState(true);
}

flib::Executor::Token flib::Executor::Invoke(const Task& task, const Priority priority)
{
  if (!task)
  {
    throw std::invalid_argument("Invalid task");
  }
  std::lock_guard<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock);
  auto result = mTasks.emplace(0 == priority ? mTasks.cend() : std::upper_bound(mTasks.cbegin(), mTasks.cend(),
    priority, [](const decltype(priority)& value, const decltype(mTasks)::value_type& element)
    {
      return value > std::get<1>(*element);
    }), std::make_shared<decltype(mTasks)::value_type::element_type>(task, priority));
  mWaitCondition.notify_one();
  return *result;
}

flib::Executor::Token flib::Executor::Invoke(Task&& task, const Priority priority)
{
  if (!task)
  {
    throw std::invalid_argument("Invalid task");
  }
  std::lock_guard<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock);
  auto result = mTasks.emplace(0 == priority ? mTasks.cend() : std::upper_bound(mTasks.cbegin(), mTasks.cend(),
    priority, [](const decltype(priority)& value, const decltype(mTasks)::value_type& element)
    {
      return value > std::get<1>(*element);
    }), std::make_shared<decltype(mTasks)::value_type::element_type>(std::move(task), priority));
  mWaitCondition.notify_one();
  return *result;
}

bool flib::Executor::IsEmpty(void) const
{
  std::lock_guard<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock);
  return mTasks.empty();
}

bool flib::Executor::IsEnabled(void) const
{
  return mEnabled;
}

void flib::Executor::SetWorkerCount(const std::size_t workerCount)
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  auto enabled = mEnabled.load();
  if (enabled)
  {
    UpdateState(false);
  }
  mWorkers.resize(workerCount);
  if (enabled)
  {
    UpdateState(enabled);
  }
}

std::size_t flib::Executor::TaskCount(void) const
{
  std::lock_guard<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock);
  return mTasks.size();
}

std::size_t flib::Executor::WorkerCount(void) const
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  return mWorkers.size();
}

void flib::Executor::UpdateState(const bool enabled)
{
  mEnabled = enabled;
  if (enabled)
  {
    for (auto& worker : mWorkers)
    {
      if (!worker.thread.valid() || std::future_status::ready == worker.thread.wait_for(std::chrono::seconds(0)))
      {
        worker.thread = std::async(std::launch::async, &Executor::AsyncProcess, this, &worker);
      }
    }
  }
  else
  {
    for (auto& worker : mWorkers)
    {
      if (worker.thread.valid() && std::future_status::timeout == worker.thread.wait_for(std::chrono::seconds(0)))
      {
        do
        {
          mWaitCondition.notify_all();
        } while (std::future_status::timeout == worker.thread.wait_for(mDestructionTimeout));
      }
    }
  }
}

void flib::Executor::AsyncProcess(Worker* worker)
{
  Task task;
  std::mutex waitLock;
  std::unique_lock<decltype(waitLock)> waitGuard(waitLock);
  std::unique_lock<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock, std::defer_lock);
  while (mEnabled)
  {
    mWaitCondition.wait(waitGuard, [this]()
      {
        return !mEnabled || !IsEmpty();
      }
    );
    tasksAccessGuard.lock();
    if (!mEnabled || mTasks.empty())
    {
      tasksAccessGuard.unlock();
      continue;
    }
    task = std::get<0>(*mTasks.front());
    mTasks.pop_front();
    tasksAccessGuard.unlock();
    worker->taskStart = Clock::now();
    task();
    worker->taskEnd = Clock::now();
    std::this_thread::yield();
  }
}