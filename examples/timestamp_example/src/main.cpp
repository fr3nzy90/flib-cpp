// Copyright © 2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <chrono>
#include <iostream>

#include <flib/mld.hpp>
#include <flib/timestamp.hpp>

namespace
{
#pragma region Examples
  void example_simple_timestamp_usage(void)
  {
    // create timestamp with current timepoint
    auto timestamp = flib::timestamp();

    // convert timestamp to UTC timestamp with second precision
    std::cout << timestamp.to_string() << '\n';

    // convert timestamp to local timestamp with second precision
    std::cout << timestamp.to_string(false) << '\n';

    // convert timestamp to UTC timestamp with millisecond precision
    std::cout << timestamp.to_string(true, flib::timestamp::precision::milliseconds) << '\n';

    // convert timestamp to local timestamp with microsecond precision
    std::cout << timestamp.to_string(false, flib::timestamp::precision::microseconds) << '\n';

    // create timestamp with custom timepoint
    timestamp = flib::timestamp(flib::timestamp::time_point_t::clock::now());

    // convert timestamp to UTC timestamp with second precision
    std::cout << timestamp.to_string() << '\n';

    // check if timestamp is equal to epoch
    std::cout << (timestamp == flib::timestamp::epoch()) << '\n';
    std::cout << (timestamp != flib::timestamp::epoch()) << '\n';
  }

  void example_timestamp_parsing(void)
  {
    // create base timestamp with current timepoint
    // - just for example, in real applications this might be timestamp stored in json message or in some string variable/field
    auto timestamp = flib::timestamp();

    // print timestamp
    std::cout << timestamp.to_string(true, flib::timestamp::precision::max) << '\n';

    // parse UTC timestamp with second precision
    auto timestamp1 = flib::timestamp::parse(timestamp.to_string());

    // print timestamp
    std::cout << timestamp1.to_string(true, flib::timestamp::precision::max) << '\n';

    // parse local timestamp with millisecond precision
    auto timestamp2 = flib::timestamp::parse(timestamp.to_string(false, flib::timestamp::precision::milliseconds));

    // print timestamp
    std::cout << timestamp2.to_string(true, flib::timestamp::precision::max) << '\n';

    // parse UTC timestamp with max precision
    auto timestamp3 = flib::timestamp::parse(timestamp.to_string(true, flib::timestamp::precision::max));

    // print timestamp
    std::cout << timestamp3.to_string(true, flib::timestamp::precision::max) << '\n';
  }

  void example_timestamp_reset(void)
  {
    // create base timestamp with epoch timepoint
    auto timestamp = flib::timestamp::epoch();

    // print timestamp
    std::cout << timestamp.to_string(true, flib::timestamp::precision::max) << '\n';

    // advance timestamp 1 hour
    timestamp.set(timestamp.get() + std::chrono::seconds(3600));

    // print timestamp
    std::cout << timestamp.to_string(true, flib::timestamp::precision::max) << '\n';

    // reset timestamp to current time
    timestamp.set();

    // print timestamp
    std::cout << timestamp.to_string(true, flib::timestamp::precision::max) << '\n';
  }

  void example_timestamp_alternative(void)
  {
    // create timestamp with current timepoint
    auto timestamp = flib::timestamp();

    // convert timestamp to UTC timestamp with second precision using alternative syntax
    std::cout << flib::to_string(timestamp) << '\n';

    // convert timestamp to local timestamp with second precision using alternative syntax
    std::cout << flib::to_string(timestamp, false) << '\n';

    // convert timestamp to UTC timestamp with millisecond precision using alternative syntax
    std::cout << flib::to_string(timestamp, true, flib::timestamp::precision::milliseconds) << '\n';

    // convert timestamp to local timestamp with microsecond precision using alternative syntax
    std::cout << flib::to_string(timestamp, false, flib::timestamp::precision::microseconds) << '\n';
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_simple_timestamp_usage();
  ::example_timestamp_parsing();
  ::example_timestamp_reset();
  ::example_timestamp_alternative();

  return 0;
}