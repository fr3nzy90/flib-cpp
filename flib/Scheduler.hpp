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
#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <tuple>

namespace flib
{
  class Scheduler
  {
  public:
    using Duration = std::chrono::milliseconds;
    using Event = std::function<void(void)>;

    enum class Type
    {
      FixedDelay,
      FixedRate
    };

    inline Scheduler(void);
    inline Scheduler(const Scheduler&) = delete;
    inline Scheduler(Scheduler&&) = default;
    inline ~Scheduler(void) noexcept;
    inline Scheduler& operator=(const Scheduler&) = delete;
    inline Scheduler& operator=(Scheduler&&) = default;
    inline void Cancel(void);
    inline bool IsScheduled(void) const;
    inline void Reschedule(void);
    inline void Schedule(const Event& event, const Duration& delay, const Duration& period = Duration(0),
      const Type type = Type::FixedDelay);

  private:
    inline void AsyncProcess(void);

    using Clock = std::chrono::steady_clock;

    enum class State
    {
      Activating,
      Active,
      Idle,
      Destruct
    };

    const std::chrono::milliseconds mDestructionTimeout;
    std::atomic<State> mState;
    std::condition_variable mWaitCondition;
    std::tuple<
      Event    /*event*/,
      Duration /*delay*/,
      Duration /*period*/,
      Type     /*type*/
    > mConfiguration;
    mutable std::recursive_mutex mConfigurationAccessLock;
    std::future<void> mWorker;
  };
}

// IMPLEMENTATION

flib::Scheduler::Scheduler(void)
  : mDestructionTimeout(std::chrono::milliseconds(50)),
  mState(State::Idle),
  mConfiguration{ {},Duration(0),Duration(0),Type::FixedDelay },
  mWorker(std::async(std::launch::async, &Scheduler::AsyncProcess, this))
{
}

flib::Scheduler::~Scheduler(void) noexcept
{
  mState = State::Destruct;
  do
  {
    mWaitCondition.notify_all();
  } while (std::future_status::timeout == mWorker.wait_for(mDestructionTimeout));
}

void flib::Scheduler::Cancel(void)
{
  mState = State::Idle;
  mWaitCondition.notify_all();
}

bool flib::Scheduler::IsScheduled(void) const
{
  return State::Activating == mState || State::Active == mState;
}

void flib::Scheduler::Reschedule(void)
{
  std::unique_lock<decltype(mConfigurationAccessLock)> configurationGuard(mConfigurationAccessLock);
  if (!std::get<0>(mConfiguration))
  {
    throw std::runtime_error("Invalid event");
  }
  configurationGuard.unlock();
  mState = State::Activating;
  mWaitCondition.notify_all();
}

void flib::Scheduler::Schedule(const Event& event, const Duration& delay, const Duration& period, const Type type)
{
  if (!event)
  {
    throw std::invalid_argument("Invalid event");
  }
  mState = State::Idle;
  std::unique_lock<decltype(mConfigurationAccessLock)> configurationGuard(mConfigurationAccessLock);
  mConfiguration = { event, delay, period, type };
  configurationGuard.unlock();
  mState = State::Activating;
  mWaitCondition.notify_all();
}

void flib::Scheduler::AsyncProcess(void)
{
  Event event;
  Duration delay, period;
  Type type;
  std::mutex waitLock;
  std::unique_lock<decltype(waitLock)> waitGuard(waitLock);
  std::unique_lock<decltype(mConfigurationAccessLock)> configurationGuard(mConfigurationAccessLock, std::defer_lock);
  Clock::time_point eventTime;
  while (State::Destruct != mState)
  {
    mWaitCondition.wait(waitGuard, [this]()
      {
        return State::Destruct == mState || State::Activating == mState;
      }
    );
    if (State::Activating != mState)
    {
      continue;
    }
    mState = State::Active;
    configurationGuard.lock();
    std::tie(event, delay, period, type) = mConfiguration;
    configurationGuard.unlock();
    eventTime = Clock::now() + delay;
    mWaitCondition.wait_until(waitGuard, eventTime, [this, &eventTime]()
      {
        return eventTime <= Clock::now() || State::Active != mState;
      }
    );
    if (State::Active == mState)
    {
      if (Duration(0) == period)
      {
        mState = State::Idle;
      }
      event();
      std::this_thread::yield();
    }
    while (State::Active == mState)
    {
      if (Type::FixedDelay == type)
      {
        eventTime = Clock::now() + period;
      }
      else
      {
        eventTime += period;
      }
      mWaitCondition.wait_until(waitGuard, eventTime, [this, &eventTime]()
        {
          return eventTime <= Clock::now() || State::Active != mState;
        }
      );
      if (State::Active == mState)
      {
        event();
        std::this_thread::yield();
      }
    }
  }
}