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

#include <future>
#include <memory>
#include <tuple>

#include <flib/atomic.hpp>

#include "testing.hpp"

TEST_CASE("Atomic tests - Sanity check", "[atomic]")
{
  SECTION("Default construction")
  {
    flib::atomic<uint32_t> value;
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    REQUIRE(2 == (value = 2));
    REQUIRE(2 == value);
    REQUIRE(2 == value.load());
    REQUIRE(2 == value.exchange(3));
    REQUIRE(3 == value);
    REQUIRE(3 == value.load());
  }
  SECTION("Move construction")
  {
    auto value{ flib::atomic<uint32_t>() };
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    REQUIRE(2 == (value = 2));
    REQUIRE(2 == value);
    REQUIRE(2 == value.load());
    REQUIRE(2 == value.exchange(3));
    REQUIRE(3 == value);
    REQUIRE(3 == value.load());
  }
  SECTION("Move assignment")
  {
    auto value = flib::atomic<uint32_t>();
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    REQUIRE(2 == (value = 2));
    REQUIRE(2 == value);
    REQUIRE(2 == value.load());
    REQUIRE(2 == value.exchange(3));
    REQUIRE(3 == value);
    REQUIRE(3 == value.load());
  }
  SECTION("Construction with value")
  {
    flib::atomic<uint32_t> value(0);
    REQUIRE(0 == value);
    REQUIRE(0 == value.load());
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    REQUIRE(2 == (value = 2));
    REQUIRE(2 == value);
    REQUIRE(2 == value.load());
    REQUIRE(2 == value.exchange(3));
    REQUIRE(3 == value);
    REQUIRE(3 == value.load());
  }
}

TEST_CASE("Atomic tests - Wait and notify", "[atomic]")
{
  SECTION("Wait and notify_one")
  {
    flib::atomic<uint32_t> value(0);
    auto task = std::async(std::launch::async, [&value]
      {
        value.wait(std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    value.notify_one();
    REQUIRE(std::future_status::ready == task.wait_for(std::chrono::milliseconds(100)));
    task.get();
  }
  SECTION("Wait and notify_all")
  {
    flib::atomic<uint32_t> value(0);
    auto task1 = std::async(std::launch::async, [&value]
      {
        value.wait(std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    auto task2 = std::async(std::launch::async, [&value]
      {
        value.wait(std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    value.notify_all();
    REQUIRE(std::future_status::ready == task1.wait_for(std::chrono::milliseconds(100)));
    REQUIRE(std::future_status::ready == task2.wait_for(std::chrono::milliseconds(100)));
    task1.get();
    task2.get();
  }
  SECTION("Wait_for and notify_one")
  {
    flib::atomic<uint32_t> value(0);
    auto task = std::async(std::launch::async, [&value]
      {
        return value.wait_for(std::chrono::milliseconds(100),
          std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    value.notify_one();
    REQUIRE(task.get());
  }
  SECTION("Wait_for and notify_all")
  {
    flib::atomic<uint32_t> value(0);
    auto task1 = std::async(std::launch::async, [&value]
      {
        return value.wait_for(std::chrono::milliseconds(100),
          std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    auto task2 = std::async(std::launch::async, [&value]
      {
        return value.wait_for(std::chrono::milliseconds(100),
          std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    value.notify_all();
    REQUIRE(task1.get());
    REQUIRE(task2.get());
  }
  SECTION("Wait_for timeouting")
  {
    flib::atomic<uint32_t> value(0);
    REQUIRE(!value.wait_for(std::chrono::milliseconds(50),
      std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0)));
  }
  SECTION("Wait_until and notify_one")
  {
    flib::atomic<uint32_t> value(0);
    auto task = std::async(std::launch::async, [&value]
      {
        return value.wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(100),
          std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    value.notify_one();
    REQUIRE(task.get());
  }
  SECTION("Wait_until and notify_all")
  {
    flib::atomic<uint32_t> value(0);
    auto task1 = std::async(std::launch::async, [&value]
      {
        return value.wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(100),
          std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    auto task2 = std::async(std::launch::async, [&value]
      {
        return value.wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(100),
          std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0));
      });
    value.store(1);
    REQUIRE(1 == value);
    REQUIRE(1 == value.load());
    value.notify_all();
    REQUIRE(task1.get());
    REQUIRE(task2.get());
  }
  SECTION("Wait_until timeouting")
  {
    flib::atomic<uint32_t> value(0);
    REQUIRE(!value.wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(50),
      std::bind(std::not_equal_to<uint32_t>(), std::placeholders::_1, 0)));
  }
}

TEST_CASE("Atomic tests - Complex types", "[atomic]")
{
  using type = std::tuple<bool, std::string>;
  SECTION("Default construction")
  {
    flib::atomic<type> value;
    value.store({ true, "1" });
    REQUIRE(type{ true, "1" } == static_cast<type>(value));
    REQUIRE(type{ true, "1" } == value.load());
    REQUIRE(type{ false, "2" } == (value = { false, "2" }));
    REQUIRE(type{ false, "2" } == static_cast<type>(value));
    REQUIRE(type{ false, "2" } == value.load());
    REQUIRE(type{ false, "2" } == value.exchange({ true, "3" }));
    REQUIRE(type{ true, "3" } == static_cast<type>(value));
    REQUIRE(type{ true, "3" } == value.load());
  }
  SECTION("Construction with value")
  {
    flib::atomic<type> value({ false, "0" });
    REQUIRE(type{ false, "0" } == static_cast<type>(value));
    REQUIRE(type{ false, "0" } == value.load());
    value.store({ true, "1" });
    REQUIRE(type{ true, "1" } == static_cast<type>(value));
    REQUIRE(type{ true, "1" } == value.load());
    REQUIRE(type{ false, "2" } == (value = { false, "2" }));
    REQUIRE(type{ false, "2" } == static_cast<type>(value));
    REQUIRE(type{ false, "2" } == value.load());
    REQUIRE(type{ false, "2" } == value.exchange({ true, "3" }));
    REQUIRE(type{ true, "3" } == static_cast<type>(value));
    REQUIRE(type{ true, "3" } == value.load());
  }
}