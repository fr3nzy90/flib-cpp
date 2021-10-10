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

#include <flib/pimpl.hpp>

namespace flib
{
  class worker;

  class worker_invocation
  {
  public:
    worker_invocation(void);
    worker_invocation(const worker_invocation&) = default;
    worker_invocation(worker_invocation&&) = default;
    ~worker_invocation(void) noexcept = default;
    worker_invocation& operator=(const worker_invocation&) = default;
    worker_invocation& operator=(worker_invocation&&) = default;
    void cancel(void);
    bool expired(void) const;

  private:
    friend worker;

    using token_t = std::weak_ptr<void>;

    worker_invocation(worker& owner, token_t token);

    worker* m_owner;
    token_t m_token;
  };

  class worker
  {
  public:
    using priority_t = uint8_t;
    using size_t = std::size_t;
    using task_t = std::function<void(void)>;

    explicit worker(bool enabled = true, size_t executors = 1);
    worker(const worker&) = delete;
    worker(worker&&) = default;
    ~worker(void) noexcept;
    worker& operator=(const worker&) = delete;
    worker& operator=(worker&&) = default;
    void cancel(const worker_invocation& invocation);
    void clear(void);
    void disable(void);
    bool empty(void) const;
    void enable(void);
    bool enabled(void) const;
    size_t executors(void) const;
    worker_invocation invoke(task_t task, priority_t priority = 0);
    bool owner(const worker_invocation& invocation) const;
    size_t size(void) const;

  private:
    enum class _state_t
    {
      active,
      destruct
    };

    struct _executor;
    struct _invocation;
    struct _storage;

    bool _condition_check(void) const;
    void _init(_executor& executor);
    void _wait(_executor& executor);
    void _run(_executor& executor);

    pimpl<_storage> m_storage;
  };

  // IMPLEMENTATION

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

  inline worker_invocation::worker_invocation(worker& owner, token_t token)
    : m_owner(&owner),
    m_token(token)
  {
  }

  struct worker::_executor
  {
    bool running{ false };
    std::future<void> result;
  };

  struct worker::_invocation
  {
    task_t task;
    priority_t priority;
  };

  struct worker::_storage
  {
    _state_t state{ _state_t::destruct };
    std::list<std::shared_ptr<_invocation>> invocations;
    std::list<_executor> executors;
    std::condition_variable condition;
    mutable std::mutex condition_mtx;
  };

  inline worker::worker(bool enabled, size_t executors)
  {
    if (0 == executors)
    {
      throw std::logic_error("Executorless worker not allowed");
    }
    m_storage->executors.resize(executors);
    if (enabled)
    {
      enable();
    }
  }

  inline worker::~worker(void) noexcept
  {
    disable();
    for (auto& executor : m_storage->executors)
    {
      _wait(executor);
    }
    clear();
  }

  inline void worker::cancel(const worker_invocation& invocation)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->invocations.remove(std::static_pointer_cast<_invocation>(invocation.m_token.lock()));
  }

  inline void worker::clear(void)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->invocations.clear();
  }

  inline void worker::disable(void)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->state = _state_t::destruct;
    condition_guard.unlock();
    m_storage->condition.notify_all();
  }

  inline bool worker::empty(void) const
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return m_storage->invocations.empty();
  }

  inline void worker::enable(void)
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->state = _state_t::active;
    for (auto& executor : m_storage->executors)
    {
      _init(executor);
    }
    condition_guard.unlock();
    m_storage->condition.notify_all();
  }

  inline bool worker::enabled(void) const
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return _state_t::active == m_storage->state;
  }

  inline worker::size_t worker::executors(void) const
  {
    return static_cast<size_t>(m_storage->executors.size());
  }

  inline worker_invocation worker::invoke(task_t task, priority_t priority)
  {
    if (!task)
    {
      return {};
    }
    auto invocation_ptr = std::make_shared<_invocation>(_invocation{ std::move(task), std::move(priority) });
    worker_invocation::token_t token = invocation_ptr;
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    m_storage->invocations.emplace(0 == invocation_ptr->priority ?
      m_storage->invocations.cend() :
      std::upper_bound(m_storage->invocations.cbegin(), m_storage->invocations.cend(), invocation_ptr->priority,
        [](priority_t priority_ref, const std::shared_ptr<_invocation>& invocation_el)
        {
          return priority_ref > invocation_el->priority;
        }),
      std::move(invocation_ptr));
    condition_guard.unlock();
    m_storage->condition.notify_one();
    return { *this, token };
  }

  inline bool worker::owner(const worker_invocation& invocation) const
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return m_storage->invocations.cend() != std::find(m_storage->invocations.cbegin(), m_storage->invocations.cend(),
      std::static_pointer_cast<_invocation>(invocation.m_token.lock()));
  }

  inline worker::size_t worker::size(void) const
  {
    std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
    return static_cast<size_t>(m_storage->invocations.size());
  }

  inline bool worker::_condition_check(void) const
  {
    return _state_t::active != m_storage->state || !m_storage->invocations.empty();
  }

  inline void worker::_init(_executor& executor)
  {
    if (!executor.running)
    {
      executor.running = true;
      executor.result = std::async(std::launch::async, &worker::_run, this, std::ref(executor));
    }
  }

  inline void worker::_wait(_executor& executor)
  {
    if (executor.result.valid())
    {
      executor.result.get();
    }
  }

  inline void worker::_run(_executor& executor)
  {
    try
    {
      task_t task;
      std::unique_lock<std::mutex> condition_guard(m_storage->condition_mtx);
      while (_state_t::destruct != m_storage->state)
      {
        if (!m_storage->invocations.empty())
        {
          task = std::move(m_storage->invocations.front()->task);
          m_storage->invocations.pop_front();
          condition_guard.unlock();
          task();
          condition_guard.lock();
          continue;
        }
        m_storage->condition.wait(condition_guard, std::bind(&worker::_condition_check, this));
      }
      executor.running = false;
    }
    catch (...)
    {
      std::terminate();
    }
  }
}