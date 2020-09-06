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

#include "flib/sync_guard.hpp"

#include "testing.hpp"

TEST_CASE("Sync_guard tests - Sanity check", "[sync_guard]")
{
  flib::sync_guard guard;
  REQUIRE(0 == guard.size());
  REQUIRE(guard.lock(0));
  REQUIRE(0 == guard.size());
}

TEST_CASE("Sync_guard tests - Locking", "[sync_guard]")
{
  SECTION("Basic unlock-lock")
  {
    flib::sync_guard guard;
    REQUIRE(0 == guard.size());
    guard.unlock();
    REQUIRE(guard.lock(1, decltype(guard)::duration_t(1000)));
    REQUIRE(0 == guard.size());
  }
  SECTION("Complex unlock-lock")
  {
    flib::sync_guard guard;
    std::atomic<uint32_t> reference(0);
    REQUIRE(0 == guard.size());
    guard.unlock();
    auto task = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == guard.size());
    guard.unlock();
    task.get();
    REQUIRE(0 == guard.size());
    REQUIRE(1 == reference);
  }
  SECTION("Basic lock-unlock")
  {
    flib::sync_guard guard;
    std::atomic<uint32_t> reference(0);
    REQUIRE(0 == guard.size());
    auto task = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == guard.size());
    guard.unlock();
    REQUIRE(1 == guard.size());
    guard.unlock();
    task.get();
    REQUIRE(0 == guard.size());
    REQUIRE(1 == reference);
  }
  SECTION("Complex lock-unlock")
  {
    flib::sync_guard guard;
    std::atomic<uint32_t> reference(0);
    REQUIRE(0 == guard.size());
    auto task1 = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    auto task2 = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    auto task3 = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(3, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(3 == guard.size());
    guard.unlock();
    REQUIRE(3 == guard.size());
    guard.unlock();
    task1.get();
    task2.get();
    REQUIRE(1 == guard.size());
    REQUIRE(2 == reference);
    guard.unlock();
    task3.get();
    REQUIRE(0 == guard.size());
    REQUIRE(3 == reference);
  }
  SECTION("Basic lock-unlock all")
  {
    flib::sync_guard guard;
    std::atomic<uint32_t> reference(0);
    REQUIRE(0 == guard.size());
    auto task = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(1 == guard.size());
    guard.unlock_all();
    task.get();
    REQUIRE(0 == guard.size());
    REQUIRE(1 == reference);
  }
  SECTION("Complex lock-unlock all")
  {
    flib::sync_guard guard;
    std::atomic<uint32_t> reference(0);
    REQUIRE(0 == guard.size());
    auto task1 = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    auto task2 = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    auto task3 = std::async(std::launch::async, [&reference, &guard]()
      {
        if (guard.lock(3, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
        {
          ++reference;
        }
      }
    );
    testing::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(3 == guard.size());
    guard.unlock_all();
    task1.get();
    task2.get();
    task3.get();
    REQUIRE(0 == guard.size());
    REQUIRE(3 == reference);
  }
}

TEST_CASE("Sync_guard tests - Resetting", "[sync_guard]")
{
  flib::sync_guard guard;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == guard.size());
  auto task = std::async(std::launch::async, [&reference, &guard]()
    {
      if (guard.lock(2, testing::remove_reference_t<decltype(guard)>::duration_t(1000)))
      {
        ++reference;
      }
    }
  );
  testing::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == guard.size());
  guard.unlock();
  REQUIRE(1 == guard.size());
  guard.reset();
  guard.unlock();
  REQUIRE(1 == guard.size());
  guard.unlock();
  task.get();
  REQUIRE(0 == guard.size());
  REQUIRE(1 == reference);
}

TEST_CASE("Sync_guard tests - Timeouting", "[sync_guard]")
{
  flib::sync_guard guard;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == guard.size());
  auto task = std::async(std::launch::async, [&reference, &guard]()
    {
      if (!guard.lock(1, testing::remove_reference_t<decltype(guard)>::duration_t(100)))
      {
        ++reference;
      }
    }
  );
  testing::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == guard.size());
  task.get();
  REQUIRE(0 == guard.size());
  REQUIRE(1 == reference);
}