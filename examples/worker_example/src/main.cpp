// Copyright © 2024-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

#include <flib/mld.hpp>
#include <flib/worker.hpp>

namespace
{
#pragma region Helpers
  using milliseconds = std::chrono::duration<uint64_t, std::milli>;

  void sleep_for(const ::milliseconds& p_duration)
  {
    std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + p_duration);
  }

  void print(const std::string& p_text)
  {
    std::ostringstream outText;
    outText << p_text << " executed (tid=" << std::this_thread::get_id() << ")\n";
    std::cout << outText.str();
  }

  std::function<void(void)> do_work_after(const ::milliseconds& p_duration, const std::string& p_message = "work")
  {
    return [p_duration, p_message]()
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
    auto invocation = worker.invoke(::do_work_after(::milliseconds(200)));

    // check if invocation is expired
    std::cout << "   Invocation expired: " << std::boolalpha << invocation.expired() << '\n';

    // wait for everything to complete
    ::sleep_for(::milliseconds(1000));

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
    worker.invoke(::do_work_after(::milliseconds(200)));

    // create a pending invocation
    auto invocation = worker.invoke(::do_work_after(::milliseconds(0)));

    // check ownership of invocation
    std::cout << "Worker invocation owner: " << worker.owner(invocation) << '\n';

    // wait for everything to complete
    ::sleep_for(::milliseconds(1000));

    // check ownership of invocation
    std::cout << "Worker invocation owner: " << worker.owner(invocation) << '\n';
  }

  void example_multiple_executors(void)
  {
    // create default enabled worker with two executors
    flib::worker worker(true, 2);

    // create two invocations that will be executed in parallel
    worker.invoke(::do_work_after(::milliseconds(200), "task1"));
    worker.invoke(::do_work_after(::milliseconds(200), "task2"));

    // wait for everything to complete
    ::sleep_for(::milliseconds(1000));
  }

  void example_disabling(void)
  {
    // create default disabled worker with one executor
    flib::worker worker(false);

    // invocation will wait until executor is enabled
    worker.invoke(::do_work_after(::milliseconds(0), "task1"));

    // enable worker
    worker.enable();

    // wait for invocation to complete
    ::sleep_for(::milliseconds(200));

    // disable worker
    worker.disable();

    // invocation will wait until executor is enabled
    worker.invoke(::do_work_after(::milliseconds(0), "task2"));

    // wait, just to be sure
    ::sleep_for(::milliseconds(200));
  }

  void example_invocation_cancellation(void)
  {
    // create default enabled worker with one executor
    flib::worker worker;

    // create an delaying invocation
    worker.invoke(::do_work_after(::milliseconds(200)));

    // create a pending invocation
    auto invocation = worker.invoke(::do_work_after(::milliseconds(0)));

    // cancel the invocation - alternative code: worker.cancel(invocation);
    invocation.cancel();

    // check ownership of invocation
    std::cout << "Worker invocation owner: " << worker.owner(invocation) << '\n';

    // wait for everything to complete
    ::sleep_for(::milliseconds(1000));
  }

  void example_priority_invocations(void)
  {
    // create default enabled worker with one executor
    flib::worker worker;

    // create an delaying invocation
    worker.invoke(::do_work_after(::milliseconds(200)));

    // invoke some work with regular priority
    worker.invoke(::do_work_after(::milliseconds(0), "work - normal"));

    // invoke some work with higher priority
    worker.invoke(::do_work_after(::milliseconds(0), "work - priority"), 1);

    // wait for everything to complete
    ::sleep_for(::milliseconds(1000));
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