// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <utility>

namespace flib
{
  template<class T>
  class pimpl
  {
  public:
    template<class ...Args>
    pimpl(Args&& ...args);
    pimpl(const pimpl&) = delete;
    pimpl(pimpl&&) = default;
    ~pimpl(void);
    pimpl& operator=(const pimpl&) = delete;
    pimpl& operator=(pimpl&&) = default;
    T* operator->(void);
    const T* operator->(void) const;
    T& operator*(void);
    const T& operator*(void) const;
    T* get(void);
    const T* get(void) const;

  private:
    T* m_impl;
  };

  // IMPLEMENTATION

  template<class T>
  template<class ...Args>
  inline pimpl<T>::pimpl(Args&& ...args)
    : m_impl{ new T{ std::forward<Args>(args)... } }
  {
  }

  template<class T>
  inline pimpl<T>::~pimpl(void)
  {
    delete m_impl;
  }

  template<class T>
  inline T* pimpl<T>::operator->(void)
  {
    return get();
  }

  template<class T>
  inline const T* pimpl<T>::operator->(void) const
  {
    return get();
  }

  template<class T>
  inline T& pimpl<T>::operator*(void)
  {
    return *get();
  }

  template<class T>
  inline const T& pimpl<T>::operator*(void) const
  {
    return *get();
  }

  template<class T>
  inline T* pimpl<T>::get(void)
  {
    return m_impl;
  }

  template<class T>
  inline const T* pimpl<T>::get(void) const
  {
    return m_impl;
  }
}