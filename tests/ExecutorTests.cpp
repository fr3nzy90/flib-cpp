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

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:26444)
#endif

TEST_CASE("Executor tests - Sanity check", "[Executor]")
{
  flib::Executor executor;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
  };
  executor.Invoke(task);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task2);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  };
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 9; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(9 == executor.TaskCount());
  std::this_thread::sleep_for(std::chrono::milliseconds(700));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
  };
  executor.Invoke(task);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(3 == executor.TaskCount());
  executor.Clear();
  REQUIRE(0 == executor.TaskCount());
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(2 == executor.TaskCount());
  REQUIRE(2 == reference);
  executor.Cancel(token1);
  executor.Cancel(token4);
  REQUIRE(1 == executor.TaskCount());
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(3 == executor.TaskCount());
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(0 == reference);
  REQUIRE(1 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  executor.Invoke(task);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(0 == reference);
  REQUIRE(3 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  executor.Invoke(task);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == reference);
  REQUIRE(1 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  executor.Disable();
  REQUIRE(!executor.IsEnabled());
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(3 == executor.TaskCount());
  executor.Enable();
  REQUIRE(executor.IsEnabled());
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  };
  auto task2 = [&reference]()
  {
    reference = reference * 2;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  };
  auto task3 = [&reference]()
  {
    reference += 3;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  };
  executor.Invoke(task);
  executor.Invoke(task2, 1);
  executor.Invoke(task3, 1);
  REQUIRE(3 == executor.TaskCount());
  executor.Enable();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(6 == reference);
  executor.Invoke([&executor, &task, &task2, &task3]()
    {
      executor.Invoke(task, 1);
      executor.Invoke(task2, 2);
      executor.Invoke(task3, 3);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == executor.TaskCount());
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  REQUIRE(0 == executor.TaskCount());
  REQUIRE(19 == reference);
}

TEST_CASE("Executor tests - Reconfiguring workers", "[Executor]")
{
  flib::Executor executor;
  REQUIRE_THROWS_MATCHES(executor.SetWorkerCount(3), std::runtime_error,
    Catch::Matchers::Message("Executor not idle"));
  executor.Disable();
  executor.SetWorkerCount(3);
  executor.Enable();
  std::atomic<uint32_t> reference(0);
  auto task = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  };
  auto task2 = [&reference]()
  {
    ++reference;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  };
  for (auto i = 0; i < 3; ++i)
  {
    executor.Invoke(task);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == reference);
  REQUIRE(0 == executor.TaskCount());
  for (auto i = 0; i < 9; ++i)
  {
    executor.Invoke(task2);
  }
  REQUIRE(9 == executor.TaskCount());
  std::this_thread::sleep_for(std::chrono::milliseconds(700));
  REQUIRE(12 == reference);
  REQUIRE(0 == executor.TaskCount());
}

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif