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

#include "flib/sync_queue.hpp"

#include "testing.hpp"

TEST_CASE("Sync_queue tests - Sanity check", "[sync_queue]")
{
  SECTION("Default queue")
  {
    flib::sync_queue<int> queue;
    REQUIRE(queue.enabled());
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
    REQUIRE_THROWS_MATCHES(queue.pop_raw(), std::runtime_error, Catch::Message("Queue is empty"));
  }
  SECTION("Disabled queue")
  {
    flib::sync_queue<int> queue(false);
    REQUIRE(!queue.enabled());
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
    REQUIRE_THROWS_MATCHES(queue.pop_raw(), std::runtime_error, Catch::Message("Queue is empty"));
    REQUIRE_THROWS_MATCHES(queue.pop(), std::runtime_error, Catch::Message("Queue is disabled"));
  }
}

TEST_CASE("Sync_queue tests - Element adding and removing", "[sync_queue]")
{
  SECTION("Synchronous")
  {
    flib::sync_queue<int> queue;
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
    queue.push(1);
    REQUIRE(!queue.empty());
    REQUIRE(1 == queue.size());
    queue.push(2);
    REQUIRE(2 == queue.size());
    queue.push(3);
    REQUIRE(3 == queue.size());
    REQUIRE(1 == queue.pop_raw());
    REQUIRE(2 == queue.size());
    REQUIRE(2 == queue.pop_raw());
    REQUIRE(1 == queue.size());
    REQUIRE(3 == queue.pop_raw());
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
  }
  SECTION("Asynchronous")
  {
    flib::sync_queue<int> queue;
    std::atomic<uint32_t> reference(0);
    auto task = std::async(std::launch::async, [&queue, &reference]()
      {
        if (1 == queue.pop())
        {
          ++reference;
        }
        if (2 == queue.pop())
        {
          ++reference;
        }
        if (3 == queue.pop())
        {
          ++reference;
        }
      }
    );
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
    queue.push(1);
    queue.push(2);
    queue.push(3);
    task.get();
    REQUIRE(3 == reference);
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
  }
  SECTION("Clearing")
  {
    flib::sync_queue<int> queue;
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
    queue.push(1);
    queue.push(2);
    REQUIRE(!queue.empty());
    REQUIRE(2 == queue.size());
    queue.clear();
    REQUIRE(queue.empty());
    REQUIRE(0 == queue.size());
    REQUIRE_THROWS_MATCHES(queue.pop_raw(), std::runtime_error, Catch::Message("Queue is empty"));
  }
}

TEST_CASE("Sync_queue tests - Prioritization", "[sync_queue]")
{
  flib::sync_queue<int> queue;
  REQUIRE(queue.empty());
  REQUIRE(0 == queue.size());
  queue.push(1);
  REQUIRE(!queue.empty());
  REQUIRE(1 == queue.size());
  queue.push(2, 1);
  REQUIRE(2 == queue.size());
  queue.push(3, 1);
  REQUIRE(3 == queue.size());
  queue.push(4, 2);
  REQUIRE(4 == queue.size());
  REQUIRE(4 == queue.pop_raw());
  REQUIRE(2 == queue.pop_raw());
  REQUIRE(3 == queue.pop_raw());
  REQUIRE(1 == queue.pop_raw());
  REQUIRE(queue.empty());
  REQUIRE(0 == queue.size());
}

TEST_CASE("Sync_queue tests - Timeouting", "[sync_queue]")
{
  flib::sync_queue<int> queue;
  REQUIRE(queue.empty());
  REQUIRE(0 == queue.size());
  REQUIRE_THROWS_MATCHES(queue.pop(decltype(queue)::duration_t(50)), std::runtime_error,
    Catch::Message("Queue element retrieval has timed out"));
  auto task = std::async(std::launch::async, [&queue]()
    {
      testing::sleep_for(std::chrono::milliseconds(100));
      queue.push(1);
    }
  );
  REQUIRE_THROWS_MATCHES(queue.pop(decltype(queue)::duration_t(50)), std::runtime_error,
    Catch::Message("Queue element retrieval has timed out"));
  REQUIRE(1 == queue.pop());
}

TEST_CASE("Sync_queue tests - Disabling", "[sync_queue]")
{
  flib::sync_queue<int> queue;
  REQUIRE(queue.enabled());
  REQUIRE(queue.empty());
  REQUIRE(0 == queue.size());
  queue.push(1);
  REQUIRE(!queue.empty());
  REQUIRE(1 == queue.size());
  REQUIRE(1 == queue.pop());
  testing::sleep_for(std::chrono::milliseconds(100));
  queue.disable();
  REQUIRE_THROWS_MATCHES(queue.pop(), std::runtime_error, Catch::Message("Queue is disabled"));
  REQUIRE(!queue.enabled());
  queue.push(2);
  REQUIRE(!queue.empty());
  REQUIRE(1 == queue.size());
  queue.enable();
  REQUIRE(queue.enabled());
  REQUIRE(2 == queue.pop());
  testing::sleep_for(std::chrono::milliseconds(100));
  queue.disable();
  REQUIRE_THROWS_MATCHES(queue.pop(), std::runtime_error, Catch::Message("Queue is disabled"));
  REQUIRE(!queue.enabled());
  queue.push(3);
  REQUIRE(!queue.empty());
  REQUIRE(1 == queue.size());
  REQUIRE_THROWS_MATCHES(queue.pop(), std::runtime_error, Catch::Message("Queue is disabled"));
}

TEST_CASE("Sync_queue tests - Complex types", "[sync_queue]")
{
  flib::sync_queue<std::tuple<bool, int>> queue;
  REQUIRE(queue.empty());
  REQUIRE(0 == queue.size());
  queue.push({ true, 1 });
  REQUIRE(!queue.empty());
  REQUIRE(1 == queue.size());
  REQUIRE(decltype(queue)::element_t{ true, 1 } == queue.pop_raw());
  REQUIRE(queue.empty());
  REQUIRE(0 == queue.size());
  REQUIRE_THROWS_MATCHES(queue.pop_raw(), std::runtime_error, Catch::Message("Queue is empty"));
  queue.push({ false, 2 });
  REQUIRE(!queue.empty());
  REQUIRE(1 == queue.size());
  REQUIRE(decltype(queue)::element_t{ false, 2 } == queue.pop());
  REQUIRE(queue.empty());
  REQUIRE(0 == queue.size());
  queue.disable();
  REQUIRE_THROWS_MATCHES(queue.pop(), std::runtime_error, Catch::Message("Queue is disabled"));
}