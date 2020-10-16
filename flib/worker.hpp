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
  class worker
  {
  public:
    using priority_t = uint8_t;
    using size_t = std::size_t;
    using task_t = std::function<void(void)>;

    class invocation_t;

    explicit inline worker(bool enabled = true, size_t executors = 1);
    worker(const worker&) = delete;
    worker(worker&&) = default;
    ~worker(void) noexcept = default;
    worker& operator=(const worker&) = delete;
    worker& operator=(worker&&) = default;
    inline void cancel(const invocation_t& invocation);
    inline void clear(void);
    inline void disable(void);
    inline bool empty(void) const;
    inline void enable(void);
    inline bool enabled(void) const;
    inline size_t executors(void) const;
    inline invocation_t invoke(task_t task, priority_t priority = 0);
    inline bool owner(const invocation_t& invocation) const;
    inline size_t size(void) const;

  private:
    class _impl;
    pimpl<_impl> m_impl;
  };

  class worker::invocation_t
  {
  public:
    inline invocation_t(void);
    inline void cancel(void); // It is only safe to invoke method if the owner is not being destructed.
    inline bool expired(void) const;

  private:
    friend worker;

    using token_t = std::weak_ptr<void>;

    inline invocation_t(worker& owner, token_t token);

    worker* m_owner;
    token_t m_token;
  };

  // IMPLEMENTATION

  class worker::_impl
  {
  public:
    using token_t = invocation_t::token_t;

    struct _invocation_t
    {
      task_t task;
      priority_t priority;
    };

    inline _impl(bool enabled, size_t executors);
    inline ~_impl(void) noexcept;
    inline void _cancel(const token_t& token);
    inline void _clear(void);
    inline void _disable(void);
    inline bool _empty(void) const;
    inline void _enable(void);
    inline bool _enabled(void) const;
    inline size_t _executors(void) const;
    inline token_t _invoke(_invocation_t invocation);
    inline bool _owner(const token_t& token) const;
    inline size_t _size(void) const;

  private:
    enum class __state_t
    {
      active,
      destruct
    };

    struct __executor_t
    {
      bool running = false;
      std::future<void> result;
    };

    inline bool __condition_check(void) const;
    inline void __init(__executor_t& executor);
    inline void __wait(__executor_t& executor);
    inline void __run(__executor_t& executor);

    __state_t m_state;
    std::list<std::shared_ptr<_invocation_t>> m_invocations;
    std::list<__executor_t> m_executors;
    std::condition_variable m_condition;
    mutable std::mutex m_condition_mtx;
  };

  worker::_impl::_impl(bool enabled, size_t executors)
    : m_state{ __state_t::destruct },
    m_executors(executors)
  {
    if (0 == executors)
    {
      throw std::logic_error("Executorless worker not allowed");
    }
    if (enabled)
    {
      _enable();
    }
  }

  worker::_impl::~_impl(void) noexcept
  {
    _disable();
    for (auto& executor : m_executors)
    {
      __wait(executor);
    }
  }

  void worker::_impl::_cancel(const token_t& token)
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    m_invocations.remove(std::static_pointer_cast<_invocation_t>(token.lock()));
  }

  void worker::_impl::_clear(void)
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    m_invocations.clear();
  }

  void worker::_impl::_disable(void)
  {
    {
      std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
      m_state = __state_t::destruct;
    }
    m_condition.notify_all();
  }

  bool worker::_impl::_empty(void) const
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    return m_invocations.empty();
  }

  void worker::_impl::_enable(void)
  {
    {
      std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
      m_state = __state_t::active;
      for (auto& executor : m_executors)
      {
        __init(executor);
      }
    }
    m_condition.notify_all();
  }

  bool worker::_impl::_enabled(void) const
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    return __state_t::active == m_state;
  }

  worker::size_t worker::_impl::_executors(void) const
  {
    return static_cast<size_t>(m_executors.size());
  }

  worker::_impl::token_t worker::_impl::_invoke(_invocation_t invocation)
  {
    if (!invocation.task)
    {
      return {};
    }
    auto invocation_ptr = std::make_shared<_invocation_t>(std::move(invocation));
    worker::_impl::token_t token = invocation_ptr;
    {
      std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
      m_invocations.emplace(0 == invocation_ptr->priority ?
        m_invocations.cend() :
        std::upper_bound(m_invocations.cbegin(), m_invocations.cend(), invocation_ptr->priority,
          [](priority_t priority, const std::shared_ptr<_invocation_t>& invocation_el)
          {
            return priority > invocation_el->priority;
          }),
        std::move(invocation_ptr));
    }
    m_condition.notify_one();
    return token;
  }

  bool worker::_impl::_owner(const token_t& token) const
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    return m_invocations.cend() != std::find(m_invocations.cbegin(), m_invocations.cend(),
      std::static_pointer_cast<_invocation_t>(token.lock()));
  }

  worker::size_t worker::_impl::_size(void) const
  {
    std::lock_guard<std::mutex> condition_guard(m_condition_mtx);
    return static_cast<size_t>(m_invocations.size());
  }

  bool worker::_impl::__condition_check(void) const
  {
    return __state_t::active != m_state || !m_invocations.empty();
  }

  void worker::_impl::__init(__executor_t& executor)
  {
    if (!executor.running)
    {
      executor.running = true;
      executor.result = std::async(std::launch::async, &_impl::__run, this, std::ref(executor));
    }
  }

  void worker::_impl::__wait(__executor_t& executor)
  {
    if (executor.result.valid())
    {
      executor.result.get();
    }
  }

  void worker::_impl::__run(__executor_t& executor)
  {
    try
    {
      task_t task;
      std::unique_lock<std::mutex> condition_guard(m_condition_mtx);
      while (__state_t::destruct != m_state)
      {
        if (!m_invocations.empty())
        {
          task = std::move(m_invocations.front()->task);
          m_invocations.pop_front();
          condition_guard.unlock();
          task();
          condition_guard.lock();
          continue;
        }
        m_condition.wait(condition_guard, std::bind(&_impl::__condition_check, this));
      }
      executor.running = false;
    }
    catch (...)
    {
      std::terminate();
    }
  }

  worker::invocation_t::invocation_t(void)
    : m_owner(nullptr)
  {
  }

  void worker::invocation_t::cancel(void)
  {
    if (m_token.expired() || !m_owner)
    {
      return;
    }
    m_owner->cancel(*this);
  }

  bool worker::invocation_t::expired(void) const
  {
    return m_token.expired();
  }

  worker::invocation_t::invocation_t(worker& owner, token_t token)
    : m_owner(&owner),
    m_token(token)
  {
  }

  worker::worker(bool enabled, size_t executors)
    : m_impl(enabled, executors)
  {
  }

  void worker::cancel(const invocation_t& invocation)
  {
    m_impl->_cancel(invocation.m_token);
  }

  void worker::clear(void)
  {
    m_impl->_clear();
  }

  void worker::disable(void)
  {
    m_impl->_disable();
  }

  bool worker::empty(void) const
  {
    return m_impl->_empty();
  }

  void worker::enable(void)
  {
    m_impl->_enable();
  }

  bool worker::enabled(void) const
  {
    return m_impl->_enabled();
  }

  worker::size_t worker::executors(void) const
  {
    return m_impl->_executors();
  }

  worker::invocation_t worker::invoke(task_t task, priority_t priority)
  {
    return { *this, m_impl->_invoke({ std::move(task), std::move(priority) }) };
  }

  bool worker::owner(const invocation_t& invocation) const
  {
    return m_impl->_owner(invocation.m_token);
  }

  worker::size_t worker::size(void) const
  {
    return m_impl->_size();
  }
}