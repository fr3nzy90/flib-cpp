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
#include <chrono>
#include <future>
#include <sstream>
#include <thread>

#include <catch2/catch.hpp>

#include "flib/Scriptler.hpp"

TEST_CASE("Scriptler tests - Sanity check", "[Scriptler]")
{
  flib::Scriptler scriptler;
  REQUIRE(0 == scriptler.CommandCount());
  REQUIRE(scriptler.CommandList().empty());
  REQUIRE(scriptler.IsEmpty());
  REQUIRE(!scriptler.IsDefaultSet());
  REQUIRE(!scriptler.IsActive());
  REQUIRE_THROWS_MATCHES(scriptler.Set("unsetCmd"), std::invalid_argument,
    Catch::Matchers::Message("Nothing to erase - invalid command id"));
}

TEST_CASE("Scriptler tests - Setting default", "[Scriptler]")
{
  flib::Scriptler scriptler;
  scriptler.SetDefault([](const flib::Scriptler::Tokens&) {});
  REQUIRE(0 == scriptler.CommandCount());
  REQUIRE(scriptler.IsDefaultSet());
  scriptler.SetDefault();
  REQUIRE(0 == scriptler.CommandCount());
  REQUIRE(!scriptler.IsDefaultSet());
}

TEST_CASE("Scriptler tests - Setting commands", "[Scriptler]")
{
  flib::Scriptler scriptler;
  scriptler.Set("cmd1", [](const flib::Scriptler::Tokens&) {});
  REQUIRE(1 == scriptler.CommandCount());
  REQUIRE(!scriptler.CommandList().empty());
  REQUIRE(!scriptler.IsEmpty());
  REQUIRE(!scriptler.IsDefaultSet());
  scriptler.Set("cmd2", [](const flib::Scriptler::Tokens&) {});
  REQUIRE(2 == scriptler.CommandCount());
  REQUIRE(!scriptler.CommandList().empty());
  REQUIRE(!scriptler.IsEmpty());
  REQUIRE(!scriptler.IsDefaultSet());
  scriptler.Set("cmd1");
  REQUIRE(1 == scriptler.CommandCount());
  REQUIRE(!scriptler.CommandList().empty());
  REQUIRE(!scriptler.IsEmpty());
  REQUIRE(!scriptler.IsDefaultSet());
  scriptler.Set("cmd2");
  REQUIRE(0 == scriptler.CommandCount());
  REQUIRE(scriptler.CommandList().empty());
  REQUIRE(scriptler.IsEmpty());
  REQUIRE(!scriptler.IsDefaultSet());
}

TEST_CASE("Scriptler tests - Setting commands with default", "[Scriptler]")
{
  flib::Scriptler scriptler;
  scriptler.SetDefault([](const flib::Scriptler::Tokens&) {});
  REQUIRE(0 == scriptler.CommandCount());
  REQUIRE(scriptler.IsDefaultSet());
  scriptler.Set("cmd1", [](const flib::Scriptler::Tokens&) {});
  REQUIRE(1 == scriptler.CommandCount());
  REQUIRE(!scriptler.CommandList().empty());
  REQUIRE(!scriptler.IsEmpty());
  REQUIRE(scriptler.IsDefaultSet());
  scriptler.Set("cmd2", [](const flib::Scriptler::Tokens&) {});
  REQUIRE(2 == scriptler.CommandCount());
  REQUIRE(!scriptler.CommandList().empty());
  REQUIRE(!scriptler.IsEmpty());
  REQUIRE(scriptler.IsDefaultSet());
  scriptler.SetDefault();
  REQUIRE(2 == scriptler.CommandCount());
  REQUIRE(!scriptler.IsDefaultSet());
  scriptler.Set("cmd1");
  REQUIRE(1 == scriptler.CommandCount());
  REQUIRE(!scriptler.CommandList().empty());
  REQUIRE(!scriptler.IsEmpty());
  REQUIRE(!scriptler.IsDefaultSet());
  scriptler.Set("cmd2");
  REQUIRE(0 == scriptler.CommandCount());
  REQUIRE(scriptler.CommandList().empty());
  REQUIRE(scriptler.IsEmpty());
  REQUIRE(!scriptler.IsDefaultSet());
}

