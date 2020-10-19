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
#include <cstdint>

#include <flib/worker.hpp>

#include "testing.hpp"

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
    REQUIRE_THROWS_MATCHES(flib::worker(true, 0), std::logic_error,
      Catch::Message("Executorless worker not allowed"));
    REQUIRE_THROWS_MATCHES(flib::worker(false, 0), std::logic_error,
      Catch::Message("Executorless worker not allowed"));
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
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    auto task2 = [&reference]
    {
      ++reference;
    };
    worker.invoke(task1);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    auto invocation1 = worker.invoke(task2);
    auto invocation2 = worker.invoke(task2);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    testing::sleep_for(std::chrono::milliseconds(100));
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
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    for (auto i = 0; i < 2; ++i)
    {
      worker.invoke(task);
    }
    testing::sleep_for(std::chrono::milliseconds(50));
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
    testing::sleep_for(std::chrono::milliseconds(300));
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
      testing::sleep_for(std::chrono::milliseconds(100));
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
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    for (auto i = 0; i < 3; ++i)
    {
      worker.invoke(task2);
    }
    REQUIRE(3 == worker.size());
    testing::sleep_for(std::chrono::milliseconds(100));
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
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
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
    testing::sleep_for(std::chrono::milliseconds(100));
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
      testing::sleep_for(std::chrono::milliseconds(100));
      worker.cancel(invocation1);
      REQUIRE(invocation1.expired());
      REQUIRE(!worker.owner(invocation1));
      invocation2.cancel();
      REQUIRE(invocation2.expired());
      REQUIRE(!worker.owner(invocation2));
    };
    worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    invocation1 = worker.invoke(task);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    invocation2 = worker.invoke(task);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    REQUIRE(2 == worker.size());
    testing::sleep_for(std::chrono::milliseconds(100));
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
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
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
    testing::sleep_for(std::chrono::milliseconds(100));
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
      testing::sleep_for(std::chrono::milliseconds(100));
      worker.clear();
      REQUIRE(invocation1.expired());
      REQUIRE(!worker.owner(invocation1));
      REQUIRE(invocation2.expired());
      REQUIRE(!worker.owner(invocation2));
    };
    worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    invocation1 = worker.invoke(task);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.owner(invocation1));
    invocation2 = worker.invoke(task);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.owner(invocation2));
    REQUIRE(2 == worker.size());
    testing::sleep_for(std::chrono::milliseconds(100));
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
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(0 == reference);
    REQUIRE(1 == worker.size());
    worker.enable();
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    worker.disable();
    REQUIRE(!worker.enabled());
    worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
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
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    worker.invoke(task2);
    testing::sleep_for(std::chrono::milliseconds(50));
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
    testing::sleep_for(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]
  {
    reference = reference * 2;
    testing::sleep_for(std::chrono::milliseconds(50));
  };
  auto task3 = [&reference]
  {
    reference += 3;
    testing::sleep_for(std::chrono::milliseconds(50));
  };
  worker.invoke(task1);
  testing::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == worker.size());
  worker.invoke(task1);
  worker.invoke(task2, 1);
  worker.invoke(task3, 1);
  REQUIRE(3 == worker.size());
  testing::sleep_for(std::chrono::milliseconds(300));
  REQUIRE(0 == worker.size());
  REQUIRE(6 == reference);
  worker.invoke([&worker, &task1, &task2, &task3]
    {
      worker.invoke(task1, 1);
      worker.invoke(task2, 2);
      worker.invoke(task3, 3);
      testing::sleep_for(std::chrono::milliseconds(100));
    }
  );
  testing::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == worker.size());
  testing::sleep_for(std::chrono::milliseconds(300));
  REQUIRE(0 == worker.size());
  REQUIRE(19 == reference);
}