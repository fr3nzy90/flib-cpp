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
    using Priority = uint8_t;
    using Task = std::function<void(void)>;
    using Token = std::weak_ptr<void>;

    explicit inline Executor(const bool enabled = true, const std::size_t workerCount = 1);
    inline Executor(const Executor&) = delete;
    inline Executor(Executor&&) = default;
    inline ~Executor(void) noexcept;
    inline Executor& operator=(const Executor&) = delete;
    inline Executor& operator=(Executor&&) = default;
    inline void Cancel(const Token& token);
    inline void Clear(void);
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
    inline void AsyncProcess(void);

    enum class State
    {
      Active,
      Idle,
      Destruct
    };

    const std::chrono::milliseconds mDestructionTimeout;
    std::atomic<State> mState;
    std::condition_variable mWaitCondition;
    std::list<std::shared_ptr<std::tuple<Task, Priority>>> mTasks;
    mutable std::recursive_mutex mTasksAccessLock;
    std::list<std::future<void>> mWorkers;
    mutable std::recursive_mutex mWorkersAccessLock;
  };
}

// IMPLEMENTATION

flib::Executor::Executor(const bool enabled, const std::size_t workerCount)
  : mDestructionTimeout(std::chrono::milliseconds(50)),
  mState(enabled ? State::Active : State::Idle),
  mWorkers(workerCount)
{
  for (auto& worker : mWorkers)
  {
    worker = std::async(std::launch::async, &Executor::AsyncProcess, this);
  }
}

flib::Executor::~Executor(void) noexcept
{
  mState = State::Destruct;
  for (auto& worker : mWorkers)
  {
    do
    {
      mWaitCondition.notify_all();
    } while (std::future_status::timeout == worker.wait_for(mDestructionTimeout));
  }
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

void flib::Executor::Disable(void)
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  mState = State::Idle;
}

void flib::Executor::Enable(void)
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  mState = State::Active;
  mWaitCondition.notify_all();
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
  return State::Active == mState;
}

void flib::Executor::SetWorkerCount(const std::size_t workerCount)
{
  std::lock_guard<decltype(mWorkersAccessLock)> workersAccessGuard(mWorkersAccessLock);
  if (State::Idle != mState)
  {
    throw std::runtime_error("Executor not idle");
  }
  mState = State::Destruct;
  for (auto& worker : mWorkers)
  {
    do
    {
      mWaitCondition.notify_all();
    } while (std::future_status::timeout == worker.wait_for(mDestructionTimeout));
  }
  mWorkers.resize(workerCount);
  mState = State::Idle;
  for (auto& worker : mWorkers)
  {
    worker = std::async(std::launch::async, &Executor::AsyncProcess, this);
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

void flib::Executor::AsyncProcess(void)
{
  Task task;
  std::mutex waitLock;
  std::unique_lock<decltype(waitLock)> waitGuard(waitLock);
  std::unique_lock<decltype(mTasksAccessLock)> tasksAccessGuard(mTasksAccessLock, std::defer_lock);
  while (State::Destruct != mState)
  {
    mWaitCondition.wait(waitGuard, [this]()
      {
        return State::Destruct == mState || (State::Active == mState && !IsEmpty());
      }
    );
    tasksAccessGuard.lock();
    if (State::Active != mState || mTasks.empty())
    {
      tasksAccessGuard.unlock();
      continue;
    }
    task = std::get<0>(*mTasks.front());
    mTasks.pop_front();
    tasksAccessGuard.unlock();
    task();
    std::this_thread::yield();
  }
}