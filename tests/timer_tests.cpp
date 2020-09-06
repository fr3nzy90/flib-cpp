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

#include "flib/timer.hpp"

#include "testing.hpp"

TEST_CASE("Timer tests - Sanity check", "[timer]")
{
  flib::timer timer;
  auto event = []() {};
  REQUIRE(!timer.scheduled());
  REQUIRE_THROWS_MATCHES(timer.schedule({}, decltype(timer)::duration_t(100)), std::invalid_argument,
    Catch::Message("Invalid event"));
  REQUIRE_THROWS_MATCHES(timer.reschedule(), std::runtime_error, Catch::Message("Invalid event"));
  timer.schedule(event, decltype(timer)::duration_t(100));
  REQUIRE(timer.scheduled());
  timer.cancel();
  REQUIRE(!timer.scheduled());
}

TEST_CASE("Timer tests - Timing", "[timer]")
{
  SECTION("Immediate non-periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(0));
    testing::sleep_for(decltype(timer)::duration_t(50));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
  }
  SECTION("Delayed non-periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(250));
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(200));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
  }
  SECTION("Immediate periodic execution with fixed delay")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      testing::sleep_for(decltype(timer)::duration_t(100));
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(0), decltype(timer)::duration_t(100),
      decltype(timer)::type_t::fixed_delay);
    testing::sleep_for(decltype(timer)::duration_t(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
  }
  SECTION("Delayed periodic execution with fixed delay")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      testing::sleep_for(decltype(timer)::duration_t(100));
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(250), decltype(timer)::duration_t(100),
      decltype(timer)::type_t::fixed_delay);
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(200));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
  }
  SECTION("Immediate periodic execution with fixed rate")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      testing::sleep_for(decltype(timer)::duration_t(100));
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(0), decltype(timer)::duration_t(100),
      decltype(timer)::type_t::fixed_rate);
    testing::sleep_for(decltype(timer)::duration_t(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
  }
  SECTION("Delayed periodic execution with fixed rate")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      testing::sleep_for(decltype(timer)::duration_t(50));
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(250), decltype(timer)::duration_t(100),
      decltype(timer)::type_t::fixed_rate);
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(200));
    REQUIRE(timer.scheduled());
    REQUIRE(0 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
  }
}

TEST_CASE("Timer tests - Cancellation", "[timer]")
{
  SECTION("Within immediate execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      testing::sleep_for(decltype(timer)::duration_t(100));
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(0), decltype(timer)::duration_t(10));
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    timer.cancel();
    REQUIRE(!timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(1 == reference);
  }
  SECTION("Within periodic execution")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      testing::sleep_for(decltype(timer)::duration_t(100));
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(0), decltype(timer)::duration_t(100),
      decltype(timer)::type_t::fixed_rate);
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(2 == reference);
    timer.cancel();
    REQUIRE(!timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(2 == reference);
  }
  SECTION("Event driven")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference, &timer]()
    {
      ++reference;
      timer.cancel();
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(0), decltype(timer)::duration_t(10));
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(50));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(1 == reference);
  }
}

TEST_CASE("Timer tests - Rescheduling", "[timer]")
{
  SECTION("Normal")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(50));
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(1 == reference);
    timer.reschedule();
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(!timer.scheduled());
    REQUIRE(2 == reference);
    timer.schedule(event, decltype(timer)::duration_t(200), decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(250));
    REQUIRE(3 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(4 == reference);
    timer.reschedule();
    testing::sleep_for(decltype(timer)::duration_t(250));
    REQUIRE(5 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(6 == reference);
  }
  SECTION("Event driven")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event2 = [&reference, &timer]()
    {
      reference += 2;
    };
    auto event = [&reference, &timer, &event2]()
    {
      ++reference;
      timer.schedule(event2, testing::remove_reference_t<decltype(timer)>::duration_t(100),
        testing::remove_reference_t<decltype(timer)>::duration_t(100));
    };
    REQUIRE(!timer.scheduled());
    timer.schedule(event, decltype(timer)::duration_t(0));
    REQUIRE(timer.scheduled());
    testing::sleep_for(decltype(timer)::duration_t(50));
    REQUIRE(timer.scheduled());
    REQUIRE(1 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(3 == reference);
    testing::sleep_for(decltype(timer)::duration_t(100));
    REQUIRE(timer.scheduled());
    REQUIRE(5 == reference);
  }
}

