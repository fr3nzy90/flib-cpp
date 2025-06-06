// Copyright � 2024-2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <flib/mld.hpp>
#include <flib/timer.hpp>

using namespace std::chrono_literals;

namespace
{
#pragma region Helpers
  class EventTime
  {
  private:
    using clock = std::chrono::high_resolution_clock;

  public:
    std::string elapsed_ms(void)
    {
      return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - m_start_time).count()) + "ms";
    }

    void reset(void)
    {
      m_start_time = clock::now();
    }

  private:
    clock::time_point m_start_time{ clock::now() };
  };

  void sleep_for(const std::chrono::nanoseconds& p_duration)
  {
    std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + p_duration);
  }

  flib::timer::event_t create_timer_event(::EventTime& p_watch, std::chrono::nanoseconds p_duration = 0s)
  {
    return [&p_watch, p_duration]
      {
        std::cout << "Event triggered " << p_watch.elapsed_ms() << " since start\n";

        // simulate event execution duration to 75ms
        ::sleep_for(p_duration);
      };
  }
#pragma endregion

#pragma region Examples
  void example_simple_delay(void)
  {
    // create timer
    flib::timer timer;

    // create example related objects and event
    ::EventTime watch;
    auto event = create_timer_event(watch);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    watch.reset();

    // schedule event to be executed in 200ms without repeats
    timer.schedule(event, 200ms);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    // wait for 500ms
    ::sleep_for(500ms);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';
  }

  void example_periodic_fixed_delay(void)
  {
    // create timer
    flib::timer timer;

    // create example related objects and event
    ::EventTime watch;
    auto event = create_timer_event(watch, 75ms);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    watch.reset();

    // schedule event to be executed in 200ms with fixed delay repeats every 100ms
    timer.schedule(event, 200ms, 100ms, flib::timer::type::fixed_delay);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    // wait for 500ms
    ::sleep_for(500ms);

    // clear timer ... unschedules any scheduled executions
    timer.clear();

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';
  }

  void example_periodic_fixed_rate(void)
  {
    // create timer
    flib::timer timer;

    // create example related objects and event
    ::EventTime watch;
    auto event = create_timer_event(watch, 75ms);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    watch.reset();

    // schedule event to be executed in 200ms with fixed rate repeats every 100ms
    timer.schedule(event, 200ms, 100ms, flib::timer::type::fixed_rate);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    // wait for 500ms
    ::sleep_for(500ms);

    // clear timer ... unschedules any scheduled executions
    timer.clear();

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';
  }

  void example_reschedule(void)
  {
    // create timer
    flib::timer timer;

    // create example related objects and event
    ::EventTime watch;
    auto event = create_timer_event(watch);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    watch.reset();

    // schedule event to be executed in 200ms without repeats
    timer.schedule(event, 200ms);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    // wait for 500ms
    ::sleep_for(500ms);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    // reschedule last scheduled execution with same delay and period
    timer.reschedule();

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';

    // wait for 500ms
    ::sleep_for(500ms);

    // check if timer is scheduled
    std::cout << "Timer scheduled: " << std::boolalpha << timer.scheduled() << '\n';
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_simple_delay();
  ::example_periodic_fixed_delay();
  ::example_periodic_fixed_rate();
  ::example_reschedule();

  return 0;
}