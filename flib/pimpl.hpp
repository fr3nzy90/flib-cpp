/*
* MIT License
*
* Copyright (c) 2020 Luka Arnecic
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

#include <memory>
#include <utility>

namespace flib
{
  template<class T>
  class pimpl
  {
  public:
    template<class ...Args>
    inline pimpl(Args&& ...args);
    pimpl(const pimpl&) = delete;
    pimpl(pimpl&&) = default;
    ~pimpl(void) = default;
    pimpl& operator=(const pimpl&) = delete;
    pimpl& operator=(pimpl&&) = default;
    inline T* operator->(void);
    inline const T* operator->(void) const;
    inline T& operator*(void);
    inline const T& operator*(void) const;
    inline T* get(void);
    inline const T* get(void) const;

  private:
    std::unique_ptr<T> m_impl;
  };

  // IMPLEMENTATION

  template<class T>
  template<class ...Args>
  pimpl<T>::pimpl(Args&& ...args)
    : m_impl{ new T{ std::forward<Args>(args)... } }
  {
  }

  template<class T>
  T* pimpl<T>::operator->(void)
  {
    return get();
  }

  template<class T>
  const T* pimpl<T>::operator->(void) const
  {
    return get();
  }

  template<class T>
  T& pimpl<T>::operator*(void)
  {
    return *get();
  }

  template<class T>
  const T& pimpl<T>::operator*(void) const
  {
    return *get();
  }

  template<class T>
  T* pimpl<T>::get(void)
  {
    return m_impl.get();
  }

  template<class T>
  const T* pimpl<T>::get(void) const
  {
    return m_impl.get();
  }
}