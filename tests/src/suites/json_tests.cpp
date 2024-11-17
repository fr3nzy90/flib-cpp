// Copyright © 2022-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/json.hpp>

#include <stdexcept>
#include <utility>

#include <catch2/catch2.hpp>

namespace
{
  template<class T>
  inline void check_value(const flib::json&, T)
  {
    FAIL("Unsupported type");
  }

  template<>
  inline void check_value<flib::json::null_t>(const flib::json& object, flib::json::null_t)
  {
    REQUIRE(flib::json::value_t::null == object.type());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::boolean>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type boolean"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_int>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_int"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_uint>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_uint"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_float>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_float"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::string>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type string"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::array>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type array"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::object>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type object"));
  }

  template<>
  inline void check_value<flib::json::boolean_t>(const flib::json& object, flib::json::boolean_t expected)
  {
    REQUIRE(flib::json::value_t::boolean == object.type());
    REQUIRE(expected == object.get<flib::json::value_t::boolean>());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_int>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_int"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_uint>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_uint"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_float>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_float"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::string>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type string"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::array>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type array"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::object>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type object"));
  }

  template<>
  inline void check_value<flib::json::number_int_t>(const flib::json& object, flib::json::number_int_t expected)
  {
    REQUIRE(flib::json::value_t::number_int == object.type());
    REQUIRE(expected == object.get<flib::json::value_t::number_int>());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::boolean>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type boolean"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_uint>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_uint"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_float>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_float"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::string>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type string"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::array>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type array"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::object>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type object"));
  }

  template<>
  inline void check_value<flib::json::number_uint_t>(const flib::json& object, flib::json::number_uint_t expected)
  {
    REQUIRE(flib::json::value_t::number_uint == object.type());
    REQUIRE(expected == object.get<flib::json::value_t::number_uint>());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::boolean>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type boolean"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_int>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_int"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_float>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_float"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::string>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type string"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::array>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type array"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::object>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type object"));
  }

  template<>
  inline void check_value<flib::json::number_float_t>(const flib::json& object, flib::json::number_float_t expected)
  {
    REQUIRE(flib::json::value_t::number_float == object.type());
    REQUIRE(expected == object.get<flib::json::value_t::number_float>());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::boolean>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type boolean"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_int>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_int"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_uint>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_uint"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::string>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type string"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::array>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type array"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::object>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type object"));
  }

  template<>
  inline void check_value<flib::json::string_t>(const flib::json& object, flib::json::string_t expected)
  {
    REQUIRE(flib::json::value_t::string == object.type());
    REQUIRE(expected == object.get<flib::json::value_t::string>());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::boolean>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type boolean"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_int>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_int"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_uint>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_uint"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_float>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_float"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::array>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type array"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::object>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type object"));
  }

  template<>
  inline void check_value<flib::json::array_t>(const flib::json& object, flib::json::array_t expected)
  {
    REQUIRE(flib::json::value_t::array == object.type());
    REQUIRE(expected == object.get<flib::json::value_t::array>());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::boolean>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type boolean"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_int>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_int"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_uint>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_uint"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_float>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_float"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::string>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type string"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::object>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type object"));
  }

  template<>
  inline void check_value<flib::json::object_t>(const flib::json& object, flib::json::object_t expected)
  {
    REQUIRE(flib::json::value_t::object == object.type());
    REQUIRE(expected == object.get<flib::json::value_t::object>());
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::boolean>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type boolean"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_int>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_int"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_uint>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_uint"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::number_float>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type number_float"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::string>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type string"));
    REQUIRE_THROWS_MATCHES(object.get<flib::json::value_t::array>(), std::runtime_error,
      Catch::Matchers::Message("Json is not of type array"));
  }
}

