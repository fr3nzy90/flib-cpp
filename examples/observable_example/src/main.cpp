// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <functional>
#include <ios>
#include <iostream>
#include <string>

#include <flib/mld.hpp>
#include <flib/observable.hpp>

namespace
{
#pragma region Helpers
  struct custom_data
  {
    bool m_flag;
    int m_count;
  };

  std::string to_string(::custom_data p_data)
  {
    return std::string("[flag=") + (p_data.m_flag ? "true" : "false") + ",count=" + std::to_string(p_data.m_count) + "]";
  }

  class custom_observer
  {
  public:
    // method for execution on observable publish
    void event(std::string p_name, ::custom_data p_data)
    {
      ++m_sequential;
      std::cout << "Hello \"" << p_name << "\" for " << m_sequential << "-time with " << ::to_string(p_data) << " ... custom_observer\n";
    }

  private:
    int m_sequential{ 0 };
  };
#pragma endregion

#pragma region Examples
  void example_single_threaded_observable_usage(void)
  {
    // create observable with specific parameters
    flib::observable<std::string, ::custom_data> observable;

    // prepare class member observer
    ::custom_observer observer_obj;
    auto observer_1 = std::bind(&::custom_observer::event, &observer_obj, std::placeholders::_1, std::placeholders::_2);

    // prepare lambda observer
    int sequential = 0;
    auto observer_2 = [&sequential](std::string p_name, ::custom_data p_data)
      {
        ++sequential;
        std::cout << "Hello " << p_name << " for " << sequential << "-time with " << ::to_string(p_data) << " ... lambda\n";
      };

    // check if observable has any observers
    std::cout << "Observable is empty: " << std::boolalpha << observable.empty() << '\n';

    // subscribe class observer
    auto subscription_1 = observable.subscribe(observer_1);
    // check if subscription is expired
    std::cout << "Subscription 1 is expired: " << std::boolalpha << subscription_1.expired() << '\n';
    // check if subscription owned by observable
    std::cout << "Subscription 1 is owned by observable: " << std::boolalpha << observable.owner(subscription_1) << '\n';
    // check observable observers count
    std::cout << "Observable is observed " << observable.size() << "-times\n";
    // check if observable has any observers
    std::cout << "Observable is empty: " << std::boolalpha << observable.empty() << '\n';

    // subscribe lambda observer
    auto subscription_2 = observable.subscribe(observer_2);
    // check if subscription is expired
    std::cout << "Subscription 2 is expired: " << std::boolalpha << subscription_2.expired() << '\n';
    // check if subscription owned by observable
    std::cout << "Subscription 2 is owned by observable: " << std::boolalpha << observable.owner(subscription_2) << '\n';
    // check observable observers count
    std::cout << "Observable is observed " << observable.size() << "-times\n";

    // publish two events on observable => expected to execute class observer & lambda observer two times
    observable.publish("dev", { false, 2 });
    observable.publish("dev", { true, 3 });

    // unsubscribe class observer, can be also implemented as observable.unsubscribe(subscription_1);
    subscription_1.unsubscribe();
    // check if subscription is expired
    std::cout << "Subscription 1 is expired: " << std::boolalpha << subscription_1.expired() << '\n';
    // check if subscription owned by observable
    std::cout << "Subscription 1 is owned by observable: " << std::boolalpha << observable.owner(subscription_1) << '\n';
    // check observable observers count
    std::cout << "Observable is observed " << observable.size() << "-times\n";

    // publish one events on observable => expected to execute lambda observer
    observable.publish("admin", { true, 5 });

    // remove all observers from observable
    observable.clear();

    // check observable observers count
    std::cout << "Observable is observed " << observable.size() << "-times\n";
    // check if observable has any observers
    std::cout << "Observable is empty: " << std::boolalpha << observable.empty() << '\n';

    // publish one events on observable
    observable.publish("admin", { true, 7 });
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_single_threaded_observable_usage();

  return 0;
}