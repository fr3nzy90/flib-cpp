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

#include <flib/timer.hpp>

#include <atomic>
#include <cstdint>

#include "testing.hpp"

TEST_CASE("Timer tests - Sanity check", "[timer]")
{
  SECTION("Default construction")
  {
    flib::timer timer;
    REQUIRE(!timer.scheduled());
    timer.schedule({}, std::chrono::milliseconds(100));
    REQUIRE(!timer.scheduled());
    timer.reschedule();
    REQUIRE(!timer.scheduled());
  }
  SECTION("Move construction")
  {
    auto timer{ flib::timer() };
    REQUIRE(!timer.scheduled());
    timer.schedule({}, std::chrono::milliseconds(100));
    REQUIRE(!timer.scheduled());
    timer.reschedule();
    REQUIRE(!timer.scheduled());
  }
  SECTION("Move assignment")
  {
    auto timer = flib::timer();
    REQUIRE(!timer.scheduled());
    timer.schedule({}, std::chrono::milliseconds(100));
    REQUIRE(!timer.scheduled());
    timer.reschedule();
    REQUIRE(!timer.scheduled());
  }
}

TEST_CASE("Timer tests - Timing", "[timer]")
{
  SECTION("Immediate non-periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
    };
    timer.schedule(event, std::chrono::milliseconds(0));
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
  }
  SECTION("Delayed non-periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
    };
    timer.schedule(event, std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
  }
  SECTION("Immediate periodic execution with fixed delay")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    timer.schedule(event, std::chrono::milliseconds(0), std::chrono::milliseconds(100),
      flib::timer::type_t::fixed_delay);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
  }
  SECTION("Delayed periodic execution with fixed delay")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    timer.schedule(event, std::chrono::milliseconds(100), std::chrono::milliseconds(100),
      flib::timer::type_t::fixed_delay);
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
  }
  SECTION("Immediate periodic execution with fixed rate")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    timer.schedule(event, std::chrono::milliseconds(0), std::chrono::milliseconds(100),
      flib::timer::type_t::fixed_rate);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
  }
  SECTION("Delayed periodic execution with fixed rate")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(50));
    };
    timer.schedule(event, std::chrono::milliseconds(100), std::chrono::milliseconds(100),
      flib::timer::type_t::fixed_rate);
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
  }
}

TEST_CASE("Timer tests - Cancellation", "[timer]")
{
  SECTION("Within immediate execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    timer.schedule(event, std::chrono::milliseconds(0), std::chrono::milliseconds(10));
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    timer.clear();
    REQUIRE(!timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(1 == reference);
  }
  SECTION("Within periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    timer.schedule(event, std::chrono::milliseconds(0), std::chrono::milliseconds(100),
      flib::timer::type_t::fixed_rate);
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    timer.clear();
    REQUIRE(!timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(2 == reference);
  }
  SECTION("Event driven")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference, &timer]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(50));
      timer.clear();
    };
    timer.schedule(event, std::chrono::milliseconds(0), std::chrono::milliseconds(10));
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(1 == reference);
  }
}

TEST_CASE("Timer tests - Rescheduling", "[timer]")
{
  SECTION("Normal")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
    };
    timer.schedule(event, std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    timer.reschedule();
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(2 == reference);
    timer.schedule(event, std::chrono::milliseconds(200), std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(250));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(4 == reference);
    timer.reschedule();
    testing::sleep_for(std::chrono::milliseconds(250));
    REQUIRE(timer.scheduled());
    REQUIRE(5 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(6 == reference);
  }
  SECTION("Event driven")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event2 = [&reference]
    {
      reference += 2;
    };
    auto event1 = [&reference, &timer, &event2]
    {
      ++reference;
      timer.schedule(event2, std::chrono::milliseconds(100), std::chrono::milliseconds(100));
    };
    timer.schedule(event1, std::chrono::milliseconds(0));
    REQUIRE(timer.scheduled());
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    testing::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(5 == reference);
  }
}