TEST_CASE("Json tests - Standard construction", "[json]")
{
  SECTION("Default")
  {
    auto object = flib::json();
    REQUIRE(flib::json::value_t::null == object.type());
  }
  SECTION("Boolean")
  {
    auto object = flib::json(static_cast<flib::json::boolean_t>(false));
    REQUIRE(flib::json::value_t::boolean == object.type());
  }
  SECTION("Signed integer")
  {
    auto object = flib::json(static_cast<flib::json::number_int_t>(0));
    REQUIRE(flib::json::value_t::number_int == object.type());
  }
  SECTION("Unsigned integer")
  {
    auto object = flib::json(static_cast<flib::json::number_uint_t>(0));
    REQUIRE(flib::json::value_t::number_uint == object.type());
  }
  SECTION("Float")
  {
    auto object = flib::json(static_cast<flib::json::number_float_t>(0));
    REQUIRE(flib::json::value_t::number_float == object.type());
  }
  SECTION("String")
  {
    auto object = flib::json(flib::json::string_t());
    REQUIRE(flib::json::value_t::string == object.type());
  }
  SECTION("Array")
  {
    auto object = flib::json(flib::json::array_t());
    REQUIRE(flib::json::value_t::array == object.type());
  }
  SECTION("Object")
  {
    auto object = flib::json(flib::json::object_t());
    REQUIRE(flib::json::value_t::object == object.type());
  }
}

TEST_CASE("Json tests - Decayed construction", "[json]")
{
  SECTION("Signed integer")
  {
    auto object = flib::json(0);
    REQUIRE(flib::json::value_t::number_int == object.type());
  }
  SECTION("Unsigned integer")
  {
    auto object = flib::json(0u);
    REQUIRE(flib::json::value_t::number_uint == object.type());
  }
  SECTION("Float")
  {
    auto object = flib::json(0.0f);
    REQUIRE(flib::json::value_t::number_float == object.type());
  }
  SECTION("String")
  {
    auto object = flib::json("");
    REQUIRE(flib::json::value_t::string == object.type());
  }
}

TEST_CASE("Json tests - Copy construction", "[json]")
{
  SECTION("Default")
  {
    auto temp = flib::json();
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::null == object.type());
  }
  SECTION("Boolean")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::boolean_t>(false)));
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::boolean == object.type());
  }
  SECTION("Signed integer")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::number_int_t>(0)));
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::number_int == object.type());
  }
  SECTION("Unsigned integer")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::number_uint_t>(0)));
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::number_uint == object.type());
  }
  SECTION("Float")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::number_float_t>(0)));
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::number_float == object.type());
  }
  SECTION("String")
  {
    auto temp = flib::json(flib::json(flib::json::string_t()));
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::string == object.type());
  }
  SECTION("Array")
  {
    auto temp = flib::json(flib::json(flib::json::array_t()));
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::array == object.type());
  }
  SECTION("Object")
  {
    auto temp = flib::json(flib::json(flib::json::object_t()));
    auto object = flib::json(temp);
    REQUIRE(flib::json::value_t::object == object.type());
  }
}

TEST_CASE("Json tests - Move construction", "[json]")
{
  SECTION("Default")
  {
    auto temp = flib::json();
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::null == object.type());
  }
  SECTION("Boolean")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::boolean_t>(false)));
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::boolean == object.type());
  }
  SECTION("Signed integer")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::number_int_t>(0)));
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::number_int == object.type());
  }
  SECTION("Unsigned integer")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::number_uint_t>(0)));
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::number_uint == object.type());
  }
  SECTION("Float")
  {
    auto temp = flib::json(flib::json(static_cast<flib::json::number_float_t>(0)));
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::number_float == object.type());
  }
  SECTION("String")
  {
    auto temp = flib::json(flib::json(flib::json::string_t()));
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::string == object.type());
  }
  SECTION("Array")
  {
    auto temp = flib::json(flib::json(flib::json::array_t()));
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::array == object.type());
  }
  SECTION("Object")
  {
    auto temp = flib::json(flib::json(flib::json::object_t()));
    auto object = flib::json(std::move(temp));
    REQUIRE(flib::json::value_t::object == object.type());
  }
}

