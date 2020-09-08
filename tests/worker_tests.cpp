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

#include "flib/worker.hpp"

#include "testing.hpp"

TEST_CASE("Worker tests - Sanity check", "[worker]")
{
  SECTION("Default worker")
  {
    flib::worker worker;
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    REQUIRE_THROWS_MATCHES(worker.invoke({}), std::invalid_argument, Catch::Message("Invalid task"));
  }
  SECTION("Executorless worker")
  {
    flib::worker worker(true, 0);
    REQUIRE(0 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    REQUIRE_THROWS_MATCHES(worker.invoke({}), std::invalid_argument, Catch::Message("Invalid task"));
  }
  SECTION("Multi executor worker")
  {
    flib::worker worker(true, 3);
    REQUIRE(3 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    REQUIRE_THROWS_MATCHES(worker.invoke({}), std::invalid_argument, Catch::Message("Invalid task"));
  }
  SECTION("Disabled worker")
  {
    flib::worker worker(false);
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.empty());
    REQUIRE(0 == worker.size());
    REQUIRE(!worker.enabled());
    REQUIRE_THROWS_MATCHES(worker.invoke({}), std::invalid_argument, Catch::Message("Invalid task"));
  }
}

TEST_CASE("Worker tests - Invocation", "[worker]")
{
  SECTION("Executorless worker")
  {
    flib::worker worker(true, 0);
    auto task = [] {};
    auto invocation = worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(!worker.empty());
    REQUIRE(1 == worker.size());
    REQUIRE(!invocation.expired());
    REQUIRE(worker.invoked(invocation));
  }
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
    REQUIRE(worker.invoked(invocation1));
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.invoked(invocation2));
    testing::sleep_for(std::chrono::milliseconds(200));
    REQUIRE(3 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(invocation1.expired());
    REQUIRE(!worker.invoked(invocation1));
    REQUIRE(invocation2.expired());
    REQUIRE(!worker.invoked(invocation2));
  }
  SECTION("Multi executor worker")
  {
    flib::worker worker(true, 3);
    std::atomic<uint32_t> reference(0);
    auto task1 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    auto task2 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(200));
    };
    for (auto i = 0; i < 3; ++i)
    {
      worker.invoke(task1);
    }
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(3 == reference);
    REQUIRE(0 == worker.size());
    auto invocation1 = worker.invoke(task2);
    REQUIRE(!invocation1.expired());
    REQUIRE(worker.invoked(invocation1));
    auto invocation2 = worker.invoke(task2);
    REQUIRE(!invocation2.expired());
    REQUIRE(worker.invoked(invocation2));
    auto invocation3 = worker.invoke(task2);
    REQUIRE(!invocation3.expired());
    REQUIRE(worker.invoked(invocation3));
    auto invocation4 = worker.invoke(task2);
    REQUIRE(!invocation4.expired());
    REQUIRE(worker.invoked(invocation4));
    auto invocation5 = worker.invoke(task2);
    REQUIRE(!invocation5.expired());
    REQUIRE(worker.invoked(invocation5));
    auto invocation6 = worker.invoke(task2);
    REQUIRE(!invocation6.expired());
    REQUIRE(worker.invoked(invocation6));
    REQUIRE(6 == worker.size());
    testing::sleep_for(std::chrono::milliseconds(500));
    REQUIRE(9 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(invocation1.expired());
    REQUIRE(!worker.invoked(invocation1));
    REQUIRE(invocation2.expired());
    REQUIRE(!worker.invoked(invocation2));
    REQUIRE(invocation3.expired());
    REQUIRE(!worker.invoked(invocation3));
    REQUIRE(invocation4.expired());
    REQUIRE(!worker.invoked(invocation4));
    REQUIRE(invocation5.expired());
    REQUIRE(!worker.invoked(invocation5));
    REQUIRE(invocation6.expired());
    REQUIRE(!worker.invoked(invocation6));
  }
  SECTION("Disabled worker")
  {
    flib::worker worker(false);
    auto task = [] {};
    auto invocation = worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(!worker.empty());
    REQUIRE(1 == worker.size());
    REQUIRE(!invocation.expired());
    REQUIRE(worker.invoked(invocation));
  }
  SECTION("Self invocation")
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
    testing::sleep_for(std::chrono::milliseconds(200));
    REQUIRE(7 == reference);
    REQUIRE(0 == worker.size());
  }
}

