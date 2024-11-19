// Copyright © 2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <chrono>
#include <future>
#include <ios>
#include <iostream>
#include <string>
#include <tuple>
#include <thread>

#include <flib/atomic.hpp>
#include <flib/mld.hpp>

namespace
{
#pragma region Helpers
  using custom_type = std::tuple<std::string, int, bool>;

  enum class test_state
  {
    created,
    initialized,
    processed,
    completed
  };

  std::string to_string(::test_state p_value)
  {
    switch (p_value)
    {
    case ::test_state::created:
      return "created";
    case ::test_state::initialized:
      return "initialized";
    case ::test_state::processed:
      return "processed";
    case ::test_state::completed:
      return "completed";
    default:
      return "unknown";
    }
  }

  std::string to_string(::custom_type p_value)
  {
    std::string name;
    int count;
    bool flag;
    std::tie(name, count, flag) = p_value;
    return "(" + name + ", " + std::to_string(count) + ", " + (flag ? "true" : "false") + ")";
  }

  template<class T>
  void set_and_notify(flib::atomic<T>& p_obj, T p_value, std::chrono::milliseconds p_delay)
  {
    // wait for specified duration
    std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + p_delay);

    // set atomic to new value
    p_obj = p_value;

    // notify all waiters
    p_obj.notify_all();
  }
#pragma endregion

#pragma region Examples
  void example_basic_atomic_usage(void)
  {
    // create atomic with initial value
    flib::atomic<::custom_type> value{ {"test",1,false} };

    // check if atomic is lock free (flib::atomic always uses locks)
    std::cout << "flib::atomic<::custom_type> " << (value.is_lock_free() ? "is lock free" : "uses locks") << '\n';

    // print current atomic value, below casting retrieval is same as value.load()
    std::cout << "         Initial value: " << ::to_string(value) << '\n';

    auto returned = value = { "test",2,false };

    // print return value by assignment
    std::cout << "Returned changed value: " << ::to_string(returned) << '\n';
    // print current atomic value and return value by assignment
    std::cout << "    After change value: " << ::to_string(value) << '\n';

    auto returned2 = value.exchange({ "final",3,true });

    // print exchanged value
    std::cout << "       Exchanged value: " << ::to_string(returned2) << '\n';
    // print current atomic value
    std::cout << "    After change value: " << ::to_string(value) << '\n';
  }

  void example_atomic_waiting(void)
  {
    // create atomic with created state
    flib::atomic<::test_state> value{ ::test_state::created };

    auto processing_task = std::async(std::launch::async, [&value]
      {
        std::cout << "Processor waiting\n";

        // wait until atomic changes to initialized and is notified or until certain duration elapsed
        bool result = value.wait_for(std::chrono::seconds(1),
          std::bind(std::equal_to<::test_state>(), ::test_state::initialized, std::placeholders::_1));

        // print reason for wait stop
        std::cout << "Processor stopped waiting, predicate was " << (result ? "valid" : "invalid") << '\n';

        // print atomic value
        std::cout << "Value set to " << ::to_string(value) << " ... processing\n";

        // change atomic value to processed and notify_all waiters with delay of 1 second
        ::set_and_notify(value, ::test_state::processed, std::chrono::seconds(1));
      });

    auto completion_task = std::async(std::launch::async, [&value]
      {
        std::cout << "Completor waiting\n";

        // wait until atomic changes to processed and is notified or until certain duration elapsed
        bool result = value.wait_for(std::chrono::seconds(2),
          std::bind(std::equal_to<::test_state>(), ::test_state::processed, std::placeholders::_1));

        // print reason for wait stop
        std::cout << "Completor stopped waiting, predicate was " << (result ? "valid" : "invalid") << '\n';

        // print atomic value
        std::cout << "Value set to " << ::to_string(value) << " ... completing\n";

        // change atomic value to completed and notify_all waiters with delay of 1 second
        ::set_and_notify(value, ::test_state::completed, std::chrono::seconds(1));
      });

    auto termination_task = std::async(std::launch::async, [&value]
      {
        std::cout << "Terminator waiting\n";

        // wait until atomic changes to completed and is notified or until certain duration elapsed
        bool result = value.wait_for(std::chrono::seconds(3),
          std::bind(std::equal_to<::test_state>(), ::test_state::completed, std::placeholders::_1));

        // print reason for wait stop
        std::cout << "Terminator stopped waiting, predicate was " << (result ? "valid" : "invalid") << '\n';

        // print atomic value
        std::cout << "Value set to " << ::to_string(value) << '\n';
      });

    // change atomic value to completed and notify_all waiters with delay of 500 milliseconds
    ::set_and_notify(value, ::test_state::initialized, std::chrono::milliseconds(500));

    processing_task.get();
    completion_task.get();
    termination_task.get();
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_basic_atomic_usage();
  ::example_atomic_waiting();

  return 0;
}