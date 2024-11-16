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
#include <chrono>
#include <cstdint>
#include <thread>

#include <catch2/catch2.hpp>

namespace
{
  using milliseconds = std::chrono::duration<uint64_t, std::milli>;

  inline void sleep_for(const ::milliseconds& duration)
  {
    std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + duration);
  }
}

TEST_CASE("Timer tests - Sanity check", "[timer]")
{
  SECTION("Default construction")
  {
    flib::timer timer;
    REQUIRE(!timer.scheduled());
    timer.schedule({}, ::milliseconds(100));
    REQUIRE(!timer.scheduled());
    timer.reschedule();
    REQUIRE(!timer.scheduled());
  }
  SECTION("Move construction")
  {
    auto timer{ flib::timer() };
    REQUIRE(!timer.scheduled());
    timer.schedule({}, ::milliseconds(100));
    REQUIRE(!timer.scheduled());
    timer.reschedule();
    REQUIRE(!timer.scheduled());
  }
  SECTION("Move assignment")
  {
    auto timer = flib::timer();
    REQUIRE(!timer.scheduled());
    timer.schedule({}, ::milliseconds(100));
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
    timer.schedule(event, ::milliseconds(0));
    ::sleep_for(::milliseconds(50));
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
    timer.schedule(event, ::milliseconds(100));
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    ::sleep_for(::milliseconds(100));
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
      ::sleep_for(::milliseconds(100));
    };
    timer.schedule(event, ::milliseconds(0), ::milliseconds(100), flib::timer::type_t::fixed_delay);
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    ::sleep_for(::milliseconds(100));
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
      ::sleep_for(::milliseconds(100));
    };
    timer.schedule(event, ::milliseconds(100), ::milliseconds(100), flib::timer::type_t::fixed_delay);
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    ::sleep_for(::milliseconds(100));
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
      ::sleep_for(::milliseconds(100));
    };
    timer.schedule(event, ::milliseconds(0), ::milliseconds(100), flib::timer::type_t::fixed_rate);
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
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
      ::sleep_for(::milliseconds(50));
    };
    timer.schedule(event, ::milliseconds(100), ::milliseconds(100), flib::timer::type_t::fixed_rate);
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
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
      ::sleep_for(::milliseconds(100));
    };
    timer.schedule(event, ::milliseconds(0), ::milliseconds(10));
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    timer.clear();
    REQUIRE(!timer.scheduled());
    ::sleep_for(::milliseconds(100));
    REQUIRE(1 == reference);
  }
  SECTION("Within periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
    };
    timer.schedule(event, ::milliseconds(0), ::milliseconds(100), flib::timer::type_t::fixed_rate);
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    timer.clear();
    REQUIRE(!timer.scheduled());
    ::sleep_for(::milliseconds(100));
    REQUIRE(2 == reference);
  }
  SECTION("Event driven")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference, &timer]
    {
      ++reference;
      ::sleep_for(::milliseconds(50));
      timer.clear();
    };
    timer.schedule(event, ::milliseconds(0), ::milliseconds(10));
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
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
    timer.schedule(event, ::milliseconds(50));
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    timer.reschedule();
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(2 == reference);
    timer.schedule(event, ::milliseconds(200), ::milliseconds(100));
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(250));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(4 == reference);
    timer.reschedule();
    ::sleep_for(::milliseconds(250));
    REQUIRE(timer.scheduled());
    REQUIRE(5 == reference);
    ::sleep_for(::milliseconds(100));
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
      timer.schedule(event2, ::milliseconds(100), ::milliseconds(100));
    };
    timer.schedule(event1, ::milliseconds(0));
    REQUIRE(timer.scheduled());
    ::sleep_for(::milliseconds(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(timer.scheduled());
    REQUIRE(5 == reference);
  }
}