TEST_CASE("Worker tests - Cancellation", "[worker]")
{
  flib::worker worker(false, 2);
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]
  {
    ++reference;
    testing::sleep_for(std::chrono::milliseconds(100));
  };
  auto invocation1 = worker.invoke(task);
  REQUIRE(!invocation1.expired());
  REQUIRE(worker.invoked(invocation1));
  auto invocation2 = worker.invoke(task);
  REQUIRE(!invocation2.expired());
  REQUIRE(worker.invoked(invocation2));
  REQUIRE(2 == worker.size());
  worker.cancel(invocation1);
  REQUIRE(invocation1.expired());
  REQUIRE(!worker.invoked(invocation1));
  worker.cancel(invocation2);
  REQUIRE(invocation2.expired());
  REQUIRE(!worker.invoked(invocation2));
  REQUIRE(0 == worker.size());
  REQUIRE(0 == reference);
  invocation1 = worker.invoke(task);
  REQUIRE(!invocation1.expired());
  REQUIRE(worker.invoked(invocation1));
  invocation2 = worker.invoke(task);
  REQUIRE(!invocation2.expired());
  REQUIRE(worker.invoked(invocation2));
  auto invocation3 = worker.invoke(task);
  REQUIRE(!invocation3.expired());
  REQUIRE(worker.invoked(invocation3));
  auto invocation4 = worker.invoke(task);
  REQUIRE(!invocation4.expired());
  REQUIRE(worker.invoked(invocation4));
  REQUIRE(4 == worker.size());
  worker.enable();
  testing::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(2 == worker.size());
  REQUIRE(2 == reference);
  REQUIRE(invocation1.expired());
  REQUIRE(!worker.invoked(invocation1));
  REQUIRE(invocation2.expired());
  REQUIRE(!worker.invoked(invocation2));
  worker.cancel(invocation1);
  worker.cancel(invocation4);
  REQUIRE(invocation4.expired());
  REQUIRE(!worker.invoked(invocation4));
  REQUIRE(1 == worker.size());
  testing::sleep_for(std::chrono::milliseconds(100));
  REQUIRE(invocation3.expired());
  REQUIRE(!worker.invoked(invocation3));
  REQUIRE(0 == worker.size());
  REQUIRE(3 == reference);
}

TEST_CASE("Worker tests - Clearing", "[worker]")
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
  for (auto i = 0; i < 3; ++i)
  {
    worker.invoke(task2);
  }
  REQUIRE(3 == worker.size());
  worker.clear();
  REQUIRE(0 == worker.size());
  testing::sleep_for(std::chrono::milliseconds(200));
  REQUIRE(1 == reference);
  REQUIRE(0 == worker.size());
}

TEST_CASE("Worker tests - Disabling", "[worker]")
{
  SECTION("Single executor worker")
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
  SECTION("Multi executor worker")
  {
    flib::worker worker(true, 3);
    std::atomic<uint32_t> reference(0);
    auto task = [&reference]
    {
      ++reference;
    };
    worker.disable();
    REQUIRE(!worker.enabled());
    for (auto i = 0; i < 2; ++i)
    {
      worker.invoke(task);
    }
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(0 == reference);
    REQUIRE(2 == worker.size());
    worker.enable();
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(2 == reference);
    REQUIRE(0 == worker.size());
    REQUIRE(worker.enabled());
    worker.disable();
    REQUIRE(!worker.enabled());
    for (auto i = 0; i < 2; ++i)
    {
      worker.invoke(task);
    }
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(2 == reference);
    REQUIRE(2 == worker.size());
  }
  SECTION("Task driven disabling")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task = [&worker, &reference]
    {
      ++reference;
      REQUIRE_THROWS_MATCHES(worker.disable(), std::runtime_error,
        Catch::Message("Synchronous invocation of stop on internal thread"));
    };
    worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.enabled());
  }
}

