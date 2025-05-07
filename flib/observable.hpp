// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <utility>

namespace flib
{
#pragma region API
  class observable_base;

  class observable_subscription
  {
  public:
    observable_subscription(void) = default;
    virtual ~observable_subscription(void) noexcept = default;
    virtual bool expired(void) const;
    virtual void unsubscribe(void);

  protected:
    friend observable_base;

  protected:
    using token_t = std::weak_ptr<void>;

  protected:
    observable_subscription(observable_base& p_owner, token_t p_token);

  protected:
    observable_base* m_owner{ nullptr };
    token_t m_token;
  };

  class observable_base
  {
  public:
    using size_t = std::size_t;

  public:
    virtual ~observable_base(void) noexcept = default;
    virtual void clear(void);
    virtual bool empty(void) const;
    virtual bool owner(const observable_subscription& p_subscription) const;
    virtual size_t size(void) const;
    virtual void unsubscribe(const observable_subscription& p_subscription);

  protected:
    virtual observable_subscription _make_subscription(observable_subscription::token_t p_token);

  protected:
    std::set<std::shared_ptr<void>> m_subscriptions;
  };

  template<class ...Args>
  class observable
    : public observable_base
  {
  public:
    using observer_t = std::function<void(Args...)>;

  public:
    virtual ~observable(void) noexcept = default;
    virtual void publish(Args... p_args) const;
    virtual observable_subscription subscribe(observer_t p_observer);
  };
#pragma endregion

#pragma region IMPLEMENTATION
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

  inline observable_subscription::observable_subscription(observable_base& p_owner, token_t p_token)
    : m_owner(&p_owner),
    m_token(std::move(p_token))
  {
  }

  inline void observable_base::clear(void)
  {
    m_subscriptions.clear();
  }

  inline bool observable_base::empty(void) const
  {
    return m_subscriptions.empty();
  }

  inline bool observable_base::owner(const observable_subscription& p_subscription) const
  {
    return m_subscriptions.cend() != m_subscriptions.find(p_subscription.m_token.lock());
  }

  inline size_t observable_base::size(void) const
  {
    return static_cast<size_t>(m_subscriptions.size());
  }

  inline void observable_base::unsubscribe(const observable_subscription& p_subscription)
  {
    m_subscriptions.erase(p_subscription.m_token.lock());
  }

  inline observable_subscription observable_base::_make_subscription(observable_subscription::token_t p_token)
  {
    return { *this, p_token };
  }

  template<class ...Args>
  inline void observable<Args...>::publish(Args... p_args) const
  {
    for (const auto& subscription : m_subscriptions)
    {
      (*std::static_pointer_cast<observer_t>(subscription))(p_args...);
    }
  }

  template<class ...Args>
  inline observable_subscription observable<Args...>::subscribe(observer_t p_observer)
  {
    if (!p_observer)
    {
      return {};
    }
    return _make_subscription(*(m_subscriptions.emplace(std::make_shared<observer_t>(std::move(p_observer))).first));
  }
#pragma endregion
}