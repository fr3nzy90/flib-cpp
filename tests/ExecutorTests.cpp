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

#include "flib/Executor.hpp"

#include "Tools.hpp"

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 6237 6319 26444)
#endif

TEST_CASE("Executor tests - Sanity check", "[Executor]")
{
  flib::Executor executor;
  bool active;
  flib::Executor::Clock::time_point taskStart;
  flib::Executor::Clock::time_point taskEnd;
  auto diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(active);
  REQUIRE(flib::Executor::Clock::time_point() == taskStart);
  REQUIRE(flib::Executor::Clock::time_point() == taskEnd);
  REQUIRE(1 == executor.WorkerCount());
  REQUIRE(executor.IsEmpty());
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(executor.IsEnabled());
  REQUIRE_THROWS_MATCHES(executor.Invoke({}), std::invalid_argument, Catch::Matchers::Message("Invalid task"));
}

TEST_CASE("Executor tests - Workerless executor", "[Executor]")
{
  flib::Executor executor(true, 0);
  auto task = []() {};
  REQUIRE(executor.Diagnostics().empty());
  REQUIRE(0 == executor.WorkerCount());
  REQUIRE(executor.IsEmpty());
  REQUIRE(0 == executor.TaskCount());
  executor.Invoke(task);
  REQUIRE(!executor.IsEmpty());
  REQUIRE(1 == executor.TaskCount());
  executor.Clear();
  REQUIRE(executor.IsEmpty());
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Multi worker executor", "[Executor]")
{
  flib::Executor executor(true, 3);
  bool active;
  flib::Executor::Clock::time_point taskStart;
  flib::Executor::Clock::time_point taskEnd;
  auto diagnosticsList = executor.Diagnostics();
  REQUIRE(3 == diagnosticsList.size());
  for (const auto& diagnostics : diagnosticsList)
  {
    std::tie(active, taskStart, taskEnd) = diagnostics;
    REQUIRE(active);
    REQUIRE(flib::Executor::Clock::time_point() == taskStart);
    REQUIRE(flib::Executor::Clock::time_point() == taskEnd);
  }
  REQUIRE(3 == executor.WorkerCount());
  REQUIRE(executor.IsEmpty());
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(executor.IsEnabled());
  REQUIRE_THROWS_MATCHES(executor.Invoke({}), std::invalid_argument, Catch::Matchers::Message("Invalid task"));
}

TEST_CASE("Executor tests - Disabled executor", "[Executor]")
{
  flib::Executor executor(false, 1);
  auto task = []() {};
  bool active;
  flib::Executor::Clock::time_point taskStart;
  flib::Executor::Clock::time_point taskEnd;
  auto diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(!active);
  REQUIRE(flib::Executor::Clock::time_point() == taskStart);
  REQUIRE(flib::Executor::Clock::time_point() == taskEnd);
  REQUIRE(1 == executor.WorkerCount());
  REQUIRE(executor.IsEmpty());
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(!executor.IsEnabled());
  executor.Invoke(task);
  REQUIRE(!executor.IsEmpty());
  REQUIRE(1 == executor.TaskCount());
  executor.Clear();
  REQUIRE(executor.IsEmpty());
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Single worker invoking", "[Executor]")
{
  flib::Executor executor;
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
  };
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task2);
  }
  ::SleepFor(std::chrono::milliseconds(200));
  REQUIRE(4 == reference);
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Multi worker invoking", "[Executor]")
{
  flib::Executor executor(true, 3);
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(200));
  };
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 9; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(9 == executor.TaskCount());
  ::SleepFor(std::chrono::milliseconds(700));
  REQUIRE(12 == reference);
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Clearing", "[Executor]")
{
  flib::Executor executor;
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
  };
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(3 == executor.TaskCount());
  executor.Clear();
  REQUIRE(0 == executor.TaskCount());
  ::SleepFor(std::chrono::milliseconds(200));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Task cancellation", "[Executor]")
{
  flib::Executor executor(false, 2);
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  auto token1 = executor.Invoke(task);
  auto token2 = executor.Invoke(task);
  REQUIRE(2 == executor.TaskCount());
  executor.Cancel(token1);
  executor.Cancel(token2);
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(0 == reference);
  token1 = executor.Invoke(task);
  token2 = executor.Invoke(task);
  auto token3 = executor.Invoke(task);
  auto token4 = executor.Invoke(task);
  REQUIRE(4 == executor.TaskCount());
  executor.Enable();
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(2 == executor.TaskCount());
  REQUIRE(2 == reference);
  executor.Cancel(token1);
  executor.Cancel(token4);
  REQUIRE(1 == executor.TaskCount());
  ::SleepFor(std::chrono::milliseconds(100));
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(3 == reference);
}

