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
  class observable
  {
  public:
    using observer_t = std::function<void(const T&...)>;
    using subscription_t = std::weak_ptr<void>;

    inline observable(void) = default;
    inline observable(const observable&) = delete;
    inline observable(observable&&) = default;
    inline ~observable(void) = default;
    inline observable& operator=(const observable&) = delete;
    inline observable& operator=(observable&&) = default;
    inline observable& clear(void);
    inline bool empty(void) const;
    inline observable& publish(const T& ...args);
    inline std::size_t size(void) const;
    inline subscription_t subscribe(const observer_t& observer);
    inline subscription_t subscribe(observer_t&& observer);
    inline bool subscribed(const subscription_t& subscription) const;
    inline observable& unsubscribe(const subscription_t& subscription);

  private:
    std::set<std::shared_ptr<observer_t>> m_subscriptions;
    mutable std::mutex m_subscriptions_mtx;
  };
}

// IMPLEMENTATION

template<class... T>
flib::observable<T...>& flib::observable<T...>::clear(void)
{
  std::lock_guard<decltype(m_subscriptions_mtx)> subscriptions_guard(m_subscriptions_mtx);
  m_subscriptions.clear();
  return *this;
}

template<class... T>
bool flib::observable<T...>::empty(void) const
{
  std::lock_guard<decltype(m_subscriptions_mtx)> subscriptions_guard(m_subscriptions_mtx);
  return m_subscriptions.empty();
}

template<class... T>
flib::observable<T...>& flib::observable<T...>::publish(const T& ...args)
{
  std::unique_lock<decltype(m_subscriptions_mtx)> subscriptions_guard(m_subscriptions_mtx);
  auto subscriptions = m_subscriptions;
  subscriptions_guard.unlock();
  for (const auto& subscription : subscriptions)
  {
    (*subscription)(std::forward<const T&>(args)...);
  }
  return *this;
}

template<class... T>
std::size_t flib::observable<T...>::size(void) const
{
  std::lock_guard<decltype(m_subscriptions_mtx)> subscriptions_guard(m_subscriptions_mtx);
  return m_subscriptions.size();
}

template<class... T>
typename flib::observable<T...>::subscription_t flib::observable<T...>::subscribe(const observer_t& observer)
{
  return subscribe(observer_t(observer));
}

template<class... T>
typename flib::observable<T...>::subscription_t flib::observable<T...>::subscribe(observer_t&& observer)
{
  if (!observer)
  {
    throw std::invalid_argument("Invalid observer");
  }
  std::lock_guard<decltype(m_subscriptions_mtx)> subscriptions_guard(m_subscriptions_mtx);
  return *m_subscriptions.emplace(std::make_shared<observer_t>(std::move(observer))).first;
}

template<class... T>
bool flib::observable<T...>::subscribed(const subscription_t& subscription) const
{
  std::lock_guard<decltype(m_subscriptions_mtx)> subscriptions_guard(m_subscriptions_mtx);
  return m_subscriptions.cend() != m_subscriptions.find(std::static_pointer_cast<observer_t>(subscription.lock()));
}

template<class... T>
flib::observable<T...>& flib::observable<T...>::unsubscribe(const subscription_t& subscription)
{
  std::lock_guard<decltype(m_subscriptions_mtx)> subscriptions_guard(m_subscriptions_mtx);
  m_subscriptions.erase(std::static_pointer_cast<observer_t>(subscription.lock()));
  return *this;
}