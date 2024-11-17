// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <flib/worker.hpp>

#include <atomic>
#include <cstdint>
#include <chrono>
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

TEST_CASE("Worker tests - Sanity check", "[worker]")
{
  SECTION("Default construction")
  {
    flib::worker worker;
    REQUIRE(worker.enabled());
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
  }
  SECTION("Move construction")
  {
    auto worker{ flib::worker() };
    REQUIRE(worker.enabled());
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
  }
  SECTION("Move assignment")
  {
    auto worker = flib::worker();
    REQUIRE(worker.enabled());
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
  }
  SECTION("Executorless worker")
  {
    REQUIRE_THROWS_MATCHES(flib::worker(true, 0), std::logic_error, Catch::Matchers::Message("Executorless worker not allowed"));
    REQUIRE_THROWS_MATCHES(flib::worker(false, 0), std::logic_error, Catch::Matchers::Message("Executorless worker not allowed"));
  }
  SECTION("Multi executor worker")
  {
    flib::worker worker(true, 2);
    REQUIRE(worker.enabled());
    REQUIRE(2 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
  }
  SECTION("Disabled single executor worker")
  {
    flib::worker worker(false);
    REQUIRE(!worker.enabled());
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
  }
  SECTION("Disabled multi executor worker")
  {
    flib::worker worker(false, 2);
    REQUIRE(!worker.enabled());
    REQUIRE(2 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
  }
}

TEST_CASE("Worker tests - Invocation", "[worker]")
{
  SECTION("Single executor worker")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task1 = [&reference]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
    };
    auto task2 = [&reference]
    {
      ++reference;
    };
    worker.invoke(task1);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    auto invocation1 = worker.invoke(task2);
    auto invocation2 = worker.invoke(task2);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    ::sleep_for(::milliseconds(100));
    REQUIRE(3 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(invocation1.expired());
    REQUIRE(!worker.owner(invocation1));
    REQUIRE(invocation2.expired());
    REQUIRE(!worker.owner(invocation2));
  }
  SECTION("Multi executor worker")
  {
    flib::worker worker(true, 2);
    std::atomic<uint32_t> reference(0);
    auto task = [&reference]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
    };
    for (auto i = 0; i < 2; ++i)
    {
      worker.invoke(task);
    }
    ::sleep_for(::milliseconds(50));
    REQUIRE(2 == reference);
    REQUIRE(0 == worker.size());
    auto invocation1 = worker.invoke(task);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    auto invocation2 = worker.invoke(task);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    auto invocation3 = worker.invoke(task);
    REQUIRE(!invocation3.expired());
    REQUIRE(worker.owner(invocation3));
    auto invocation4 = worker.invoke(task);
    REQUIRE(!invocation4.expired());
    REQUIRE(worker.owner(invocation4));
    REQUIRE(4 == worker.size());
    ::sleep_for(::milliseconds(300));
    REQUIRE(6 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(invocation1.expired());
    REQUIRE(!worker.owner(invocation1));
    REQUIRE(invocation2.expired());
    REQUIRE(!worker.owner(invocation2));
    REQUIRE(invocation3.expired());
    REQUIRE(!worker.owner(invocation3));
    REQUIRE(invocation4.expired());
    REQUIRE(!worker.owner(invocation4));
  }
  SECTION("Task driven")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task1 = [&reference]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
    };
    auto task3 = [&reference]
    {
      ++reference;
    };
    auto task2 = [&reference, &worker, &task3]
    {
      ++reference;
      worker.invoke(task3);
    };
    worker.invoke(task1);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    for (auto i = 0; i < 3; ++i)
    {
      worker.invoke(task2);
    }
    REQUIRE(3 == worker.size());
    ::sleep_for(::milliseconds(100));
    REQUIRE(7 == reference);
    REQUIRE(0 == worker.size());
  }
}

