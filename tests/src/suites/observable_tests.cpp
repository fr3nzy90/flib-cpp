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

#include <flib/observable.hpp>

#include <atomic>
#include <cstdint>
#include <string>

#include <catch2/catch2.hpp>

TEST_CASE("Observable tests - Sanity check", "[observable]")
{
  SECTION("Default construction")
  {
    flib::observable<> observable;
    REQUIRE(observable.empty());
    REQUIRE(0 == observable.size());
    REQUIRE(observable.subscribe({}).expired());
    REQUIRE(observable.empty());
    REQUIRE(0 == observable.size());
  }
  SECTION("Move construction")
  {
    auto observable{ flib::observable<>() };
    REQUIRE(observable.empty());
    REQUIRE(0 == observable.size());
    REQUIRE(observable.subscribe({}).expired());
    REQUIRE(observable.empty());
    REQUIRE(0 == observable.size());
  }
  SECTION("Move assignment")
  {
    auto observable = flib::observable<>();
    REQUIRE(observable.empty());
    REQUIRE(0 == observable.size());
    REQUIRE(observable.subscribe({}).expired());
    REQUIRE(observable.empty());
    REQUIRE(0 == observable.size());
  }
}

TEST_CASE("Observable tests - Subscription cycle", "[observable]")
{
  flib::observable<> observable;
  auto observer = [] {};
  auto subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  REQUIRE(observable.owner(subscription1));
  REQUIRE(!observable.empty());
  REQUIRE(1 == observable.size());
  auto subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(observable.owner(subscription2));
  REQUIRE(!observable.empty());
  REQUIRE(2 == observable.size());
  observable.unsubscribe(subscription1);
  REQUIRE(subscription1.expired());
  REQUIRE(!observable.owner(subscription1));
  REQUIRE(!subscription2.expired());
  REQUIRE(observable.owner(subscription2));
  REQUIRE(!observable.empty());
  REQUIRE(1 == observable.size());
  subscription2.unsubscribe();
  REQUIRE(subscription1.expired());
  REQUIRE(!observable.owner(subscription1));
  REQUIRE(subscription2.expired());
  REQUIRE(!observable.owner(subscription2));
  REQUIRE(observable.empty());
  REQUIRE(0 == observable.size());
  subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  REQUIRE(observable.owner(subscription1));
  REQUIRE(1 == observable.size());
  REQUIRE(!observable.empty());
  subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(observable.owner(subscription2));
  REQUIRE(!observable.empty());
  REQUIRE(2 == observable.size());
  observable.clear();
  REQUIRE(subscription1.expired());
  REQUIRE(!observable.owner(subscription1));
  REQUIRE(subscription2.expired());
  REQUIRE(!observable.owner(subscription2));
  REQUIRE(observable.empty());
  REQUIRE(0 == observable.size());
}

TEST_CASE("Observable tests - Notification cycle", "[observable]")
{
  flib::observable<> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference]
  {
    ++reference;
  };
  auto subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  REQUIRE(observable.owner(subscription1));
  auto subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(observable.owner(subscription2));
  REQUIRE(!observable.empty());
  REQUIRE(2 == observable.size());
  REQUIRE(0 == reference);
  observable.publish();
  observable.publish();
  REQUIRE(4 == reference);
  observable.clear();
  REQUIRE(subscription1.expired());
  REQUIRE(!observable.owner(subscription1));
  REQUIRE(subscription2.expired());
  REQUIRE(!observable.owner(subscription2));
  REQUIRE(observable.empty());
  REQUIRE(0 == observable.size());
  observable.publish();
  REQUIRE(4 == reference);
}

TEST_CASE("Observable tests - Complex types", "[observable]")
{
  flib::observable<bool, std::string> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference](bool arg1, const std::string& arg2)
    {
      if (arg1 && "1" == arg2)
      {
        ++reference;
      }
    };
  auto subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  REQUIRE(observable.owner(subscription1));
  auto subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(observable.owner(subscription2));
  REQUIRE(!observable.empty());
  REQUIRE(2 == observable.size());
  REQUIRE(0 == reference);
  observable.publish(true, "1");
  REQUIRE(2 == reference);
  observable.publish(false, "1");
  REQUIRE(2 == reference);
  observable.publish(true, "1");
  REQUIRE(4 == reference);
  observable.clear();
  REQUIRE(subscription1.expired());
  REQUIRE(!observable.owner(subscription1));
  REQUIRE(subscription2.expired());
  REQUIRE(!observable.owner(subscription2));
  REQUIRE(observable.empty());
  REQUIRE(0 == observable.size());
  observable.publish(true, "1");
  REQUIRE(4 == reference);
}