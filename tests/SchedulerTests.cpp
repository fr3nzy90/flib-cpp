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

#include <atomic>
#include <thread>

#include <catch2/catch.hpp>

#include "flib/Scheduler.hpp"

#include "Tools.hpp"

TEST_CASE("Scheduler tests - Sanity check", "[Scheduler]")
{
  flib::Scheduler scheduler;
  auto event = []() {};
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE_THROWS_MATCHES(scheduler.Schedule({}, flib::Scheduler::Duration(100)), std::invalid_argument,
    Catch::Matchers::Message("Invalid event"));
  REQUIRE_THROWS_MATCHES(scheduler.Reschedule(), std::runtime_error, Catch::Matchers::Message("Invalid event"));
  scheduler.Schedule(event, flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  scheduler.Cancel();
  REQUIRE(!scheduler.IsScheduled());
}

TEST_CASE("Scheduler tests - Immediate execution", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(0));
  ::SleepFor(flib::Scheduler::Duration(50));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Delayed execution", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(250));
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(200));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(0 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Immediate periodic execution with fixed delay", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    ::SleepFor(flib::Scheduler::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(0), flib::Scheduler::Duration(100),
    flib::Scheduler::Type::FixedDelay);
  ::SleepFor(flib::Scheduler::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Delayed periodic execution with fixed delay", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    ::SleepFor(flib::Scheduler::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(250), flib::Scheduler::Duration(100),
    flib::Scheduler::Type::FixedDelay);
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(200));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(0 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Immediate periodic execution with fixed rate", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    ::SleepFor(flib::Scheduler::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(0), flib::Scheduler::Duration(100),
    flib::Scheduler::Type::FixedRate);
  ::SleepFor(flib::Scheduler::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Delayed periodic execution with fixed rate", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    ::SleepFor(flib::Scheduler::Duration(50));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(250), flib::Scheduler::Duration(100),
    flib::Scheduler::Type::FixedRate);
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(200));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(0 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Normal cancellation within immediate execution", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    ::SleepFor(flib::Scheduler::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(0), flib::Scheduler::Duration(10));
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  scheduler.Cancel();
  REQUIRE(!scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Normal cancellation within periodic execution", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    ::SleepFor(flib::Scheduler::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(0), flib::Scheduler::Duration(100),
    flib::Scheduler::Type::FixedRate);
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  scheduler.Cancel();
  REQUIRE(!scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(2 == reference);
}

TEST_CASE("Scheduler tests - Event scheduler cancellation", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference, &scheduler]()
  {
    ++reference;
    scheduler.Cancel();
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(0), flib::Scheduler::Duration(10));
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(50));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Event scheduler re-scheduling", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event2 = [&reference, &scheduler]()
  {
    reference += 2;
  };
  auto event = [&reference, &scheduler, &event2]()
  {
    ++reference;
    scheduler.Cancel();
    scheduler.Schedule(event2, flib::Scheduler::Duration(100), flib::Scheduler::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(0));
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(5 == reference);
}

TEST_CASE("Scheduler tests - Scheduler rescheduling", "[Scheduler]")
{
  flib::Scheduler scheduler;
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, flib::Scheduler::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
  scheduler.Reschedule();
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(2 == reference);
  scheduler.Schedule(event, flib::Scheduler::Duration(200), flib::Scheduler::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  ::SleepFor(flib::Scheduler::Duration(250));
  REQUIRE(3 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(4 == reference);
  scheduler.Reschedule();
  ::SleepFor(flib::Scheduler::Duration(250));
  REQUIRE(5 == reference);
  ::SleepFor(flib::Scheduler::Duration(100));
  REQUIRE(6 == reference);
}