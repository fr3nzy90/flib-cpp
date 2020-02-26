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
#include <future>
#include <thread>

#include <catch2/catch.hpp>

#include "flib/SyncLock.hpp"

TEST_CASE("SyncLock tests - Sanity check", "[SyncLock]")
{
  flib::SyncLock syncLock;
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(syncLock.Acquire(0));
  REQUIRE(0 == syncLock.LockCount());
}

TEST_CASE("SyncLock tests - Simple release-lock", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  syncLock.Release();
  std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(1, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  ).get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(1 == reference);
}

TEST_CASE("SyncLock tests - Complex release-lock", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  syncLock.Release();
  auto task = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == syncLock.LockCount());
  syncLock.Release();
  task.get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(1 == reference);
}

TEST_CASE("SyncLock tests - Simple lock-release", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  auto task = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == syncLock.LockCount());
  syncLock.Release();
  REQUIRE(1 == syncLock.LockCount());
  syncLock.Release();
  task.get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(1 == reference);
}

TEST_CASE("SyncLock tests - Complex lock-release", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  auto task1 = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  auto task2 = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  auto task3 = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(3, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == syncLock.LockCount());
  syncLock.Release();
  REQUIRE(3 == syncLock.LockCount());
  syncLock.Release();
  task1.get();
  task2.get();
  REQUIRE(1 == syncLock.LockCount());
  REQUIRE(2 == reference);
  syncLock.Release();
  task3.get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(3 == reference);
}

TEST_CASE("SyncLock tests - Simple lock-release all", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  auto task = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == syncLock.LockCount());
  syncLock.ReleaseAll();
  task.get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(1 == reference);
}

TEST_CASE("SyncLock tests - Complex lock-release all", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  auto task1 = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  auto task2 = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  auto task3 = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(3, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(3 == syncLock.LockCount());
  syncLock.ReleaseAll();
  task1.get();
  task2.get();
  task3.get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(3 == reference);
}

TEST_CASE("SyncLock tests - Resetting", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  auto task = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (syncLock.Acquire(2, flib::SyncLock::Duration(1000)))
      {
        ++reference;
      }
    }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == syncLock.LockCount());
  syncLock.Release();
  REQUIRE(1 == syncLock.LockCount());
  syncLock.Reset();
  syncLock.Release();
  REQUIRE(1 == syncLock.LockCount());
  syncLock.Release();
  task.get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(1 == reference);
}

TEST_CASE("SyncLock tests - Lock timeout", "[SyncLock]")
{
  flib::SyncLock syncLock;
  std::atomic<uint32_t> reference(0);
  REQUIRE(0 == syncLock.LockCount());
  auto task = std::async(std::launch::async, [&reference, &syncLock]()
    {
      if (!syncLock.Acquire(1, flib::SyncLock::Duration(100)))
      {
        ++reference;
      }
    }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(1 == syncLock.LockCount());
  task.get();
  REQUIRE(0 == syncLock.LockCount());
  REQUIRE(1 == reference);
}