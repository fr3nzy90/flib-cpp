/*
* MIT License
*
* Copyright (c) 2020 Luka Arnecic
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
#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <string>

#include <catch2/catch.hpp>

#include "flib/WaitQueue.hpp"

TEST_CASE("WaitQueue tests - Sanity check", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  REQUIRE(queue.IsEnabled());
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  REQUIRE_THROWS_MATCHES(queue.Pop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is empty"));
}

TEST_CASE("WaitQueue tests - Inactive queue", "[WaitQueue]")
{
  flib::WaitQueue<int> queue(false);
  REQUIRE(!queue.IsEnabled());
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  REQUIRE_THROWS_MATCHES(queue.Pop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is empty"));
  REQUIRE_THROWS_MATCHES(queue.WaitedPop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is disabled"));
}

TEST_CASE("WaitQueue tests - Queue clearing", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Push(1);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  queue.Clear();
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  REQUIRE_THROWS_MATCHES(queue.Pop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is empty"));
}

TEST_CASE("WaitQueue tests - Simple synchronous push-pop", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Push(1);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  queue.Push(2);
  REQUIRE(2 == queue.ObjectCount());
  queue.Push(3);
  REQUIRE(3 == queue.ObjectCount());
  REQUIRE(1 == queue.Pop());
  REQUIRE(2 == queue.ObjectCount());
  REQUIRE(2 == queue.Pop());
  REQUIRE(1 == queue.ObjectCount());
  REQUIRE(3 == queue.Pop());
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
}

TEST_CASE("WaitQueue tests - Simple asynchronous push-pop", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  std::atomic<uint32_t> reference(0);
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Push(1);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  queue.Push(2);
  REQUIRE(2 == queue.ObjectCount());
  queue.Push(3);
  REQUIRE(3 == queue.ObjectCount());
  std::async(std::launch::async, [&queue, &reference]()
    {
      if (1 == queue.Pop())
      {
        ++reference;
      }
      if (2 == queue.Pop())
      {
        ++reference;
      }
      if (3 == queue.Pop())
      {
        ++reference;
      }
    }
  ).get();
  REQUIRE(3 == reference);
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
}

TEST_CASE("WaitQueue tests - Complex asynchronous push-pop", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  std::atomic<uint32_t> reference(0);
  auto task = std::async(std::launch::async, [&queue, &reference]()
    {
      if (1 == queue.WaitedPop())
      {
        ++reference;
      }
      if (2 == queue.WaitedPop())
      {
        ++reference;
      }
      if (3 == queue.WaitedPop())
      {
        ++reference;
      }
    }
  );
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Push(1);
  queue.Push(2);
  queue.Push(3);
  task.get();
  REQUIRE(3 == reference);
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
}

TEST_CASE("WaitQueue tests - Queue prioritization", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Push(1);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  queue.Push(2, 1);
  REQUIRE(2 == queue.ObjectCount());
  queue.Push(3, 1);
  REQUIRE(3 == queue.ObjectCount());
  queue.Push(4, 2);
  REQUIRE(4 == queue.ObjectCount());
  REQUIRE(4 == queue.Pop());
  REQUIRE(2 == queue.Pop());
  REQUIRE(3 == queue.Pop());
  REQUIRE(1 == queue.Pop());
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
}

TEST_CASE("WaitQueue tests - Queue timeout", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  REQUIRE_THROWS_MATCHES(queue.WaitedPop(decltype(queue)::Duration(50)), std::runtime_error,
    Catch::Matchers::Message("WaitQueue element retrieval has timed out"));
  auto task = std::async(std::launch::async, [&queue]()
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      queue.Push(1);
    }
  );
  REQUIRE_THROWS_MATCHES(queue.WaitedPop(decltype(queue)::Duration(50)), std::runtime_error,
    Catch::Matchers::Message("WaitQueue element retrieval has timed out"));
  REQUIRE(1 == queue.WaitedPop());
}

TEST_CASE("WaitQueue tests - Queue disabling cycle", "[WaitQueue]")
{
  flib::WaitQueue<int> queue;
  REQUIRE(queue.IsEnabled());
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Push(1);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  REQUIRE(1 == queue.WaitedPop());
  std::async(std::launch::async, [&queue]()
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      queue.Disable();
    }
  ).get();
  REQUIRE_THROWS_MATCHES(queue.WaitedPop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is disabled"));
  REQUIRE(!queue.IsEnabled());
  queue.Push(2);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  queue.Enable();
  REQUIRE(queue.IsEnabled());
  REQUIRE(2 == queue.WaitedPop());
  std::async(std::launch::async, [&queue]()
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      queue.Disable();
    }
  ).get();
  REQUIRE_THROWS_MATCHES(queue.WaitedPop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is disabled"));
  REQUIRE(!queue.IsEnabled());
  queue.Push(3);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  REQUIRE_THROWS_MATCHES(queue.WaitedPop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is disabled"));
}

TEST_CASE("WaitQueue tests - Complex elements", "[WaitQueue]")
{
  flib::WaitQueue<std::tuple<bool, int, std::string>> queue;
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Push({ true, 1, "test1" });
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  REQUIRE(decltype(queue)::Element{ true, 1, "test1" } == queue.Pop());
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  REQUIRE_THROWS_MATCHES(queue.Pop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is empty"));
  queue.Push({ false, 2, "test2" });
  REQUIRE(!queue.IsEmpty());
  REQUIRE(1 == queue.ObjectCount());
  REQUIRE(decltype(queue)::Element{ false, 2, "test2" } == queue.WaitedPop());
  REQUIRE(queue.IsEmpty());
  REQUIRE(0 == queue.ObjectCount());
  queue.Disable();
  REQUIRE_THROWS_MATCHES(queue.WaitedPop(), std::runtime_error, Catch::Matchers::Message("WaitQueue is disabled"));
}