TEST_CASE("Scriptler tests - Auto start-stop cycle", "[Scriptler]")
{
  flib::Scriptler scriptler;
  std::stringstream stream;
  stream << "command\n";
  scriptler.SetDefault([](const flib::Scriptler::Tokens&)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
  REQUIRE(!scriptler.IsActive());
  auto task = std::async(std::launch::async, [&scriptler, &stream]()
    {
      scriptler.Start(stream);
    });
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(scriptler.IsActive());
  task.get();
  REQUIRE(!scriptler.IsActive());
}

TEST_CASE("Scriptler tests - Manual start-stop cycle", "[Scriptler]")
{
  flib::Scriptler scriptler;
  std::stringstream stream;
  std::atomic<uint32_t> reference(0);
  stream << "command\ncommand\n";
  scriptler.SetDefault([&reference, &scriptler](const flib::Scriptler::Tokens&)
    {
      ++reference;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      scriptler.Stop();
    });
  REQUIRE(!scriptler.IsActive());
  auto task = std::async(std::launch::async, [&scriptler, &stream]()
    {
      scriptler.Start(stream);
    });
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE(scriptler.IsActive());
  task.get();
  REQUIRE(!scriptler.IsActive());
  REQUIRE(1 == reference);
}

TEST_CASE("Scriptler tests - Simple scripting", "[Scriptler]")
{
  flib::Scriptler scriptler;
  std::stringstream stream;
  std::atomic<uint32_t> reference(0);
  scriptler.Set("cmd1", [&reference, &scriptler](const flib::Scriptler::Tokens& tokens)
    {
      reference += flib::Scriptler::Tokens{ "cmd1","1" } == tokens ? 1 : 2;
    });
  scriptler.SetDefault([&reference, &scriptler](const flib::Scriptler::Tokens& tokens)
    {
      reference += flib::Scriptler::Tokens{ "cmd2","2" } == tokens ? 3 : 5;
    });
  REQUIRE(!scriptler.IsActive());
  stream << "cmd1 1\ncmd2 2\n";
  scriptler.Start(stream);
  REQUIRE(!scriptler.IsActive());
  REQUIRE(4 == reference);
  scriptler.Set("cmd1");
  stream.clear();
  stream << "cmd1 1\ncmd2 2\n";
  scriptler.Start(stream);
  REQUIRE(!scriptler.IsActive());
  REQUIRE(12 == reference);
  scriptler.SetDefault();
  stream.clear();
  stream << "cmd1 1\ncmd2 2\n";
  scriptler.Start(stream);
  REQUIRE(!scriptler.IsActive());
  REQUIRE(12 == reference);
}

TEST_CASE("Scriptler tests - Complex scripting", "[Scriptler]")
{
  flib::Scriptler scriptler;
  std::stringstream stream;
  std::atomic<uint32_t> reference(0);
  scriptler.Set("cmd1", [&reference, &scriptler](const flib::Scriptler::Tokens& tokens)
    {
      reference += flib::Scriptler::Tokens{ "cmd1","1","2" } == tokens ? 1 : 2;
    });
  scriptler.SetDefault([&reference, &scriptler](const flib::Scriptler::Tokens&)
    {
      reference = reference * 2u;
    });
  REQUIRE(!scriptler.IsActive());
  stream << "cmd1 1 2\ncmd2 3 4\ncmd3 5 6\n";
  scriptler.Start(stream);
  REQUIRE(!scriptler.IsActive());
  REQUIRE(4 == reference);
  scriptler.Set("cmd1");
  scriptler.Set("cmd2", [&reference, &scriptler](const flib::Scriptler::Tokens& tokens)
    {
      reference += flib::Scriptler::Tokens{ "cmd2","3","4" } == tokens ? 3 : 5;
    });
  stream.clear();
  stream << "cmd1 1 2\ncmd2 3 4\ncmd3 5 6\n";
  scriptler.Start(stream);
  REQUIRE(!scriptler.IsActive());
  REQUIRE(22 == reference);
  scriptler.SetDefault();
  stream.clear();
  stream << "cmd1 1 2\ncmd2 3 4\ncmd3 5 6\n";
  scriptler.Start(stream);
  REQUIRE(!scriptler.IsActive());
  REQUIRE(25 == reference);
}

TEST_CASE("Scriptler tests - Scripting with special token pattern and deliminator", "[Scriptler]")
{
  flib::Scriptler scriptler;
  std::stringstream stream;
  std::atomic<uint32_t> reference(0);
  scriptler.SetDefault([&reference, &scriptler](const flib::Scriptler::Tokens& tokens)
    {
      if (flib::Scriptler::Tokens{ "cmd1","1","2" } == tokens ||
        flib::Scriptler::Tokens{ "cmd2","3","4" } == tokens ||
        flib::Scriptler::Tokens{ "cmd3","5","6" } == tokens)
      {
        ++reference;
      }
    });
  REQUIRE(!scriptler.IsActive());
  stream << "cmd1,1,2;cmd2,3,4;cmd3,5,6;";
  scriptler.Start(stream, "[^,]+", ';');
  REQUIRE(!scriptler.IsActive());
  REQUIRE(3 == reference);
}

TEST_CASE("Scriptler tests - Script command setting", "[Scriptler]")
{
  flib::Scriptler scriptler;
  std::stringstream stream;
  std::atomic<uint32_t> reference(1);
  scriptler.SetDefault([&reference, &scriptler](const flib::Scriptler::Tokens&)
    {
      reference = reference * 2u;
    });
  scriptler.Set("add", [&reference, &scriptler](const flib::Scriptler::Tokens&)
    {
      scriptler.Set("cmd", [&reference, &scriptler](const flib::Scriptler::Tokens&)
        {
          ++reference;
        });
    });
  scriptler.Set("rm", [&reference, &scriptler](const flib::Scriptler::Tokens&)
    {
      scriptler.Set("cmd");
    });
  REQUIRE(!scriptler.IsActive());
  stream << "cmd\nadd\ncmd\nrm\ncmd\n";
  scriptler.Start(stream);
  REQUIRE(!scriptler.IsActive());
  REQUIRE(6 == reference);
}