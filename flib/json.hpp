// Copyright © 2022-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace flib
{
  class json
  {
  public:
    enum class value_t
    {
      null,
      boolean,
      number_int,
      number_uint,
      number_float,
      string,
      array,
      object
    };

    using null_t = std::nullptr_t;
    using boolean_t = bool;
    using number_int_t = int64_t;
    using number_uint_t = uint64_t;
    using number_float_t = double;
    using string_t = std::string;
    using array_t = std::vector<json>;
    using object_t = std::map<string_t, json>;

    template<value_t>
    struct type_t;

    constexpr json(null_t value = nullptr) noexcept;
    json(boolean_t value) noexcept;
    json(number_int_t value) noexcept;
    json(number_uint_t value) noexcept;
    json(number_float_t value) noexcept;
    json(string_t value);
    json(array_t value);
    json(object_t value);
    template<class T>
    json(T value);
    json(const json& object);
    json(json&& object) noexcept;
    ~json(void) noexcept;
    json& operator=(json object);
    bool operator==(const json& object) const;
    bool operator!=(const json& object) const;
    const json& at(array_t::size_type index) const;
    json& at(array_t::size_type index);
    const json& at(const object_t::key_type& key) const;
    json& at(const object_t::key_type& key);
    template<class ...T>
    const json& at(array_t::size_type index, T... args) const;
    template<class ...T>
    json& at(array_t::size_type index, T... args);
    template<class ...T>
    const json& at(const object_t::key_type& key, T... args) const;
    template<class ...T>
    json& at(const object_t::key_type& key, T... args);
    void clear(void);
    template<value_t E>
    const typename type_t<E>::type& get() const;
    template<value_t E>
    typename type_t<E>::type& get();
    void swap(json& object);
    value_t type(void) const noexcept;

  private:
    template<class T, typename = void>
    struct _compatible_value;

    value_t m_type;
    union
    {
      boolean_t boolean;
      number_int_t number_int;
      number_uint_t number_uint;
      number_float_t number_float;
      string_t* string;
      array_t* array;
      object_t* object;
    } m_value;
  };

  // IMPLEMENTATION

  template<>
  struct json::type_t<json::value_t::null>
  {
    using type = null_t;
  };

  template<>
  struct json::type_t<json::value_t::boolean>
  {
    using type = boolean_t;
  };

  template<>
  struct json::type_t<json::value_t::number_int>
  {
    using type = number_int_t;
  };

  template<>
  struct json::type_t<json::value_t::number_uint>
  {
    using type = number_uint_t;
  };

  template<>
  struct json::type_t<json::value_t::number_float>
  {
    using type = number_float_t;
  };

  template<>
  struct json::type_t<json::value_t::string>
  {
    using type = string_t;
  };

  template<>
  struct json::type_t<json::value_t::array>
  {
    using type = array_t;
  };

  template<>
  struct json::type_t<json::value_t::object>
  {
    using type = object_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<
    std::is_same<typename std::decay<T>::type, json::null_t>::value>::type>
  {
    using type = null_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<
    std::is_same<typename std::decay<T>::type, json::boolean_t>::value>::type>
  {
    using type = boolean_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<
    std::is_integral<T>::value&& std::is_signed<T>::value>::type>
  {
    using type = number_int_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<
    !std::is_same<typename std::decay<T>::type, json::boolean_t>::value&&
    std::is_integral<T>::value &&
    !std::is_signed<T>::value>::type>
  {
    using type = number_uint_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
  {
    using type = number_float_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<
    std::is_same<typename std::decay<T>::type, json::string_t>::value ||
    std::is_same<typename std::decay<T>::type, std::initializer_list<json::string_t::value_type>>::value ||
    std::is_same<typename std::decay<T>::type, char*>::value ||
    std::is_same<typename std::decay<T>::type, char const*>::value>::type>
  {
    using type = string_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<
    std::is_same<typename std::decay<T>::type, json::array_t>::value ||
    std::is_same<typename std::decay<T>::type, std::initializer_list<json::array_t::value_type>>::value>::type>
  {
    using type = array_t;
  };

  template<class T>
  struct json::_compatible_value<T, typename std::enable_if<
    std::is_same<typename std::decay<T>::type, json::object_t>::value ||
    std::is_same<typename std::decay<T>::type, std::initializer_list<json::object_t::value_type>>::value>::type>
  {
    using type = object_t;
  };

  inline constexpr json::json(null_t) noexcept
    : m_type{ value_t::null },
    m_value{}
  {
  }

  inline json::json(boolean_t value) noexcept
    : m_type{ value_t::boolean }
  {
    m_value.boolean = std::move(value);
  }

  inline json::json(number_int_t value) noexcept
    : m_type{ value_t::number_int }
  {
    m_value.number_int = std::move(value);
  }

  inline json::json(number_uint_t value) noexcept
    : m_type{ value_t::number_uint }
  {
    m_value.number_uint = std::move(value);
  }

  inline json::json(number_float_t value) noexcept
    : m_type{ value_t::number_float }
  {
    m_value.number_float = std::move(value);
  }

  inline json::json(string_t value)
    : m_type{ value_t::string }
  {
    m_value.string = new string_t(std::move(value));
  }

  inline json::json(array_t value)
    : m_type{ value_t::array }
  {
    m_value.array = new array_t(std::move(value));
  }

  inline json::json(object_t value)
    : m_type{ value_t::object }
  {
    m_value.object = new object_t(std::move(value));
  }

  template<class T>
  inline json::json(T value)
    : json{ static_cast<typename _compatible_value<T>::type>(std::move(value)) }
  {
  }

  inline json::json(const json& object)
    : m_type{ object.m_type }
  {
    switch (m_type)
    {
    case value_t::boolean:
      m_value.boolean = object.m_value.boolean;
      break;
    case value_t::number_int:
      m_value.number_int = object.m_value.number_int;
      break;
    case value_t::number_uint:
      m_value.number_uint = object.m_value.number_uint;
      break;
    case value_t::number_float:
      m_value.number_float = object.m_value.number_float;
      break;
    case value_t::string:
      m_value.string = new string_t(const_cast<const string_t&>(*object.m_value.string));
      break;
    case value_t::array:
      m_value.array = new array_t(const_cast<const array_t&>(*object.m_value.array));
      break;
    case value_t::object:
      m_value.object = new object_t(const_cast<const object_t&>(*object.m_value.object));
      break;
    default:
      break;
    }
  }

  inline json::json(json&& object) noexcept
    : m_type{ std::move(object.m_type) },
    m_value{ std::move(object.m_value) }
  {
    object.m_type = value_t::null;
  }

  inline json::~json(void) noexcept
  {
    clear();
  }

  inline json& json::operator=(json object)
  {
    swap(object);
    return *this;
  }

  inline bool json::operator==(const json& object) const
  {
    if (m_type != object.m_type)
    {
      return false;
    }
    switch (m_type)
    {
    case value_t::null:
      return true;
    case value_t::boolean:
      return m_value.boolean == object.m_value.boolean;
    case value_t::number_int:
      return m_value.number_int == object.m_value.number_int;
    case value_t::number_uint:
      return m_value.number_uint == object.m_value.number_uint;
    case value_t::number_float:
      return m_value.number_float == object.m_value.number_float;
    case value_t::string:
      return *m_value.string == *object.m_value.string;
    case value_t::array:
      return *m_value.array == *object.m_value.array;
    case value_t::object:
      return *m_value.object == *object.m_value.object;
    default:
      throw std::runtime_error("Unknown json type");
    }
  }

  inline bool json::operator!=(const json& object) const
  {
    return !(*this == object);
  }

  template<>
  inline const typename json::type_t<json::value_t::boolean>::type& json::get<json::value_t::boolean>() const
  {
    if (value_t::boolean != m_type)
    {
      throw std::runtime_error("Json is not of type boolean");
    }
    return m_value.boolean;
  }

  template<>
  inline const typename json::type_t<json::value_t::number_int>::type& json::get<json::value_t::number_int>() const
  {
    if (value_t::number_int != m_type)
    {
      throw std::runtime_error("Json is not of type number_int");
    }
    return m_value.number_int;
  }

  template<>
  inline const typename json::type_t<json::value_t::number_uint>::type& json::get<json::value_t::number_uint>() const
  {
    if (value_t::number_uint != m_type)
    {
      throw std::runtime_error("Json is not of type number_uint");
    }
    return m_value.number_uint;
  }

  template<>
  inline const typename json::type_t<json::value_t::number_float>::type& json::get<json::value_t::number_float>() const
  {
    if (value_t::number_float != m_type)
    {
      throw std::runtime_error("Json is not of type number_float");
    }
    return m_value.number_float;
  }

  template<>
  inline const typename json::type_t<json::value_t::string>::type& json::get<json::value_t::string>() const
  {
    if (value_t::string != m_type)
    {
      throw std::runtime_error("Json is not of type string");
    }
    return *m_value.string;
  }

  template<>
  inline const typename json::type_t<json::value_t::array>::type& json::get<json::value_t::array>() const
  {
    if (value_t::array != m_type)
    {
      throw std::runtime_error("Json is not of type array");
    }
    return *m_value.array;
  }

  template<>
  inline const typename json::type_t<json::value_t::object>::type& json::get<json::value_t::object>() const
  {
    if (value_t::object != m_type)
    {
      throw std::runtime_error("Json is not of type object");
    }
    return *m_value.object;
  }

  template<json::value_t E>
  inline typename json::type_t<E>::type& json::get()
  {
    return const_cast<typename json::type_t<E>::type&>(const_cast<const json*>(this)->get<E>());
  }

  inline const json& json::at(array_t::size_type index) const
  {
    return get<value_t::array>().at(index);
  }

  inline json& json::at(array_t::size_type index)
  {
    return const_cast<json&>(const_cast<const json*>(this)->at(index));
  }

  inline const json& json::at(const object_t::key_type& key) const
  {
    return get<value_t::object>().at(key);
  }

  inline json& json::at(const object_t::key_type& key)
  {
    return const_cast<json&>(const_cast<const json*>(this)->at(key));
  }

  template<class ...T>
  inline const json& json::at(array_t::size_type index, T... args) const
  {
    return at(index).at(args...);
  }

  template<class ...T>
  inline json& json::at(array_t::size_type index, T... args)
  {
    return const_cast<json&>(const_cast<const json*>(this)->at(index, args...));
  }

  template<class ...T>
  inline const json& json::at(const object_t::key_type& key, T... args) const
  {
    return at(key).at(args...);
  }

  template<class ...T>
  inline json& json::at(const object_t::key_type& key, T... args)
  {
    return const_cast<json&>(const_cast<const json*>(this)->at(key, args...));
  }

  inline void json::clear(void)
  {
    switch (m_type)
    {
    case value_t::string:
      delete m_value.string;
      break;
    case value_t::array:
      delete m_value.array;
      break;
    case value_t::object:
      delete m_value.object;
      break;
    default:
      break;
    }
    m_type = value_t::null;
  }

  inline void json::swap(json& object)
  {
    std::swap(m_type, object.m_type);
    std::swap(m_value, object.m_value);
  }

  inline json::value_t json::type(void) const noexcept
  {
    return m_type;
  }
}