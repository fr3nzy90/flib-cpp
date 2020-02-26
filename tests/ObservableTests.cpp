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

#include <catch2/catch.hpp>

#include "flib/Observable.hpp"

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:26444)
#endif

TEST_CASE("Observable<void> tests - Sanity check", "[Observable]")
{
  flib::Observable<void> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  REQUIRE_THROWS_MATCHES(observable.Subscribe({}), std::invalid_argument,
    Catch::Matchers::Message("Invalid observer"));
}

TEST_CASE("Observable<bool> tests - Sanity check", "[Observable]")
{
  flib::Observable<bool> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  REQUIRE_THROWS_MATCHES(observable.Subscribe({}), std::invalid_argument,
    Catch::Matchers::Message("Invalid observer"));
}

TEST_CASE("Observable<bool, int> tests - Sanity check", "[Observable]")
{
  flib::Observable<bool, int> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  REQUIRE_THROWS_MATCHES(observable.Subscribe({}), std::invalid_argument,
    Catch::Matchers::Message("Invalid observer"));
}

TEST_CASE("Observable<void> tests - Simple subscription cycle", "[Observable]")
{
  flib::Observable<void> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  auto observer = []() {};
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  observable.Unsubscribe(subscription);
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
}

TEST_CASE("Observable<bool> tests - Simple subscription cycle", "[Observable]")
{
  flib::Observable<bool> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  auto observer = [](bool) {};
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  observable.Unsubscribe(subscription);
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
}

TEST_CASE("Observable<bool, int> tests - Simple subscription cycle", "[Observable]")
{
  flib::Observable<bool, int> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  auto observer = [](bool, int) {};
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  observable.Unsubscribe(subscription);
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
}

TEST_CASE("Observable<void> tests - Complex subscription cycle", "[Observable]")
{
  flib::Observable<void> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  auto observer = []() {};
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  auto subscription1 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription1));
  REQUIRE(2 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  auto subscription2 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription2));
  REQUIRE(3 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  observable.Clear();
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(!observable.IsSubscribed(subscription1));
  REQUIRE(!observable.IsSubscribed(subscription2));
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
}

TEST_CASE("Observable<bool> tests - Complex subscription cycle", "[Observable]")
{
  flib::Observable<bool> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  auto observer = [](bool) {};
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  auto subscription1 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription1));
  REQUIRE(2 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  auto subscription2 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription2));
  REQUIRE(3 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  observable.Clear();
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(!observable.IsSubscribed(subscription1));
  REQUIRE(!observable.IsSubscribed(subscription2));
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
}

TEST_CASE("Observable<bool, int> tests - Complex subscription cycle", "[Observable]")
{
  flib::Observable<bool, int> observable;
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
  auto observer = [](bool, int) {};
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  auto subscription1 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription1));
  REQUIRE(2 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  auto subscription2 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription2));
  REQUIRE(3 == observable.SubscriptionCount());
  REQUIRE(!observable.IsEmpty());
  observable.Clear();
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(!observable.IsSubscribed(subscription1));
  REQUIRE(!observable.IsSubscribed(subscription2));
  REQUIRE(0 == observable.SubscriptionCount());
  REQUIRE(observable.IsEmpty());
}

TEST_CASE("Observable<void> tests - Simple notification cycle", "[Observable]")
{
  flib::Observable<void> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference]()
  {
    ++reference;
  };
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(0 == reference);
  observable.Publish();
  REQUIRE(1 == reference);
  observable.Unsubscribe(subscription);
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(0 == observable.SubscriptionCount());
  observable.Publish();
  REQUIRE(1 == reference);
}

TEST_CASE("Observable<bool> tests - Simple notification cycle", "[Observable]")
{
  flib::Observable<bool> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference](bool value)
  {
    if (value)
    {
      ++reference;
    }
  };
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(0 == reference);
  observable.Publish(true);
  REQUIRE(1 == reference);
  observable.Unsubscribe(subscription);
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(0 == observable.SubscriptionCount());
  observable.Publish(true);
  REQUIRE(1 == reference);
}

TEST_CASE("Observable<bool, int> tests - Simple notification cycle", "[Observable]")
{
  flib::Observable<bool, int> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference](bool value1, int value2)
  {
    if (value1 && 2 == value2)
    {
      ++reference;
    }
  };
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  REQUIRE(1 == observable.SubscriptionCount());
  REQUIRE(0 == reference);
  observable.Publish(true, 2);
  REQUIRE(1 == reference);
  observable.Unsubscribe(subscription);
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(0 == observable.SubscriptionCount());
  observable.Publish(true, 2);
  REQUIRE(1 == reference);
}

TEST_CASE("Observable<void> tests - Complex notification cycle", "[Observable]")
{
  flib::Observable<void> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference]()
  {
    ++reference;
  };
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  auto subscription1 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription1));
  auto subscription2 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription2));
  REQUIRE(3 == observable.SubscriptionCount());
  REQUIRE(0 == reference);
  observable.Publish();
  observable.Publish();
  REQUIRE(6 == reference);
  observable.Clear();
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(!observable.IsSubscribed(subscription1));
  REQUIRE(!observable.IsSubscribed(subscription2));
  REQUIRE(0 == observable.SubscriptionCount());
  observable.Publish();
  REQUIRE(6 == reference);
}

TEST_CASE("Observable<bool> tests - Complex notification cycle", "[Observable]")
{
  flib::Observable<bool> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference](bool value)
  {
    if (value)
    {
      ++reference;
    }
  };
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  auto subscription1 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription1));
  auto subscription2 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription2));
  REQUIRE(3 == observable.SubscriptionCount());
  REQUIRE(0 == reference);
  observable.Publish(true);
  observable.Publish(true);
  REQUIRE(6 == reference);
  observable.Clear();
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(!observable.IsSubscribed(subscription1));
  REQUIRE(!observable.IsSubscribed(subscription2));
  REQUIRE(0 == observable.SubscriptionCount());
  observable.Publish(true);
  REQUIRE(6 == reference);
}

TEST_CASE("Observable<bool, int> tests - Complex notification cycle", "[Observable]")
{
  flib::Observable<bool, int> observable;
  std::atomic<uint32_t> reference(0);
  auto observer = [&reference](bool value1, int value2)
  {
    if (value1 && 2 == value2)
    {
      ++reference;
    }
  };
  auto subscription = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription));
  auto subscription1 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription1));
  auto subscription2 = observable.Subscribe(observer);
  REQUIRE(observable.IsSubscribed(subscription2));
  REQUIRE(3 == observable.SubscriptionCount());
  REQUIRE(0 == reference);
  observable.Publish(true, 2);
  observable.Publish(true, 2);
  REQUIRE(6 == reference);
  observable.Clear();
  REQUIRE(!observable.IsSubscribed(subscription));
  REQUIRE(!observable.IsSubscribed(subscription1));
  REQUIRE(!observable.IsSubscribed(subscription2));
  REQUIRE(0 == observable.SubscriptionCount());
  observable.Publish(true, 2);
  REQUIRE(6 == reference);
}

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif