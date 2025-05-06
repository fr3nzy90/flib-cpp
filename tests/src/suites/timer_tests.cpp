// Copyright © 2019-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/timer.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

#include <catch2/catch2.hpp>

using namespace std::chrono_literals;

namespace
{
  inline void sleep_for(const std::chrono::nanoseconds& p_duration)
  {
    std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + p_duration);
  }
}

TEST_CASE("Timer tests - Sanity check", "[timer]")
{
  SECTION("Default construction")
  {
    flib::timer timer;
    REQUIRE(!timer.scheduled());
    timer.schedule({}, 100ms);
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
    timer.schedule(event, 0s);
    ::sleep_for(50ms);
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
    timer.schedule(event, 100ms);
    REQUIRE(timer.scheduled());
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    ::sleep_for(100ms);
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
      ::sleep_for(100ms);
    };
    timer.schedule(event, 0s, 100ms, flib::timer::type::fixed_delay);
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    ::sleep_for(100ms);
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
      ::sleep_for(100ms);
    };
    timer.schedule(event, 100ms, 100ms, flib::timer::type::fixed_delay);
    REQUIRE(timer.scheduled());
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    ::sleep_for(100ms);
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
      ::sleep_for(100ms);
    };
    timer.schedule(event, 0s, 100ms, flib::timer::type::fixed_rate);
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
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
      ::sleep_for(50ms);
    };
    timer.schedule(event, 100ms, 100ms, flib::timer::type::fixed_rate);
    REQUIRE(timer.scheduled());
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
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
      ::sleep_for(100ms);
    };
    timer.schedule(event, 0s, 10ms);
    REQUIRE(timer.scheduled());
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    timer.clear();
    REQUIRE(!timer.scheduled());
    ::sleep_for(100ms);
    REQUIRE(1 == reference);
  }
  SECTION("Within periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]
    {
      ++reference;
      ::sleep_for(100ms);
    };
    timer.schedule(event, 0s, 100ms, flib::timer::type::fixed_rate);
    REQUIRE(timer.scheduled());
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    timer.clear();
    REQUIRE(!timer.scheduled());
    ::sleep_for(100ms);
    REQUIRE(2 == reference);
  }
  SECTION("Event driven")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference, &timer]
    {
      ++reference;
      ::sleep_for(50ms);
      timer.clear();
    };
    timer.schedule(event, 0s, 10ms);
    REQUIRE(timer.scheduled());
    ::sleep_for(100ms);
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
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
    timer.schedule(event, 50ms);
    REQUIRE(timer.scheduled());
    ::sleep_for(100ms);
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    timer.reschedule();
    REQUIRE(timer.scheduled());
    ::sleep_for(100ms);
    REQUIRE(!timer.scheduled());
    REQUIRE(2 == reference);
    timer.schedule(event, 200ms, 100ms);
    REQUIRE(timer.scheduled());
    ::sleep_for(250ms);
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    ::sleep_for(100ms);
    REQUIRE(4 == reference);
    timer.reschedule();
    ::sleep_for(250ms);
    REQUIRE(timer.scheduled());
    REQUIRE(5 == reference);
    ::sleep_for(100ms);
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
      timer.schedule(event2, 100ms, 100ms);
    };
    timer.schedule(event1, 0s);
    REQUIRE(timer.scheduled());
    ::sleep_for(50ms);
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    ::sleep_for(100ms);
    REQUIRE(timer.scheduled());
    REQUIRE(5 == reference);
  }
}