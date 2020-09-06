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

#include "flib/observable.hpp"

#include "testing.hpp"

TEST_CASE("Observable tests - Sanity check", "[observable]")
{
  flib::observable<> observable;
  REQUIRE(0 == observable.size());
  REQUIRE(observable.empty());
  REQUIRE_THROWS_MATCHES(observable.subscribe({}), std::invalid_argument, Catch::Message("Invalid observer"));
}

TEST_CASE("Observable tests - Subscription cycle", "[observable]")
{
  flib::observable<> observable;
  REQUIRE(0 == observable.size());
  REQUIRE(observable.empty());
  auto observer = []() {};
  auto subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  REQUIRE(1 == observable.size());
  REQUIRE(!observable.empty());
  auto subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(2 == observable.size());
  REQUIRE(!observable.empty());
  observable.unsubscribe(subscription1);
  REQUIRE(subscription1.expired());
  REQUIRE(!subscription2.expired());
  REQUIRE(1 == observable.size());
  REQUIRE(!observable.empty());
  observable.unsubscribe(subscription2);
  REQUIRE(subscription1.expired());
  REQUIRE(subscription2.expired());
  REQUIRE(0 == observable.size());
  REQUIRE(observable.empty());
  subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  REQUIRE(1 == observable.size());
  REQUIRE(!observable.empty());
  subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(2 == observable.size());
  REQUIRE(!observable.empty());
  observable.clear();
  REQUIRE(subscription1.expired());
  REQUIRE(subscription2.expired());
  REQUIRE(0 == observable.size());
  REQUIRE(observable.empty());
}

TEST_CASE("Observable tests - Notification cycle", "[observable]")
{
  flib::observable<> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference]()
  {
    ++reference;
  };
  auto subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  auto subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(2 == observable.size());
  REQUIRE(0 == reference);
  observable.publish();
  observable.publish();
  REQUIRE(4 == reference);
  observable.clear();
  REQUIRE(subscription1.expired());
  REQUIRE(subscription2.expired());
  REQUIRE(0 == observable.size());
  observable.publish();
  REQUIRE(4 == reference);
}

TEST_CASE("Observable tests - Complex types", "[observable]")
{
  flib::observable<bool, int> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference](bool arg1, int arg2)
  {
    if (arg1 && 1 == arg2)
    {
      ++reference;
    }
  };
  auto subscription1 = observable.subscribe(observer);
  REQUIRE(!subscription1.expired());
  auto subscription2 = observable.subscribe(observer);
  REQUIRE(!subscription2.expired());
  REQUIRE(2 == observable.size());
  REQUIRE(0 == reference);
  observable.publish(true, 1);
  REQUIRE(2 == reference);
  observable.publish(false, 1);
  REQUIRE(2 == reference);
  observable.publish(true, 1);
  REQUIRE(4 == reference);
  observable.clear();
  REQUIRE(subscription1.expired());
  REQUIRE(subscription2.expired());
  REQUIRE(0 == observable.size());
  observable.publish(true, 1);
  REQUIRE(4 == reference);
}