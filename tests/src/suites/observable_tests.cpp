// Copyright © 2019-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

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