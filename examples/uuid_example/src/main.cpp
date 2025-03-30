// Copyright © 2025 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#include <ios>
#include <iostream>

#include <flib/mld.hpp>
#include <flib/uuid.hpp>

namespace
{
#pragma region Helpers
  void print(const flib::uuid& p_uuid)
  {
    // convert UUID to string as lowercase
    std::cout << "UUID (lowercase): " << p_uuid.to_string() << '\n';

    // convert UUID to string as uppercase
    std::cout << "UUID (uppercase): " << p_uuid.to_string(true) << '\n';

    // get UUIDs version
    std::cout << "         Version: " << static_cast<int>(p_uuid.get_version()) << '\n';

    // get UUIDs variant
    std::cout << "         Variant: " << static_cast<int>(p_uuid.get_variant()) << '\n';

    // check if UUID is valid
    std::cout << "           Valid: " << std::boolalpha << p_uuid.valid() << '\n';
  }
#pragma endregion

#pragma region Examples
  void example_basic(void)
  {
    // temp data
    flib::uuid::data_t uuid_data1;
    flib::uuid::data_t uuid_data2;

    // generate nil UUID
    flib::uuid uuid = flib::uuid::generate_nil();

    // print UUID related data
    ::print(uuid);

    // generate max UUID
    uuid = flib::uuid::generate_max();

    // print UUID related data
    ::print(uuid);

    // generate v4 UUID
    uuid = flib::uuid::generate_v4();

    // store UUIDs data
    uuid_data1 = uuid.get_data();

    // print UUID related data
    ::print(uuid);

    // generate v7 UUID with current timepoint
    uuid = flib::uuid::generate_v7();

    // store UUIDs data
    uuid_data2 = uuid.get_data();

    // print UUID related data
    ::print(uuid);

    // generate v7 UUID with custom timepoint
    uuid = flib::uuid::generate_v7(flib::uuid::time_point_t::clock::now() + std::chrono::hours(1));

    // print UUID related data
    ::print(uuid);

    // parse given string as UUID
    uuid = flib::uuid::parse("e8be81a6-c70c-4045-87c5-b7505d0c024f");

    // print UUID related data
    ::print(uuid);

    // create UUID with custom data
    uuid = flib::uuid(uuid_data1);

    // print UUID related data
    ::print(uuid);

    // set custom UUID data
    uuid.set_data(uuid_data2);

    // print UUID related data
    ::print(uuid);
  }

  void example_comparison(void)
  {
    // create test UUIDs
    flib::uuid uuid1 = flib::uuid::generate_nil();
    flib::uuid uuid2 = flib::uuid::generate_nil();
    flib::uuid uuid3 = flib::uuid::generate_max();

    // compare UUIDs using == and != operators
    std::cout << "nil == nil: " << std::boolalpha << (uuid1 == uuid2) << '\n';
    std::cout << "nil != nil: " << std::boolalpha << (uuid1 != uuid2) << '\n';
    std::cout << "nil == max: " << std::boolalpha << (uuid1 == uuid3) << '\n';
    std::cout << "nil != max: " << std::boolalpha << (uuid1 != uuid3) << '\n';
  }
#pragma endregion
}

int main(int /*argc*/, char* /*argv*/[])
{
  // create memory leak detector
  flib::memory_leak_detector::setup();

  // examples
  ::example_basic();
  ::example_comparison();

  return 0;
}