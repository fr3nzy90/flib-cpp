// Copyright © 2019-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
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
    virtual void clear(void) = 0;
    virtual bool empty(void) const = 0;
    virtual bool owner(const observable_subscription& p_subscription) const = 0;
    virtual size_t size(void) const = 0;
    virtual void unsubscribe(const observable_subscription& p_subscription) = 0;

  protected:
    using subscription_token_t = std::shared_ptr<void>;

    struct _subscription_t
    {
    public:
      subscription_token_t token;

    public:
      _subscription_t(void);
      bool operator==(const observable_subscription& p_subscription) const;
    };

  protected:
    observable_base(void) = default;
    virtual observable_subscription _make_subscription(const _subscription_t& p_subscription);
  };

  template<class ...Args>
  class observable
    : public observable_base
  {
  public:
    using observer_t = std::function<void(Args...)>;

  public:
    virtual ~observable(void) noexcept = default;
    virtual void clear(void) override;
    virtual bool empty(void) const override;
    virtual bool owner(const observable_subscription& p_subscription) const override;
    virtual void publish(Args... p_args) const;
    virtual size_t size(void) const override;
    virtual observable_subscription subscribe(observer_t p_observer);
    virtual void unsubscribe(const observable_subscription& p_subscription) override;

  protected:
    struct _subscription_t
      : public observable_base::_subscription_t
    {
    public:
      observer_t observer;

    public:
      _subscription_t(observer_t p_observer);
    };

  protected:
    std::list<_subscription_t> m_subscriptions;
  };
#pragma endregion

#pragma region IMPLEMENTATION
#  pragma region observable_subscription IMPLEMENTATION
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
#  pragma endregion

#  pragma region observable_base IMPLEMENTATION
  inline observable_subscription observable_base::_make_subscription(const _subscription_t& p_subscription)
  {
    return { *this, p_subscription.token };
  }
#  pragma endregion

#  pragma region observable_base::_subscription_t IMPLEMENTATION
  inline observable_base::_subscription_t::_subscription_t(void)
    : token{ std::make_shared<bool>() }
  {
  }

  inline bool observable_base::_subscription_t::operator==(const observable_subscription& p_subscription) const
  {
    return p_subscription.m_token.lock() == token;
  }
#  pragma endregion

#  pragma region observable<Args...>::_subscription_t IMPLEMENTATION
  template<class ...Args>
  inline observable<Args...>::_subscription_t::_subscription_t(observer_t p_observer)
    : observer{ std::move(p_observer) }
  {
  }
#  pragma endregion

#  pragma region observable<Args...> IMPLEMENTATION
  template<class ...Args>
  inline void observable<Args...>::clear(void)
  {
    m_subscriptions.clear();
  }

  template<class ...Args>
  inline bool observable<Args...>::empty(void) const
  {
    return m_subscriptions.empty();
  }

  template<class ...Args>
  inline bool observable<Args...>::owner(const observable_subscription& p_subscription) const
  {
    return m_subscriptions.cend() != std::find(m_subscriptions.cbegin(), m_subscriptions.cend(), p_subscription);
  }

  template<class ...Args>
  inline void observable<Args...>::publish(Args... p_args) const
  {
    for (const auto& subscription : m_subscriptions)
    {
      subscription.observer(p_args...);
    }
  }

  template<class ...Args>
  inline size_t observable<Args...>::size(void) const
  {
    return static_cast<size_t>(m_subscriptions.size());
  }

  template<class ...Args>
  inline observable_subscription observable<Args...>::subscribe(observer_t p_observer)
  {
    if (!p_observer)
    {
      return {};
    }
    m_subscriptions.push_back(_subscription_t(std::move(p_observer)));
    return _make_subscription(m_subscriptions.back());
  }

  template<class ...Args>
  inline void observable<Args...>::unsubscribe(const observable_subscription& p_subscription)
  {
    m_subscriptions.erase(std::remove(m_subscriptions.begin(), m_subscriptions.end(), p_subscription), m_subscriptions.cend());
  }
#  pragma endregion
#pragma endregion
}