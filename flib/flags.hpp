// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <type_traits>

namespace flib
{
#pragma region API
  template<class T>
  using is_enum_t = typename std::enable_if<std::is_enum<T>::value>::type;

  template<class T, class = is_enum_t<T>>
  constexpr T operator~(T p_flag);

  template<class T, class = is_enum_t<T>>
  constexpr T operator&(T p_flag_1, T p_flag_2);

  template<class T, class = is_enum_t<T>>
  constexpr T operator|(T p_flag_1, T p_flag_2);

  template<class T, class = is_enum_t<T>>
  constexpr T operator^(T p_flag_1, T p_flag_2);

  template<class T, class = is_enum_t<T>>
  constexpr T& operator&=(T& p_flag_1, T p_flag_2);

  template<class T, class = is_enum_t<T>>
  constexpr T& operator|=(T& p_flag_1, T p_flag_2);

  template<class T, class = is_enum_t<T>>
  constexpr T& operator^=(T& p_flag_1, T p_flag_2);

  template<class T, class = is_enum_t<T>>
  constexpr bool is_flag_set(T p_value, T p_flag);
#pragma endregion

#pragma region IMPLEMENTATION
  template<class T, class = is_enum_t<T>>
  constexpr typename std::underlying_type<T>::type _underlying_value(T p_flag)
  {
    return static_cast<typename std::underlying_type<T>::type>(p_flag);
  }

  template<class T, class>
  constexpr T operator~(T p_flag)
  {
    return static_cast<T>(~_underlying_value(p_flag));
  }

  template<class T, class>
  constexpr T operator&(T p_flag_1, T p_flag_2)
  {
    return static_cast<T>(_underlying_value(p_flag_1) & _underlying_value(p_flag_2));
  }

  template<class T, class>
  constexpr T operator|(T p_flag_1, T p_flag_2)
  {
    return static_cast<T>(_underlying_value(p_flag_1) | _underlying_value(p_flag_2));
  }

  template<class T, class>
  constexpr T operator^(T p_flag_1, T p_flag_2)
  {
    return static_cast<T>(_underlying_value(p_flag_1) ^ _underlying_value(p_flag_2));
  }

  template<class T, class>
  constexpr T& operator&=(T& p_flag_1, T p_flag_2)
  {
    p_flag_1 = p_flag_1 & p_flag_2;
    return p_flag_1;
  }

  template<class T, class>
  constexpr T& operator|=(T& p_flag_1, T p_flag_2)
  {
    p_flag_1 = p_flag_1 | p_flag_2;
    return p_flag_1;
  }

  template<class T, class>
  constexpr T& operator^=(T& p_flag_1, T p_flag_2)
  {
    p_flag_1 = p_flag_1 ^ p_flag_2;
    return p_flag_1;
  }

  template<class T, class>
  constexpr bool is_flag_set(T p_value, T p_flag)
  {
    return (p_value & p_flag) == p_flag;
  }
#pragma endregion
}