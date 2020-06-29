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

#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <utility>
#include <stdexcept>

namespace flib
{
  template<class... T>
  class Observable
  {
  public:
    using Observer = std::function<void(const T&...)>;
    using Subscription = std::weak_ptr<void>;

    inline Observable(void) = default;
    inline Observable(const Observable&) = delete;
    inline Observable(Observable&&) = default;
    inline ~Observable(void) = default;
    inline Observable& operator=(const Observable&) = delete;
    inline Observable& operator=(Observable&&) = default;
    inline void Clear(void);
    inline bool IsEmpty(void) const;
    inline bool IsSubscribed(const Subscription& subscription) const;
    inline void Publish(const T& ...args) const;
    inline Subscription Subscribe(const Observer& observer);
    inline std::size_t SubscriptionCount(void) const;
    inline void Unsubscribe(const Subscription& subscription);

  private:
    std::set<std::shared_ptr<Observer>> mSubscriptions;
    mutable std::mutex mSubscriptionsAccessLock;
  };
}

// IMPLEMENTATION

template<class... T>
void flib::Observable<T...>::Clear(void)
{
  std::lock_guard<decltype(mSubscriptionsAccessLock)> subscriptionsAccessGuard(mSubscriptionsAccessLock);
  mSubscriptions.clear();
}

template<class... T>
bool flib::Observable<T...>::IsEmpty(void) const
{
  std::lock_guard<decltype(mSubscriptionsAccessLock)> subscriptionsAccessGuard(mSubscriptionsAccessLock);
  return mSubscriptions.empty();
}

template<class... T>
bool flib::Observable<T...>::IsSubscribed(const Subscription& subscription) const
{
  std::lock_guard<decltype(mSubscriptionsAccessLock)> subscriptionsAccessGuard(mSubscriptionsAccessLock);
  return mSubscriptions.cend() != mSubscriptions.find(std::static_pointer_cast<Observer>(subscription.lock()));
}

template<class... T>
void flib::Observable<T...>::Publish(const T& ...args) const
{
  std::unique_lock<decltype(mSubscriptionsAccessLock)> subscriptionsAccessGuard(mSubscriptionsAccessLock);
  auto subscriptions = mSubscriptions;
  subscriptionsAccessGuard.unlock();
  for (const auto& subscription : subscriptions)
  {
    (*subscription)(std::forward<const T&>(args)...);
  }
}

template<class... T>
typename flib::Observable<T...>::Subscription flib::Observable<T...>::Subscribe(const Observer& observer)
{
  if (!observer)
  {
    throw std::invalid_argument("Invalid observer");
  }
  std::lock_guard<decltype(mSubscriptionsAccessLock)> subscriptionsAccessGuard(mSubscriptionsAccessLock);
  return *mSubscriptions.emplace(std::make_shared<Observer>(observer)).first;
}

template<class... T>
std::size_t flib::Observable<T...>::SubscriptionCount(void) const
{
  std::lock_guard<decltype(mSubscriptionsAccessLock)> subscriptionsAccessGuard(mSubscriptionsAccessLock);
  return mSubscriptions.size();
}

template<class... T>
void flib::Observable<T...>::Unsubscribe(const Subscription& subscription)
{
  std::lock_guard<decltype(mSubscriptionsAccessLock)> subscriptionsAccessGuard(mSubscriptionsAccessLock);
  mSubscriptions.erase(std::static_pointer_cast<Observer>(subscription.lock()));
}