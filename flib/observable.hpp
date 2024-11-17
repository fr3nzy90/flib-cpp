// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

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
  class observable_base;

  class observable_subscription
  {
  public:
    observable_subscription(void);
    observable_subscription(const observable_subscription&) = default;
    observable_subscription(observable_subscription&&) = default;
    ~observable_subscription(void) noexcept = default;
    observable_subscription& operator=(const observable_subscription&) = default;
    observable_subscription& operator=(observable_subscription&&) = default;
    bool expired(void) const;
    void unsubscribe(void);

  private:
    friend observable_base;

    using token_t = std::weak_ptr<void>;

    observable_subscription(observable_base& owner, token_t token);

    observable_base* m_owner;
    token_t m_token;
  };

  class observable_base
  {
  public:
    using size_t = std::size_t;

    observable_base(const observable_base&) = delete;
    observable_base& operator=(const observable_base&) = delete;
    void clear(void);
    bool empty(void) const;
    bool owner(const observable_subscription& subscription) const;
    size_t size(void) const;
    void unsubscribe(const observable_subscription& subscription);

  protected:
    struct _storage;

    observable_base(void) = default;
    observable_base(observable_base&&) = default;
    ~observable_base(void) noexcept = default;
    observable_base& operator=(observable_base&&) = default;
    observable_subscription _create(observable_subscription::token_t token);

    pimpl<_storage> m_storage;
  };

  template<class ...Args>
  class observable
    : public observable_base
  {
  public:
    using observer_t = std::function<void(Args...)>;

    observable(void) = default;
    observable(const observable&) = delete;
    observable(observable&&) = default;
    ~observable(void) noexcept;
    observable& operator=(const observable&) = delete;
    observable& operator=(observable&&) = default;
    void publish(Args... args) const;
    observable_subscription subscribe(observer_t observer);
  };

  // IMPLEMENTATION

  inline observable_subscription::observable_subscription(void)
    : m_owner(nullptr)
  {
  }

  inline bool observable_subscription::expired(void) const
  {
    return m_token.expired();
  }

  inline void observable_subscription::unsubscribe(void)
  {
    if (m_token.expired() || !m_owner)
    {
      return;
    }
    m_owner->unsubscribe(*this);
  }

  inline observable_subscription::observable_subscription(observable_base& owner, token_t token)
    : m_owner(&owner),
    m_token(token)
  {
  }

  struct observable_base::_storage
  {
    std::set<std::shared_ptr<void>> subscriptions;
    mutable std::mutex subscriptions_mtx;
  };

  inline void observable_base::clear(void)
  {
    std::unique_lock<std::mutex> subscriptions_guard(m_storage->subscriptions_mtx);
    m_storage->subscriptions.clear();
  }

  inline bool observable_base::empty(void) const
  {
    std::unique_lock<std::mutex> subscriptions_guard(m_storage->subscriptions_mtx);
    return m_storage->subscriptions.empty();
  }

  inline bool observable_base::owner(const observable_subscription& subscription) const
  {
    std::unique_lock<std::mutex> subscriptions_guard(m_storage->subscriptions_mtx);
    return m_storage->subscriptions.cend() != m_storage->subscriptions.find(subscription.m_token.lock());
  }

  inline size_t observable_base::size(void) const
  {
    std::unique_lock<std::mutex> subscriptions_guard(m_storage->subscriptions_mtx);
    return static_cast<size_t>(m_storage->subscriptions.size());
  }

  inline void observable_base::unsubscribe(const observable_subscription& subscription)
  {
    std::unique_lock<std::mutex> subscriptions_guard(m_storage->subscriptions_mtx);
    m_storage->subscriptions.erase(subscription.m_token.lock());
  }

  inline observable_subscription observable_base::_create(observable_subscription::token_t token)
  {
    return { *this, token };
  }

  template<class ...Args>
  inline observable<Args...>::~observable(void) noexcept
  {
    clear();
  }

  template<class ...Args>
  inline void observable<Args...>::publish(Args... args) const
  {
    std::unique_lock<std::mutex> subscriptions_guard(m_storage->subscriptions_mtx);
    auto subscriptions = m_storage->subscriptions;
    subscriptions_guard.unlock();
    for (const auto& subscription : subscriptions)
    {
      (*std::static_pointer_cast<observer_t>(subscription))(args...);
    }
  }

  template<class ...Args>
  inline observable_subscription observable<Args...>::subscribe(observer_t observer)
  {
    if (!observer)
    {
      return {};
    }
    auto subscription = std::make_shared<observer_t>(std::move(observer));
    std::unique_lock<std::mutex> subscriptions_guard(m_storage->subscriptions_mtx);
    return _create(*(m_storage->subscriptions.emplace(std::move(subscription)).first));
  }
}