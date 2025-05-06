// Copyright © 2024-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

#include <flib/mld.hpp>
#include <flib/worker.hpp>

using namespace std::chrono_literals;

namespace
{
#pragma region Helpers
  void sleep_for(const std::chrono::nanoseconds& p_duration)
  {
    std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + p_duration);
  }

  void print(const std::string& p_text)
  {
    std::ostringstream outText;
    outText << p_text << " executed (tid=" << std::this_thread::get_id() << ")\n";
    std::cout << outText.str();
  }

  std::function<void(void)> do_work_after(const std::chrono::nanoseconds& p_duration, const std::string& p_message = "work")
  {
    return [p_duration, p_message]
      {
        ::sleep_for(p_duration);
        ::print(p_message);
      };
  }
#pragma endregion

#pragma region Examples
  void example_basic(void)
  {
    // create default enabled worker with one executor
    flib::worker worker;

    // print main thread id
    ::print("main");

    // invoke some work
    auto invocation = worker.invoke(::do_work_after(200ms));

    // check if invocation is expired
    std::cout << "   Invocation expired: " << std::boolalpha << invocation.expired() << '\n';

    // wait for everything to complete
    ::sleep_for(1s);

    // check if invocation is expired
    std::cout << "   Invocation expired: " << std::boolalpha << invocation.expired() << '\n';
  }

  void example_checks(void)
  {
    // create default enabled worker with one executor
    flib::worker worker;

    // check if worker is enabled
    std::cout << "         Worker enabled: " << std::boolalpha << worker.enabled() << '\n';

    // check worker executor count
    std::cout << "  Worker executor count: " << worker.executors() << '\n';

    // check if worker is empty
    std::cout << "           Worker empty: " << std::boolalpha << worker.empty() << '\n';

    // check worker queued tasks count
    std::cout << "            Worker size: " << worker.size() << '\n';

    // create an delaying invocation
    worker.invoke(::do_work_after(200ms));

    // create a pending invocation
    auto invocation = worker.invoke(::do_work_after(0s));

    // check ownership of invocation
    std::cout << "Worker invocation owner: " << worker.owner(invocation) << '\n';

    // wait for everything to complete
    ::sleep_for(1s);

    // check ownership of invocation
    std::cout << "Worker invocation owner: " << worker.owner(invocation) << '\n';
  }

  void example_multiple_executors(void)
  {
    // create default enabled worker with two executors
    flib::worker worker(true, 2);

    // create two invocations that will be executed in parallel
    worker.invoke(::do_work_after(200ms, "task1"));
    worker.invoke(::do_work_after(200ms, "task2"));

    // wait for everything to complete
    ::sleep_for(1s);
  }

  void example_disabling(void)
  {
    // create default disabled worker with one executor
    flib::worker worker(false);

    // invocation will wait until executor is enabled
    worker.invoke(::do_work_after(0s, "task1"));

    // enable worker
    worker.enable();

    // wait for invocation to complete
    ::sleep_for(200ms);

    // disable worker
    worker.disable();

    // invocation will wait until executor is enabled
    worker.invoke(::do_work_after(0s, "task2"));

    // wait, just to be sure
    ::sleep_for(200ms);
  }

  void example_invocation_cancellation(void)
  {
    // create default enabled worker with one executor
    flib::worker worker;

    // create an delaying invocation
    worker.invoke(::do_work_after(200ms));

    // create a pending invocation
    auto invocation = worker.invoke(::do_work_after(0s));

    // cancel the invocation - alternative code: worker.cancel(invocation);
    invocation.cancel();

    // check ownership of invocation
    std::cout << "Worker invocation owner: " << worker.owner(invocation) << '\n';

    // wait for everything to complete
    ::sleep_for(1s);
  }

  void example_priority_invocations(void)
  {
    // create default enabled worker with one executor
    flib::worker worker;

    // create an delaying invocation
    worker.invoke(::do_work_after(200ms));

    // invoke some work with regular priority
    worker.invoke(::do_work_after(0s, "work - normal"));

    // invoke some work with higher priority
    worker.invoke(::do_work_after(0s, "work - priority"), 1);

    // wait for everything to complete
    ::sleep_for(1s);
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_basic();
  ::example_checks();
  ::example_multiple_executors();
  ::example_disabling();
  ::example_invocation_cancellation();
  ::example_priority_invocations();

  return 0;
}