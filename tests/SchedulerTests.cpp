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

TEST_CASE("Scheduler tests - Sanity check", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  auto event = []() {};
  REQUIRE(!scheduler.IsScheduled());
  try
  {
    scheduler.Schedule({}, SchedulerType::Duration(100));
    FAIL("Exception not thrown");
  }
  catch (const std::invalid_argument&)
  {
  }
  try
  {
    scheduler.Reschedule();
    FAIL("Exception not thrown");
  }
  catch (const std::runtime_error&)
  {
  }
  scheduler.Schedule(event, SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  scheduler.Cancel();
  REQUIRE(!scheduler.IsScheduled());
}

TEST_CASE("Scheduler tests - Immediate execution", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(0));
  std::this_thread::sleep_for(SchedulerType::Duration(50));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Delayed execution", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(250));
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(200));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(0 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Immediate periodic execution with fixed delay", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(SchedulerType::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(0), SchedulerType::Duration(100), SchedulerType::Type::FixedDelay);
  std::this_thread::sleep_for(SchedulerType::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Delayed periodic execution with fixed delay", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(SchedulerType::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(250), SchedulerType::Duration(100),
    SchedulerType::Type::FixedDelay);
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(200));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(0 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Immediate periodic execution with fixed rate", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(SchedulerType::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(0), SchedulerType::Duration(100), SchedulerType::Type::FixedRate);
  std::this_thread::sleep_for(SchedulerType::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Delayed periodic execution with fixed rate", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(SchedulerType::Duration(50));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(250), SchedulerType::Duration(100),
    SchedulerType::Type::FixedRate);
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(200));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(0 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
}

TEST_CASE("Scheduler tests - Normal cancellation within immediate execution", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(SchedulerType::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(0), SchedulerType::Duration(10));
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  scheduler.Cancel();
  REQUIRE(!scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Normal cancellation within periodic execution", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(SchedulerType::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(0), SchedulerType::Duration(100),
    SchedulerType::Type::FixedRate);
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(2 == reference);
  scheduler.Cancel();
  REQUIRE(!scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(2 == reference);
}

TEST_CASE("Scheduler tests - Event scheduler cancellation", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference, &scheduler]()
  {
    ++reference;
    scheduler.Cancel();
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(0), SchedulerType::Duration(10));
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(50));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(1 == reference);
}

TEST_CASE("Scheduler tests - Event scheduler re-scheduling", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event2 = [&reference, &scheduler]()
  {
    reference += 2;
  };
  auto event = [&reference, &scheduler, &event2]()
  {
    ++reference;
    scheduler.Cancel();
    scheduler.Schedule(event2, SchedulerType::Duration(100), SchedulerType::Duration(100));
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(0));
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(1 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(3 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  REQUIRE(5 == reference);
}

TEST_CASE("Scheduler tests - Scheduler rescheduling", "[Scheduler]")
{
  flib::Scheduler<> scheduler;
  using SchedulerType = decltype(scheduler);
  std::atomic<uint32_t> reference(0);
  auto event = [&reference]()
  {
    ++reference;
  };
  REQUIRE(!scheduler.IsScheduled());
  scheduler.Schedule(event, SchedulerType::Duration(50));
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(1 == reference);
  scheduler.Reschedule();
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(!scheduler.IsScheduled());
  REQUIRE(2 == reference);
  scheduler.Schedule(event, SchedulerType::Duration(200), SchedulerType::Duration(100));
  REQUIRE(scheduler.IsScheduled());
  std::this_thread::sleep_for(SchedulerType::Duration(250));
  REQUIRE(3 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(4 == reference);
  scheduler.Reschedule();
  std::this_thread::sleep_for(SchedulerType::Duration(250));
  REQUIRE(5 == reference);
  std::this_thread::sleep_for(SchedulerType::Duration(100));
  REQUIRE(6 == reference);
}