TEST_CASE("Worker tests - Cancellation", "[worker]")
{
  SECTION("Normal")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task = [&reference]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
    };
    worker.invoke(task);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    auto invocation1 = worker.invoke(task);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    auto invocation2 = worker.invoke(task);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    REQUIRE(2 == worker.size());
    worker.cancel(invocation1);
    REQUIRE(invocation1.expired());
    REQUIRE(!worker.owner(invocation1));
    invocation2.cancel();
    REQUIRE(invocation2.expired());
    REQUIRE(!worker.owner(invocation2));
    REQUIRE(0 == worker.size());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(1 == reference);
  }
  SECTION("Task driven")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    flib::worker_invocation invocation1, invocation2;
    auto task = [&reference, &worker, &invocation1, &invocation2]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
      worker.cancel(invocation1);
      REQUIRE(invocation1.expired());
      REQUIRE(!worker.owner(invocation1));
      invocation2.cancel();
      REQUIRE(invocation2.expired());
      REQUIRE(!worker.owner(invocation2));
    };
    worker.invoke(task);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    invocation1 = worker.invoke(task);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    invocation2 = worker.invoke(task);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    REQUIRE(2 == worker.size());
    ::sleep_for(::milliseconds(100));
    REQUIRE(0 == worker.size());
    REQUIRE(1 == reference);
  }
}

TEST_CASE("Worker tests - Clearing", "[worker]")
{
  SECTION("Normal")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task = [&reference]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
    };
    worker.invoke(task);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    auto invocation1 = worker.invoke(task);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    auto invocation2 = worker.invoke(task);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    REQUIRE(2 == worker.size());
    worker.clear();
    REQUIRE(invocation1.expired());
    REQUIRE(!worker.owner(invocation1));
    REQUIRE(invocation2.expired());
    REQUIRE(!worker.owner(invocation2));
    REQUIRE(0 == worker.size());
    REQUIRE(1 == reference);
    ::sleep_for(::milliseconds(100));
    REQUIRE(1 == reference);
  }
  SECTION("Task driven")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    flib::worker_invocation invocation1, invocation2;
    auto task = [&reference, &worker, &invocation1, &invocation2]
    {
      ++reference;
      ::sleep_for(::milliseconds(100));
      worker.clear();
      REQUIRE(invocation1.expired());
      REQUIRE(!worker.owner(invocation1));
      REQUIRE(invocation2.expired());
      REQUIRE(!worker.owner(invocation2));
    };
    worker.invoke(task);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    invocation1 = worker.invoke(task);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    invocation2 = worker.invoke(task);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    REQUIRE(2 == worker.size());
    ::sleep_for(::milliseconds(100));
    REQUIRE(0 == worker.size());
    REQUIRE(1 == reference);
  }
}

TEST_CASE("Worker tests - Disabling", "[worker]")
{
  SECTION("Normal")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task = [&reference]
    {
      ++reference;
    };
    worker.disable();
    REQUIRE(!worker.enabled());
    worker.invoke(task);
    ::sleep_for(::milliseconds(50));
    REQUIRE(0 == reference);
    REQUIRE(1 == worker.size());
    worker.enable();
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    worker.disable();
    REQUIRE(!worker.enabled());
    worker.invoke(task);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(1 == worker.size());
  }
  SECTION("Task driven")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task1 = [&worker, &reference]
    {
      ++reference;
      worker.enable();
    };
    auto task2 = [&worker, &reference]
    {
      ++reference;
      worker.disable();
    };
    worker.invoke(task1);
    ::sleep_for(::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    worker.invoke(task2);
    ::sleep_for(::milliseconds(50));
    REQUIRE(2 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(!worker.enabled());
  }
}

TEST_CASE("Worker tests - Prioritization", "[worker]")
{
  flib::worker worker;
  std::atomic<uint32_t> reference(0);
  auto task1 = [&reference]
  {
    ++reference;
    ::sleep_for(::milliseconds(100));
  };
  auto task2 = [&reference]
  {
    reference = reference * 2;
    ::sleep_for(::milliseconds(50));
  };
  auto task3 = [&reference]
  {
    reference += 3;
    ::sleep_for(::milliseconds(50));
  };
  worker.invoke(task1);
  ::sleep_for(::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == worker.size());
  worker.invoke(task1);
  worker.invoke(task2, 1);
  worker.invoke(task3, 1);
  REQUIRE(3 == worker.size());
  ::sleep_for(::milliseconds(300));
  REQUIRE(0 == worker.size());
  REQUIRE(6 == reference);
  worker.invoke([&worker, &task1, &task2, &task3]
    {
      worker.invoke(task1, 1);
      worker.invoke(task2, 2);
      worker.invoke(task3, 3);
      ::sleep_for(::milliseconds(100));
    }
  );
  ::sleep_for(::milliseconds(50));
  REQUIRE(3 == worker.size());
  ::sleep_for(::milliseconds(300));
  REQUIRE(0 == worker.size());
  REQUIRE(19 == reference);
}