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

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <utility>

#include <flib/pimpl.hpp>

namespace flib
{
  template<class ...Args>
  class observable
  {
  public:
    using observer_t = std::function<void(Args...)>;
    using size_t = std::size_t;

    class subscription_t;

    observable(void) = default;
    observable(const observable&) = delete;
    observable(observable&&) = default;
    ~observable(void) noexcept = default;
    observable& operator=(const observable&) = delete;
    observable& operator=(observable&&) = default;
    inline void clear(void);
    inline bool empty(void) const;
    inline bool owner(const subscription_t& subscription) const;
    inline void publish(Args... args) const;
    inline size_t size(void) const;
    inline subscription_t subscribe(observer_t observer);
    inline void unsubscribe(const subscription_t& subscription);

  private:
    class _impl;
    pimpl<_impl> m_impl;
  };

  template<class ...Args>
  class observable<Args...>::subscription_t
  {
  public:
    inline subscription_t(void);
    inline bool expired(void) const;
    inline void unsubscribe(void); // It is only safe to invoke method if the owner is not being destructed.

  private:
    friend observable<Args...>;

    using token_t = std::weak_ptr<void>;

    inline subscription_t(observable<Args...>& owner, token_t token);

    observable<Args...>* m_owner;
    token_t m_token;
  };

  // IMPLEMENTATION

  template<class ...Args>
  class observable<Args...>::_impl
  {
  public:
    using token_t = typename subscription_t::token_t;

    inline void _clear(void);
    inline bool _empty(void) const;
    inline bool _owner(const token_t& subscription) const;
    inline void _publish(Args... args) const;
    inline size_t _size(void) const;
    inline token_t _subscribe(observer_t observer);
    inline void _unsubscribe(const token_t& subscription);

  private:
    std::set<std::shared_ptr<observer_t>> m_subscriptions;
    mutable std::mutex m_subscriptions_mtx;
  };

  template<class ...Args>
  void observable<Args...>::_impl::_clear(void)
  {
    std::lock_guard<std::mutex> subscriptions_guard(m_subscriptions_mtx);
    m_subscriptions.clear();
  }

  template<class ...Args>
  bool observable<Args...>::_impl::_empty(void) const
  {
    std::lock_guard<std::mutex> subscriptions_guard(m_subscriptions_mtx);
    return m_subscriptions.empty();
  }

  template<class ...Args>
  bool observable<Args...>::_impl::_owner(const token_t& token) const
  {
    std::lock_guard<std::mutex> subscriptions_guard(m_subscriptions_mtx);
    return m_subscriptions.cend() != m_subscriptions.find(std::static_pointer_cast<observer_t>(token.lock()));
  }

  template<class ...Args>
  void observable<Args...>::_impl::_publish(Args... args) const
  {
    std::unique_lock<std::mutex> subscriptions_guard(m_subscriptions_mtx);
    auto subscriptions = m_subscriptions;
    subscriptions_guard.unlock();
    for (const auto& subscription : subscriptions)
    {
      (*subscription)(args...);
    }
  }

  template<class ...Args>
  typename observable<Args...>::size_t observable<Args...>::_impl::_size(void) const
  {
    std::lock_guard<std::mutex> subscriptions_guard(m_subscriptions_mtx);
    return static_cast<size_t>(m_subscriptions.size());
  }

  template<class ...Args>
  typename observable<Args...>::_impl::token_t observable<Args...>::_impl::_subscribe(observer_t observer)
  {
    if (!observer)
    {
      return {};
    }
    auto subscription = std::make_shared<observer_t>(std::move(observer));
    std::lock_guard<std::mutex> subscriptions_guard(m_subscriptions_mtx);
    return *m_subscriptions.emplace(std::move(subscription)).first;
  }

  template<class ...Args>
  void observable<Args...>::_impl::_unsubscribe(const token_t& token)
  {
    std::lock_guard<std::mutex> subscriptions_guard(m_subscriptions_mtx);
    m_subscriptions.erase(std::static_pointer_cast<observer_t>(token.lock()));
  }

  template<class ...Args>
  observable<Args...>::subscription_t::subscription_t(void)
    : m_owner(nullptr)
  {
  }

  template<class ...Args>
  bool observable<Args...>::subscription_t::expired(void) const
  {
    return m_token.expired();
  }

  template<class ...Args>
  void observable<Args...>::subscription_t::unsubscribe(void)
  {
    if (m_token.expired() || !m_owner)
    {
      return;
    }
    m_owner->unsubscribe(*this);
  }

  template<class ...Args>
  observable<Args...>::subscription_t::subscription_t(observable<Args...>& owner, token_t token)
    : m_owner(&owner),
    m_token(token)
  {
  }

  template<class ...Args>
  void observable<Args...>::clear(void)
  {
    m_impl->_clear();
  }

  template<class ...Args>
  bool observable<Args...>::empty(void) const
  {
    return m_impl->_empty();
  }

  template<class ...Args>
  bool observable<Args...>::owner(const subscription_t& subscription) const
  {
    return m_impl->_owner(subscription.m_token);
  }

  template<class ...Args>
  void observable<Args...>::publish(Args... args) const
  {
    m_impl->_publish(std::forward<Args>(args)...);
  }

  template<class ...Args>
  size_t observable<Args...>::size(void) const
  {
    return m_impl->_size();
  }

  template<class ...Args>
  typename observable<Args...>::subscription_t observable<Args...>::subscribe(observer_t observer)
  {
    return { *this, m_impl->_subscribe(std::move(observer)) };
  }

  template<class ...Args>
  void observable<Args...>::unsubscribe(const subscription_t& subscription)
  {
    m_impl->_unsubscribe(subscription.m_token);
  }
}