TEST_CASE("Worker tests - Prioritization", "[worker]")
{
  flib::worker worker(false, 1);
  std::atomic<uint32_t> reference(1);
  auto task1 = [&reference]
  {
    ++reference;
    testing::sleep_for(std::chrono::milliseconds(50));
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
  auto invocation2 = worker.invoke(task2, 1);
  REQUIRE(!invocation2.expired());
  REQUIRE(worker.invoked(invocation2));
  auto invocation3 = worker.invoke(task3, 1);
  REQUIRE(!invocation3.expired());
  REQUIRE(worker.invoked(invocation3));
  REQUIRE(3 == worker.size());
  worker.enable();
  testing::sleep_for(std::chrono::milliseconds(200));
  REQUIRE(invocation2.expired());
  REQUIRE(!worker.invoked(invocation2));
  REQUIRE(invocation3.expired());
  REQUIRE(!worker.invoked(invocation3));
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
  testing::sleep_for(std::chrono::milliseconds(200));
  REQUIRE(0 == worker.size());
  REQUIRE(19 == reference);
}

TEST_CASE("Worker tests - Reconfiguration", "[worker]")
{
  SECTION("Normal reconfiguration")
  {
    flib::worker worker;
    worker.executors(3);
    std::atomic<uint32_t> reference(0);
    auto task1 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    auto task2 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(200));
    };
    for (auto i = 0; i < 3; ++i)
    {
      worker.invoke(task1);
    }
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(3 == reference);
    REQUIRE(0 == worker.size());
    for (auto i = 0; i < 9; ++i)
    {
      worker.invoke(task2);
    }
    REQUIRE(9 == worker.size());
    testing::sleep_for(std::chrono::milliseconds(700));
    REQUIRE(12 == reference);
    REQUIRE(0 == worker.size());
  }
  SECTION("Task driven reconfiguration")
  {
    flib::worker worker;
    std::atomic<uint32_t> reference(0);
    auto task = [&worker, &reference]
    {
      ++reference;
      REQUIRE_THROWS_MATCHES(worker.executors(3), std::runtime_error,
        Catch::Message("Synchronous invocation of stop on internal thread"));
    };
    worker.invoke(task);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == worker.executors());
    REQUIRE(worker.enabled());
  }
}