TEST_CASE("Json tests - Value retrieval", "[json]")
{
  SECTION("Boolean")
  {
    auto value = static_cast<flib::json::boolean_t>(true);
    auto object = flib::json(value);
    check_value(object, value);
  }
  SECTION("Signed integer")
  {
    auto value = static_cast<flib::json::number_int_t>(123);
    auto object = flib::json(value);
    check_value(object, value);
  }
  SECTION("Unsigned integer")
  {
    auto value = static_cast<flib::json::number_uint_t>(123);
    auto object = flib::json(value);
    check_value(object, value);
  }
  SECTION("Float")
  {
    auto value = static_cast<flib::json::number_float_t>(123.456);
    auto object = flib::json(value);
    check_value(object, value);
  }
  SECTION("String")
  {
    auto value = flib::json::string_t("test");
    auto object = flib::json(value);
    check_value(object, value);
  }
  SECTION("Array")
  {
    auto value = flib::json::array_t{ 1,2,3 };
    auto object = flib::json(value);
    check_value(object, value);
  }
  SECTION("Object")
  {
    auto value = flib::json::object_t{ {"key1",1},{"key2",1} };
    auto object = flib::json(value);
    check_value(object, value);
  }
}

TEST_CASE("Json tests - Value change", "[json]")
{
  SECTION("Boolean")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    auto value = static_cast<flib::json::boolean_t>(true);
    object = value;
    check_value(object, value);
  }
  SECTION("Signed integer")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    auto value = static_cast<flib::json::number_int_t>(123);
    object = value;
    check_value(object, value);
  }
  SECTION("Unsigned integer")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    auto value = static_cast<flib::json::number_uint_t>(123);
    object = value;
    check_value(object, value);
  }
  SECTION("Float")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    auto value = static_cast<flib::json::number_float_t>(123.456);
    object = value;
    check_value(object, value);
  }
  SECTION("String")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    auto value = flib::json::string_t("test");
    object = value;
    check_value(object, value);
  }
  SECTION("Array")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    auto value = flib::json::array_t{ 1,2,3 };
    object = value;
    check_value(object, value);
  }
  SECTION("Object")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    auto value = flib::json::object_t{ {"key1",1},{"key2",1} };
    object = value;
    check_value(object, value);
  }
}

TEST_CASE("Json tests - Value clearing", "[json]")
{
  SECTION("Null")
  {
    auto object = flib::json();
    check_value(object, nullptr);
    object.clear();
    check_value(object, nullptr);
  }
  SECTION("Boolean")
  {
    auto value = static_cast<flib::json::boolean_t>(true);
    auto object = flib::json(value);
    check_value(object, value);
    object.clear();
    check_value(object, nullptr);
  }
  SECTION("Signed integer")
  {
    auto value = static_cast<flib::json::number_int_t>(123);
    auto object = flib::json(value);
    check_value(object, value);
    object.clear();
    check_value(object, nullptr);
  }
  SECTION("Unsigned integer")
  {
    auto value = static_cast<flib::json::number_uint_t>(123);
    auto object = flib::json(value);
    check_value(object, value);
    object.clear();
    check_value(object, nullptr);
  }
  SECTION("Float")
  {
    auto value = static_cast<flib::json::number_float_t>(123.456);
    auto object = flib::json(value);
    check_value(object, value);
    object.clear();
    check_value(object, nullptr);
  }
  SECTION("String")
  {
    auto value = flib::json::string_t("test");
    auto object = flib::json(value);
    check_value(object, value);
    object.clear();
    check_value(object, nullptr);
  }
  SECTION("Array")
  {
    auto value = flib::json::array_t{ 1,2,3 };
    auto object = flib::json(value);
    check_value(object, value);
    object.clear();
    check_value(object, nullptr);
  }
  SECTION("Object")
  {
    auto value = flib::json::object_t{ {"key1",1},{"key2",1} };
    auto object = flib::json(value);
    check_value(object, value);
    object.clear();
    check_value(object, nullptr);
  }
}