TEST_CASE("Timer tests - Diagnostics", "[timer]")
{
  flib::timer timer;
  std::atomic<uint32_t> reference(0);
  auto diagnostics = timer.diagnostics();
  REQUIRE(diagnostics.active);
  REQUIRE(decltype(timer)::clock_t::time_point() == diagnostics.event_start);
  REQUIRE(decltype(timer)::clock_t::time_point() == diagnostics.event_end);
  auto lastDiagnostics = diagnostics;
  auto event = [&reference]()
  {
    ++reference;
    testing::sleep_for(decltype(timer)::duration_t(100));
  };
  timer.schedule(event, decltype(timer)::duration_t(0));
  REQUIRE(timer.scheduled());
  testing::sleep_for(decltype(timer)::duration_t(50));
  diagnostics = timer.diagnostics();
  REQUIRE(diagnostics.active);
  REQUIRE(lastDiagnostics.event_start < diagnostics.event_start);
  REQUIRE(lastDiagnostics.event_end == diagnostics.event_end);
  lastDiagnostics = diagnostics;
  testing::sleep_for(decltype(timer)::duration_t(100));
  REQUIRE(!timer.scheduled());
  REQUIRE(1 == reference);
  diagnostics = timer.diagnostics();
  REQUIRE(diagnostics.active);
  REQUIRE(lastDiagnostics.event_start == diagnostics.event_start);
  REQUIRE(lastDiagnostics.event_end < diagnostics.event_end);
  REQUIRE(diagnostics.event_start < diagnostics.event_end);
  REQUIRE(decltype(timer)::duration_t(100) <= diagnostics.event_end - diagnostics.event_start);
  lastDiagnostics = diagnostics;
  auto event2 = [&reference]()
  {
    ++reference;
    testing::sleep_for(decltype(timer)::duration_t(150));
  };
  timer.schedule(event2, decltype(timer)::duration_t(0));
  REQUIRE(timer.scheduled());
  testing::sleep_for(decltype(timer)::duration_t(50));
  diagnostics = timer.diagnostics();
  REQUIRE(diagnostics.active);
  REQUIRE(lastDiagnostics.event_start < diagnostics.event_start);
  REQUIRE(lastDiagnostics.event_end == diagnostics.event_end);
  lastDiagnostics = diagnostics;
  testing::sleep_for(decltype(timer)::duration_t(150));
  REQUIRE(!timer.scheduled());
  REQUIRE(2 == reference);
  diagnostics = timer.diagnostics();
  REQUIRE(diagnostics.active);
  REQUIRE(lastDiagnostics.event_start == diagnostics.event_start);
  REQUIRE(lastDiagnostics.event_end < diagnostics.event_end);
  REQUIRE(diagnostics.event_start < diagnostics.event_end);
  REQUIRE(decltype(timer)::duration_t(100) <= diagnostics.event_end - diagnostics.event_start);
}

TEST_CASE("Timer tests - Exceptions", "[timer]")
{
  SECTION("Handlerless exception detection")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      throw std::runtime_error("Some exception");
    };
    timer.schedule(event, decltype(timer)::duration_t(0));
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(!timer.diagnostics().active);
    REQUIRE(!timer.scheduled());
  }
  SECTION("Exception detection with handler")
  {
    flib::timer timer;
    std::atomic<uint32_t> reference(0);
    auto event = [&reference]()
    {
      ++reference;
      throw std::runtime_error("Some exception");
    };
    auto exception_handler = [&reference](std::exception_ptr e)
    {
      auto test = [&reference, e]()
      {
        if (e)
        {
          ++reference;
          std::rethrow_exception(e);
        }
      };
      REQUIRE_THROWS_MATCHES(test(), std::runtime_error, Catch::Message("Some exception"));
    };
    timer.handle_exceptions(exception_handler);
    timer.schedule(event, decltype(timer)::duration_t(0));
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(!timer.diagnostics().active);
    REQUIRE(!timer.scheduled());
  }
}