TEST_CASE("Worker tests - Diagnostics", "[worker]")
{
  SECTION("Executorless worker")
  {
    flib::worker worker(true, 0);
    REQUIRE(worker.diagnostics().empty());
  }
  SECTION("Single executor worker")
  {
    flib::worker worker(false, 1);
    std::atomic<uint32_t> reference(0);
    decltype(worker)::diagnostics_t diagnostics;
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(1 == diagnostics_list.size());
      diagnostics = diagnostics_list.front();
    }
    REQUIRE(!diagnostics.active);
    REQUIRE(decltype(worker)::clock_t::time_point() == diagnostics.task_start);
    REQUIRE(decltype(worker)::clock_t::time_point() == diagnostics.task_end);
    auto last_diagnostics = diagnostics;
    worker.enable();
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(1 == diagnostics_list.size());
      diagnostics = diagnostics_list.front();
    }
    REQUIRE(diagnostics.active);
    REQUIRE(last_diagnostics.task_start == diagnostics.task_start);
    REQUIRE(last_diagnostics.task_end == diagnostics.task_end);
    last_diagnostics = diagnostics;
    auto task1 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    worker.invoke(task1);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(1 == diagnostics_list.size());
      diagnostics = diagnostics_list.front();
    }
    REQUIRE(diagnostics.active);
    REQUIRE(last_diagnostics.task_start < diagnostics.task_start);
    REQUIRE(last_diagnostics.task_end == diagnostics.task_end);
    last_diagnostics = diagnostics;
    testing::sleep_for(std::chrono::milliseconds(100));
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(1 == diagnostics_list.size());
      diagnostics = diagnostics_list.front();
    }
    REQUIRE(diagnostics.active);
    REQUIRE(last_diagnostics.task_start == diagnostics.task_start);
    REQUIRE(last_diagnostics.task_end < diagnostics.task_end);
    REQUIRE(diagnostics.task_start < diagnostics.task_end);
    REQUIRE(std::chrono::milliseconds(100) <= diagnostics.task_end - diagnostics.task_start);
    last_diagnostics = diagnostics;
    auto task2 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(150));
    };
    worker.invoke(task2);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(2 == reference);
    REQUIRE(0 == worker.size());
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(1 == diagnostics_list.size());
      diagnostics = diagnostics_list.front();
    }
    REQUIRE(diagnostics.active);
    REQUIRE(last_diagnostics.task_start < diagnostics.task_start);
    REQUIRE(last_diagnostics.task_end == diagnostics.task_end);
    last_diagnostics = diagnostics;
    testing::sleep_for(std::chrono::milliseconds(150));
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(1 == diagnostics_list.size());
      diagnostics = diagnostics_list.front();
    }
    REQUIRE(diagnostics.active);
    REQUIRE(last_diagnostics.task_start == diagnostics.task_start);
    REQUIRE(last_diagnostics.task_end < diagnostics.task_end);
    REQUIRE(diagnostics.task_start < diagnostics.task_end);
    REQUIRE(std::chrono::milliseconds(150) <= diagnostics.task_end - diagnostics.task_start);
    last_diagnostics = diagnostics;
    worker.disable();
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(1 == diagnostics_list.size());
      diagnostics = diagnostics_list.front();
    }
    REQUIRE(!diagnostics.active);
    REQUIRE(last_diagnostics.task_start == diagnostics.task_start);
    REQUIRE(last_diagnostics.task_end == diagnostics.task_end);
  }
  SECTION("Multi executor worker")
  {
    flib::worker worker(false, 2);
    std::atomic<uint32_t> reference(0);
    decltype(worker)::diagnostics_t diagnostics1, diagnostics2;
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(2 == diagnostics_list.size());
      diagnostics1 = diagnostics_list.front();
      diagnostics2 = diagnostics_list.back();
    }
    REQUIRE(!diagnostics1.active);
    REQUIRE(decltype(worker)::clock_t::time_point() == diagnostics1.task_start);
    REQUIRE(decltype(worker)::clock_t::time_point() == diagnostics1.task_end);
    REQUIRE(!diagnostics2.active);
    REQUIRE(decltype(worker)::clock_t::time_point() == diagnostics2.task_start);
    REQUIRE(decltype(worker)::clock_t::time_point() == diagnostics2.task_end);
    auto last_diagnostics1 = diagnostics1;
    auto last_diagnostics2 = diagnostics2;
    worker.enable();
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(2 == diagnostics_list.size());
      diagnostics1 = diagnostics_list.front();
      diagnostics2 = diagnostics_list.back();
    }
    REQUIRE(diagnostics1.active);
    REQUIRE(last_diagnostics1.task_start == diagnostics1.task_start);
    REQUIRE(last_diagnostics1.task_end == diagnostics1.task_end);
    REQUIRE(diagnostics2.active);
    REQUIRE(last_diagnostics2.task_start == diagnostics2.task_start);
    REQUIRE(last_diagnostics2.task_end == diagnostics2.task_end);
    last_diagnostics1 = diagnostics1;
    last_diagnostics2 = diagnostics2;
    auto task1 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(150));
    };
    worker.invoke(task1);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == reference);
    REQUIRE(0 == worker.size());
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(2 == diagnostics_list.size());
      diagnostics1 = diagnostics_list.front();
      if (last_diagnostics1.task_start < diagnostics1.task_start)
      {
        diagnostics2 = diagnostics_list.back();
      }
      else
      {
        diagnostics1 = diagnostics_list.back();
        diagnostics2 = diagnostics_list.front();
      }
    }
    REQUIRE(diagnostics1.active);
    REQUIRE(last_diagnostics1.task_start < diagnostics1.task_start);
    REQUIRE(last_diagnostics1.task_end == diagnostics1.task_end);
    REQUIRE(diagnostics2.active);
    REQUIRE(last_diagnostics2.task_start == diagnostics2.task_start);
    REQUIRE(last_diagnostics2.task_end == diagnostics2.task_end);
    last_diagnostics1 = diagnostics1;
    last_diagnostics2 = diagnostics2;
    auto task2 = [&reference]
    {
      ++reference;
      testing::sleep_for(std::chrono::milliseconds(100));
    };
    worker.invoke(task2);
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(2 == reference);
    REQUIRE(0 == worker.size());
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(2 == diagnostics_list.size());
      diagnostics1 = diagnostics_list.front();
      if (last_diagnostics1.task_start == diagnostics1.task_start)
      {
        diagnostics2 = diagnostics_list.back();
      }
      else
      {
        diagnostics1 = diagnostics_list.back();
        diagnostics2 = diagnostics_list.front();
      }
    }
    REQUIRE(diagnostics1.active);
    REQUIRE(last_diagnostics1.task_start == diagnostics1.task_start);
    REQUIRE(last_diagnostics1.task_end == diagnostics1.task_end);
    REQUIRE(diagnostics2.active);
    REQUIRE(last_diagnostics2.task_start < diagnostics2.task_start);
    REQUIRE(last_diagnostics2.task_end == diagnostics2.task_end);
    last_diagnostics1 = diagnostics1;
    last_diagnostics2 = diagnostics2;
    testing::sleep_for(std::chrono::milliseconds(100));
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(2 == diagnostics_list.size());
      diagnostics1 = diagnostics_list.front();
      if (last_diagnostics1.task_start == diagnostics1.task_start)
      {
        diagnostics2 = diagnostics_list.back();
      }
      else
      {
        diagnostics1 = diagnostics_list.back();
        diagnostics2 = diagnostics_list.front();
      }
    }
    REQUIRE(diagnostics1.active);
    REQUIRE(last_diagnostics1.task_start == diagnostics1.task_start);
    REQUIRE(last_diagnostics1.task_end < diagnostics1.task_end);
    REQUIRE(diagnostics1.task_start < diagnostics1.task_end);
    REQUIRE(std::chrono::milliseconds(150) <= diagnostics1.task_end - diagnostics1.task_start);
    REQUIRE(diagnostics2.active);
    REQUIRE(last_diagnostics2.task_start == diagnostics2.task_start);
    REQUIRE(last_diagnostics2.task_end < diagnostics2.task_end);
    REQUIRE(diagnostics2.task_start < diagnostics2.task_end);
    REQUIRE(std::chrono::milliseconds(100) <= diagnostics2.task_end - diagnostics2.task_start);
    last_diagnostics1 = diagnostics1;
    last_diagnostics2 = diagnostics2;
    worker.disable();
    {
      auto diagnostics_list = worker.diagnostics();
      REQUIRE(2 == diagnostics_list.size());
      diagnostics1 = diagnostics_list.front();
      if (last_diagnostics1.task_start == diagnostics1.task_start)
      {
        diagnostics2 = diagnostics_list.back();
      }
      else
      {
        diagnostics1 = diagnostics_list.back();
        diagnostics2 = diagnostics_list.front();
      }
    }
    REQUIRE(!diagnostics1.active);
    REQUIRE(last_diagnostics1.task_start == diagnostics1.task_start);
    REQUIRE(last_diagnostics1.task_end == diagnostics1.task_end);
    REQUIRE(!diagnostics2.active);
    REQUIRE(last_diagnostics2.task_start == diagnostics2.task_start);
    REQUIRE(last_diagnostics2.task_end == diagnostics2.task_end);
  }
}