TEST_CASE("Json tests - Value swapping", "[json]")
{
  SECTION("Null")
  {
    auto object = flib::json();
    auto object2 = flib::json();
    check_value(object, nullptr);
    check_value(object2, nullptr);
    object.swap(object2);
    check_value(object, nullptr);
    check_value(object2, nullptr);
  }
  SECTION("Boolean")
  {
    auto value = static_cast<flib::json::boolean_t>(true);
    auto object = flib::json();
    auto object2 = flib::json(value);
    check_value(object, nullptr);
    check_value(object2, value);
    object.swap(object2);
    check_value(object, value);
    check_value(object2, nullptr);
  }
  SECTION("Signed integer")
  {
    auto value = static_cast<flib::json::number_int_t>(123);
    auto object = flib::json();
    auto object2 = flib::json(value);
    check_value(object, nullptr);
    check_value(object2, value);
    object.swap(object2);
    check_value(object, value);
    check_value(object2, nullptr);
  }
  SECTION("Unsigned integer")
  {
    auto value = static_cast<flib::json::number_uint_t>(123);
    auto object = flib::json();
    auto object2 = flib::json(value);
    check_value(object, nullptr);
    check_value(object2, value);
    object.swap(object2);
    check_value(object, value);
    check_value(object2, nullptr);
  }
  SECTION("Float")
  {
    auto value = static_cast<flib::json::number_float_t>(123.456);
    auto object = flib::json();
    auto object2 = flib::json(value);
    check_value(object, nullptr);
    check_value(object2, value);
    object.swap(object2);
    check_value(object, value);
    check_value(object2, nullptr);
  }
  SECTION("String")
  {
    auto value = flib::json::string_t("test");
    auto object = flib::json();
    auto object2 = flib::json(value);
    check_value(object, nullptr);
    check_value(object2, value);
    object.swap(object2);
    check_value(object, value);
    check_value(object2, nullptr);
  }
  SECTION("Array")
  {
    auto value = flib::json::array_t{ 1,2,3 };
    auto object = flib::json();
    auto object2 = flib::json(value);
    check_value(object, nullptr);
    check_value(object2, value);
    object.swap(object2);
    check_value(object, value);
    check_value(object2, nullptr);
  }
  SECTION("Object")
  {
    auto value = flib::json::object_t{ {"key1",1},{"key2",1} };
    auto object = flib::json();
    auto object2 = flib::json(value);
    check_value(object, nullptr);
    check_value(object2, value);
    object.swap(object2);
    check_value(object, value);
    check_value(object2, nullptr);
  }
}

