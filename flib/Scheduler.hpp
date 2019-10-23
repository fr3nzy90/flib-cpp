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

namespace flib
{
  template<class T = std::chrono::milliseconds>
  class Scheduler
  {
  public:
    using Duration = T;
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
    void AsyncProcess(void);

    using Clock = std::chrono::steady_clock;

    enum class State
    {
      Activating,
      Active,
      Idle,
      Destruct
    };

    std::atomic<State> mState;
    std::condition_variable mWaitCondition;
    struct
    {
      Event event;
      Duration delay;
      Duration period;
      Type type;
    } mConfiguration;
    mutable std::recursive_mutex mConfigurationAccessLock;
    std::future<void> mWorker;

    static const std::chrono::milliseconds cDestructionTimeout;
  };
}

// IMPLEMENTATION

template<class T>
const std::chrono::milliseconds flib::Scheduler<T>::cDestructionTimeout = std::chrono::milliseconds(250);

template<class T>
flib::Scheduler<T>::Scheduler(void)
  : mState(State::Idle),
  mConfiguration{ {},Duration(0),Duration(0),Type::FixedDelay },
  mWorker(std::async(std::launch::async, &Scheduler::AsyncProcess, this))
{
}

template<class T>
flib::Scheduler<T>::~Scheduler(void) noexcept
{
  mState = State::Destruct;
  do
  {
    mWaitCondition.notify_all();
  } while (std::future_status::timeout == mWorker.wait_for(cDestructionTimeout));
}

template<class T>
void flib::Scheduler<T>::Cancel(void)
{
  mState = State::Idle;
  mWaitCondition.notify_all();
}

template<class T>
bool flib::Scheduler<T>::IsScheduled(void) const
{
  return State::Activating == mState || State::Active == mState;
}

template<class T>
void flib::Scheduler<T>::Reschedule(void)
{
  std::unique_lock<decltype(mConfigurationAccessLock)> configurationGuard(mConfigurationAccessLock);
  if (!mConfiguration.event)
  {
    throw std::runtime_error("Invalid event");
  }
  configurationGuard.unlock();
  mState = State::Activating;
  mWaitCondition.notify_all();
}

template<class T>
void flib::Scheduler<T>::Schedule(const Event& event, const Duration& delay, const Duration& period, const Type type)
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

template<class T>
void flib::Scheduler<T>::AsyncProcess(void)
{
  std::mutex waitLock;
  std::unique_lock<decltype(waitLock)> waitGuard(waitLock);
  std::unique_lock<decltype(mConfigurationAccessLock)> configurationGuard(mConfigurationAccessLock, std::defer_lock);
  decltype(mConfiguration) configuration;
  Clock::time_point eventTime;
  while (State::Destruct != mState)
  {
    mWaitCondition.wait(waitGuard, [this]()
      {
        return State::Destruct == mState || State::Activating == mState;
      });
    if (State::Activating != mState)
    {
      continue;
    }
    mState = State::Active;
    configurationGuard.lock();
    configuration = mConfiguration;
    configurationGuard.unlock();
    eventTime = Clock::now() + configuration.delay;
    mWaitCondition.wait_until(waitGuard, eventTime, [this, &eventTime]()
      {
        return eventTime <= Clock::now() || State::Active != mState;
      }
    );
    if (State::Active == mState)
    {
      if (Duration(0) == configuration.period)
      {
        mState = State::Idle;
      }
      configuration.event();
    }
    while (State::Active == mState)
    {
      if (Type::FixedDelay == configuration.type)
      {
        eventTime = Clock::now() + configuration.period;
      }
      else
      {
        eventTime += configuration.period;
      }
      mWaitCondition.wait_until(waitGuard, eventTime, [this, &eventTime]()
        {
          return eventTime <= Clock::now() || State::Active != mState;
        }
      );
      if (State::Active == mState)
      {
        configuration.event();
      }
    }
  }
}