TEST_CASE("Executor tests - Self invoking", "[Executor]")
{
  flib::Executor executor;
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  auto task3 = [&reference]()
  {
    ++reference;
  };
  auto task2 = [&reference, &executor, &task3]()
  {
    ++reference;
    executor.Invoke(task3);
  };
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(3 == executor.TaskCount());
  ::SleepFor(std::chrono::milliseconds(200));
  REQUIRE(7 == reference);
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Simple disabling executor cycle", "[Executor]")
{
  flib::Executor executor(false, 1);
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
  };
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(0 == reference);
  REQUIRE(1 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(1 == executor.TaskCount());
}

TEST_CASE("Executor tests - Complex disabling executor cycle", "[Executor]")
{
  flib::Executor executor(false, 3);
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
  };
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(0 == reference);
  REQUIRE(3 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(3 == executor.TaskCount());
}

TEST_CASE("Executor tests - Simple enabling executor cycle", "[Executor]")
{
  flib::Executor executor;
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
  };
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(1 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(2 == reference);
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Complex enabling executor cycle", "[Executor]")
{
  flib::Executor executor(true, 3);
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
  };
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(3 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(6 == reference);
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Task prioritization", "[Executor]")
{
  flib::Executor executor(false, 1);
  std::atomic<uint32_t> reference(1);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(50));
  };
  auto task2 = [&reference]()
  {
    reference = reference * 2;
    ::SleepFor(std::chrono::milliseconds(50));
  };
  auto task3 = [&reference]()
  {
    reference += 3;
    ::SleepFor(std::chrono::milliseconds(50));
  };
  executor.Invoke(task);
  executor.Invoke(task2, 1);
  executor.Invoke(task3, 1);
  REQUIRE(3 == executor.TaskCount());
  executor.Enable();
  ::SleepFor(std::chrono::milliseconds(200));
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(6 == reference);
  executor.Invoke([&executor, &task, &task2, &task3]()
    {
      executor.Invoke(task, 1);
      executor.Invoke(task2, 2);
      executor.Invoke(task3, 3);
      ::SleepFor(std::chrono::milliseconds(100));
    }
  );
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(3 == executor.TaskCount());
  ::SleepFor(std::chrono::milliseconds(200));
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(19 == reference);
}

TEST_CASE("Executor tests - Reconfiguring workers", "[Executor]")
{
  flib::Executor executor;
  executor.SetWorkerCount(3);
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(200));
  };
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 9; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(9 == executor.TaskCount());
  ::SleepFor(std::chrono::milliseconds(700));
  REQUIRE(12 == reference);
  REQUIRE(0 == executor.TaskCount());
}