TEST_CASE("Json tests - Navigation", "[json]")
{
  SECTION("Array")
  {
    auto value = flib::json::array_t
    {
      static_cast<flib::json::number_int_t>(1),
      static_cast<flib::json::number_int_t>(2)
    };
    auto object = flib::json(value);
    check_value(object, value);
    check_value(object.at(0), static_cast<flib::json::number_int_t>(1));
    check_value(object.at(1), static_cast<flib::json::number_int_t>(2));
  }
  SECTION("Object")
  {
    auto value = flib::json::object_t
    {
      { "key0", static_cast<flib::json::number_int_t>(1) },
      { "key1", static_cast<flib::json::number_int_t>(2) }
    };
    auto object = flib::json(value);
    check_value(object, value);
    check_value(object.at("key0"), static_cast<flib::json::number_int_t>(1));
    check_value(object.at("key1"), static_cast<flib::json::number_int_t>(2));
  }
  SECTION("Nested")
  {
    auto value = flib::json::array_t
    {
      flib::json::object_t
      {
        { "key0", flib::json::array_t
          {
            flib::json::array_t
            {
              static_cast<flib::json::number_int_t>(1),
              static_cast<flib::json::number_uint_t>(2)
            },
            flib::json::object_t
            {
              { "key1", flib::json::array_t
                {
                  static_cast<flib::json::number_int_t>(3),
                  static_cast<flib::json::number_uint_t>(4)
                }
              },
              { "key2", static_cast<flib::json::number_int_t>(5) }
            }
          }
        },
        { "key3", flib::json::array_t
          {
            flib::json::object_t
            {
              { "key4", static_cast<flib::json::number_int_t>(6) },
              { "key5", static_cast<flib::json::number_uint_t>(7) }
            },
            flib::json::object_t
            {
              { "key6", static_cast<flib::json::number_int_t>(8) },
              { "key7", static_cast<flib::json::number_uint_t>(9) }
            }
          }
        }
      },
      flib::json::array_t
      {
        static_cast<flib::json::number_int_t>(10),
        static_cast<flib::json::number_uint_t>(11)
      }
    };
    auto object = flib::json(value);
    check_value(object, value);
    check_value(object.at(0, "key0", 0, 0), static_cast<flib::json::number_int_t>(1));
    check_value(object.at(0, "key0", 0, 1), static_cast<flib::json::number_uint_t>(2));
    check_value(object.at(0, "key0", 1, "key1", 0), static_cast<flib::json::number_int_t>(3));
    check_value(object.at(0, "key0", 1, "key1", 1), static_cast<flib::json::number_uint_t>(4));
    check_value(object.at(0, "key0", 1, "key2"), static_cast<flib::json::number_int_t>(5));
    check_value(object.at(0, "key3", 0, "key4"), static_cast<flib::json::number_int_t>(6));
    check_value(object.at(0, "key3", 0, "key5"), static_cast<flib::json::number_uint_t>(7));
    check_value(object.at(0, "key3", 1, "key6"), static_cast<flib::json::number_int_t>(8));
    check_value(object.at(0, "key3", 1, "key7"), static_cast<flib::json::number_uint_t>(9));
    check_value(object.at(1, 0), static_cast<flib::json::number_int_t>(10));
    check_value(object.at(1, 1), static_cast<flib::json::number_uint_t>(11));
  }
}

TEST_CASE("Json tests - Equality check", "[json]")
{
  SECTION("Null")
  {
    auto object = flib::json();
    REQUIRE(object == nullptr);
    REQUIRE_FALSE(object != nullptr);
  }
  SECTION("Boolean")
  {
    auto value = true;
    auto object = flib::json(value);
    REQUIRE(object == value);
    REQUIRE_FALSE(object != value);
  }
  SECTION("Signed integer")
  {
    auto value = 123;
    auto object = flib::json(value);
    REQUIRE(object == value);
    REQUIRE_FALSE(object != value);
  }
  SECTION("Unsigned integer")
  {
    auto value = 123u;
    auto object = flib::json(value);
    REQUIRE(object == value);
    REQUIRE_FALSE(object != value);
  }
  SECTION("Float")
  {
    auto value = 123.456;
    auto object = flib::json(value);
    REQUIRE(object == value);
    REQUIRE_FALSE(object != value);
  }
  SECTION("String")
  {
    auto value = "test";
    auto object = flib::json(value);
    REQUIRE(object == value);
    REQUIRE_FALSE(object != value);
  }
  SECTION("Array")
  {
    auto value = flib::json::array_t{ 1,2,3 };
    auto object = flib::json(value);
    REQUIRE(object == value);
    REQUIRE_FALSE(object != value);
  }
  SECTION("Object")
  {
    auto value = flib::json::object_t{ {"key1",1},{"key2",1} };
    auto object = flib::json(value);
    REQUIRE(object == value);
    REQUIRE_FALSE(object != value);
  }
}