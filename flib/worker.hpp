// Copyright © 2019-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace flib
{
#pragma region API
  class worker;

  class worker_invocation
  {
  public:
    worker_invocation(void);
    worker_invocation(const worker_invocation&) = default;
    worker_invocation(worker_invocation&&) = default;
    virtual ~worker_invocation(void) noexcept = default;
    virtual worker_invocation& operator=(const worker_invocation&) = default;
    virtual worker_invocation& operator=(worker_invocation&&) = default;
    virtual void cancel(void);
    virtual bool expired(void) const;

  protected:
    friend worker;

  protected:
    using token_t = std::weak_ptr<void>;

  protected:
    worker_invocation(worker& p_owner, token_t p_token);

  protected:
    worker* m_owner;
    token_t m_token;
  };

  class worker
  {
  public:
    using priority_t = uint8_t;
    using size_t = std::size_t;
    using task_t = std::function<void(void)>;

  public:
    explicit worker(bool p_enabled = true, size_t p_executors = 1);
    worker(const worker&) = delete;
    worker(worker&&) = delete;
    virtual ~worker(void) noexcept;
    virtual worker& operator=(const worker&) = delete;
    virtual worker& operator=(worker&&) = delete;
    virtual void cancel(const worker_invocation& p_invocation);
    virtual void clear(void);
    virtual void disable(void);
    virtual bool empty(void) const;
    virtual void enable(void);
    virtual bool enabled(void) const;
    virtual size_t executors(void) const;
    virtual worker_invocation invoke(task_t p_task, priority_t p_priority = 0);
    virtual bool owner(const worker_invocation& p_invocation) const;
    virtual size_t size(void) const;

  protected:
    enum class _state
    {
      active,
      destruct
    };

    struct _executor;
    struct _invocation;

  protected:
    virtual bool _condition_check(void) const;
    virtual void _init(_executor& p_executor);
    virtual void _wait(_executor& p_executor);
    virtual void _run(_executor& p_executor);

  protected:
    _state m_state{ _state::destruct };
    std::list<std::shared_ptr<_invocation>> m_invocations;
    std::list<_executor> m_executors;
    std::condition_variable m_condition;
    mutable std::mutex m_mtx;
  };
#pragma endregion

#pragma region IMPLEMENTATION
  inline worker_invocation::worker_invocation(void)
    : m_owner(nullptr)
  {
  }

  inline void worker_invocation::cancel(void)
  {
    if (m_token.expired() || !m_owner)
    {
      return;
    }
    m_owner->cancel(*this);
  }

  inline bool worker_invocation::expired(void) const
  {
    return m_token.expired();
  }

  inline worker_invocation::worker_invocation(worker& p_owner, token_t p_token)
    : m_owner(&p_owner),
    m_token(p_token)
  {
  }

  struct worker::_executor final
  {
    bool m_running{ false };
    std::future<void> m_result;
  };

  struct worker::_invocation final
  {
    task_t m_task;
    priority_t m_priority;
  };

  inline worker::worker(bool p_enabled, size_t p_executors)
  {
    if (0 == p_executors)
    {
      throw std::logic_error("Executorless worker not allowed");
    }
    m_executors.resize(p_executors);
    if (p_enabled)
    {
      enable();
    }
  }

  inline worker::~worker(void) noexcept
  {
    disable();
    for (auto& executor : m_executors)
    {
      _wait(executor);
    }
    clear();
  }

  inline void worker::cancel(const worker_invocation& p_invocation)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_invocations.remove(std::static_pointer_cast<_invocation>(p_invocation.m_token.lock()));
  }

  inline void worker::clear(void)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_invocations.clear();
  }

  inline void worker::disable(void)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_state = _state::destruct;
    guard.unlock();
    m_condition.notify_all();
  }

  inline bool worker::empty(void) const
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return m_invocations.empty();
  }

  inline void worker::enable(void)
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_state = _state::active;
    for (auto& executor : m_executors)
    {
      _init(executor);
    }
    guard.unlock();
    m_condition.notify_all();
  }

  inline bool worker::enabled(void) const
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return _state::active == m_state;
  }

  inline worker::size_t worker::executors(void) const
  {
    return static_cast<size_t>(m_executors.size());
  }

  inline worker_invocation worker::invoke(task_t p_task, priority_t p_priority)
  {
    if (!p_task)
    {
      return {};
    }
    auto invocation_ptr = std::make_shared<_invocation>(_invocation{ std::move(p_task), std::move(p_priority) });
    worker_invocation::token_t token = invocation_ptr;
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    m_invocations.emplace(0 == invocation_ptr->m_priority ?
      m_invocations.cend() :
      std::upper_bound(m_invocations.cbegin(), m_invocations.cend(), invocation_ptr->m_priority,
        [](priority_t p_priority_ref, const std::shared_ptr<_invocation>& p_invocation_el)
        {
          return p_priority_ref > p_invocation_el->m_priority;
        }),
      std::move(invocation_ptr));
    guard.unlock();
    m_condition.notify_one();
    return { *this, token };
  }

  inline bool worker::owner(const worker_invocation& p_invocation) const
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return m_invocations.cend() != std::find(m_invocations.cbegin(), m_invocations.cend(),
      std::static_pointer_cast<_invocation>(p_invocation.m_token.lock()));
  }

  inline worker::size_t worker::size(void) const
  {
    std::unique_lock<decltype(m_mtx)> guard(m_mtx);
    return static_cast<size_t>(m_invocations.size());
  }

  inline bool worker::_condition_check(void) const
  {
    return _state::active != m_state || !m_invocations.empty();
  }

  inline void worker::_init(_executor& p_executor)
  {
    if (!p_executor.m_running)
    {
      p_executor.m_running = true;
      p_executor.m_result = std::async(std::launch::async, &worker::_run, this, std::ref(p_executor));
    }
  }

  inline void worker::_wait(_executor& p_executor)
  {
    if (p_executor.m_result.valid())
    {
      p_executor.m_result.get();
    }
  }

  inline void worker::_run(_executor& p_executor)
  {
    try
    {
      task_t task;
      std::unique_lock<decltype(m_mtx)> guard(m_mtx);
      while (_state::destruct != m_state)
      {
        if (!m_invocations.empty())
        {
          task = std::move(m_invocations.front()->m_task);
          m_invocations.pop_front();
          guard.unlock();
          task();
          guard.lock();
          continue;
        }
        m_condition.wait(guard, std::bind(&worker::_condition_check, this));
      }
      p_executor.m_running = false;
    }
    catch (...)
    {
      std::terminate();
    }
  }
#pragma endregion
}