TEST_CASE("Executor tests - Single worker diagnostics", "[Scheduler]")
{
  flib::Executor executor(false, 1);
  std::atomic<uint32_t> reference(0);
  bool active;
  flib::Executor::Clock::time_point lastTaskStart, lastTaskEnd, taskStart, taskEnd;
  auto diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(!active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  executor.Enable();
  diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(active);
  REQUIRE(lastTaskStart < taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  lastTaskStart = taskStart;
  ::SleepFor(std::chrono::milliseconds(100));
  diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd < taskEnd);
  lastTaskEnd = taskEnd;
  REQUIRE(taskStart < taskEnd);
  REQUIRE(std::chrono::milliseconds(100) <= taskEnd - taskStart);
  auto task2 = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(150));
  };
  executor.Invoke(task2);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(2 == reference);
  REQUIRE(0 == executor.TaskCount());
  diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(active);
  REQUIRE(lastTaskStart < taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  lastTaskStart = taskStart;
  ::SleepFor(std::chrono::milliseconds(150));
  diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd < taskEnd);
  lastTaskEnd = taskEnd;
  REQUIRE(taskStart < taskEnd);
  REQUIRE(std::chrono::milliseconds(150) <= taskEnd - taskStart);
  executor.Disable();
  diagnosticsList = executor.Diagnostics();
  REQUIRE(1 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  REQUIRE(!active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
}

TEST_CASE("Executor tests - Multi worker diagnostics", "[Scheduler]")
{
  flib::Executor executor(false, 2);
  std::atomic<uint32_t> reference(0);
  bool active, active2;
  flib::Executor::Clock::time_point lastTaskStart, lastTaskEnd, lastTaskStart2, lastTaskEnd2, taskStart, taskEnd,
    taskStart2, taskEnd2;
  auto diagnosticsList = executor.Diagnostics();
  REQUIRE(2 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.back();
  REQUIRE(!active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  REQUIRE(!active2);
  REQUIRE(lastTaskStart2 == taskStart2);
  REQUIRE(lastTaskEnd2 == taskEnd2);
  executor.Enable();
  diagnosticsList = executor.Diagnostics();
  REQUIRE(2 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.back();
  REQUIRE(active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  REQUIRE(active2);
  REQUIRE(lastTaskStart2 == taskStart2);
  REQUIRE(lastTaskEnd2 == taskEnd2);
  auto task = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(150));
  };
  executor.Invoke(task);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  diagnosticsList = executor.Diagnostics();
  REQUIRE(2 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  if (lastTaskStart < taskStart)
  {
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.back();
  }
  else
  {
    std::tie(active, taskStart, taskEnd) = diagnosticsList.back();
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.front();
  }
  REQUIRE(active);
  REQUIRE(lastTaskStart < taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  REQUIRE(active2);
  REQUIRE(lastTaskStart2 == taskStart2);
  REQUIRE(lastTaskEnd2 == taskEnd2);
  lastTaskStart = taskStart;
  auto task2 = [&reference]()
  {
    ++reference;
    ::SleepFor(std::chrono::milliseconds(100));
  };
  executor.Invoke(task2);
  ::SleepFor(std::chrono::milliseconds(50));
  REQUIRE(2 == reference);
  REQUIRE(0 == executor.TaskCount());
  diagnosticsList = executor.Diagnostics();
  REQUIRE(2 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  if (lastTaskStart == taskStart)
  {
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.back();
  }
  else
  {
    std::tie(active, taskStart, taskEnd) = diagnosticsList.back();
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.front();
  }
  REQUIRE(active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  REQUIRE(active2);
  REQUIRE(lastTaskStart2 < taskStart2);
  REQUIRE(lastTaskEnd2 == taskEnd2);
  lastTaskStart2 = taskStart2;
  ::SleepFor(std::chrono::milliseconds(100));
  diagnosticsList = executor.Diagnostics();
  REQUIRE(2 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  if (lastTaskStart == taskStart)
  {
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.back();
  }
  else
  {
    std::tie(active, taskStart, taskEnd) = diagnosticsList.back();
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.front();
  }
  REQUIRE(active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd < taskEnd);
  REQUIRE(taskStart < taskEnd);
  REQUIRE(std::chrono::milliseconds(150) <= taskEnd - taskStart);
  REQUIRE(active2);
  REQUIRE(lastTaskStart2 == taskStart2);
  REQUIRE(lastTaskEnd2 < taskEnd2);
  REQUIRE(taskStart2 < taskEnd2);
  REQUIRE(std::chrono::milliseconds(100) <= taskEnd2 - taskStart2);
  lastTaskEnd = taskEnd;
  lastTaskEnd2 = taskEnd2;
  executor.Disable();
  diagnosticsList = executor.Diagnostics();
  REQUIRE(2 == diagnosticsList.size());
  std::tie(active, taskStart, taskEnd) = diagnosticsList.front();
  if (lastTaskStart == taskStart)
  {
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.back();
  }
  else
  {
    std::tie(active, taskStart, taskEnd) = diagnosticsList.back();
    std::tie(active2, taskStart2, taskEnd2) = diagnosticsList.front();
  }
  REQUIRE(!active);
  REQUIRE(lastTaskStart == taskStart);
  REQUIRE(lastTaskEnd == taskEnd);
  REQUIRE(!active2);
  REQUIRE(lastTaskStart2 == taskStart2);
  REQUIRE(lastTaskEnd2 == taskEnd2);
}

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif