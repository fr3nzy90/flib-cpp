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

#include <chrono>
#include <exception>
#include <string>
#include <thread>

#include <catch2/catch.hpp>

namespace testing
{
  template<class T>
  inline void sleep_for(const T& duration)
  {
    std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + duration);
  }

  class exception_message_starts_with_matcher
    : public Catch::MatcherBase<std::exception>
  {
  public:
    exception_message_starts_with_matcher(const std::string& message)
      : m_message(message)
    {
    }

    bool match(const std::exception& ex) const override
    {
      return Catch::startsWith(ex.what(), m_message);
    }

    std::string describe(void) const override
    {
      return "exception message does not starts with \"" + m_message + "\"";
    }

  private:
    std::string m_message;
  };

  inline exception_message_starts_with_matcher starts_with(std::string const& message)
  {
    return exception_message_starts_with_matcher(message);
  }
}