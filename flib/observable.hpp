// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
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
    bool expired(void) const;
    void unsubscribe(void);

  private:
    friend observable_base;

  private:
    using token_t = std::weak_ptr<void>;

  private:
    observable_subscription(observable_base& p_owner, token_t p_token);

  private:
    observable_base* m_owner{ nullptr };
    token_t m_token;
  };

  class observable_base
  {
  public:
    using size_t = std::size_t;

  public:
    virtual ~observable_base(void) = default;
    virtual void clear(void);
    virtual bool empty(void) const;
    virtual bool owner(const observable_subscription& p_subscription) const;
    virtual size_t size(void) const;
    virtual void unsubscribe(const observable_subscription& p_subscription);

  protected:
    virtual observable_subscription _create(observable_subscription::token_t p_token);

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
    void publish(Args... p_args) const;
    observable_subscription subscribe(observer_t p_observer);
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
    m_token(p_token)
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

  inline observable_subscription observable_base::_create(observable_subscription::token_t p_token)
  {
    return { *this, p_token };
  }

  template<class ...Args>
  inline void observable<Args...>::publish(Args... p_args) const
  {
    auto subscriptions = m_subscriptions;
    for (const auto& subscription : subscriptions)
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
    return _create(*(m_subscriptions.emplace(std::make_shared<observer_t>(std::move(p_observer))).first));
  }
#pragma endregion
}