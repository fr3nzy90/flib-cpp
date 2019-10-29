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

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <regex>
#include <string>
#include <stdexcept>

namespace flib
{
  class Scriptler
  {
  public:
    using Stream = std::istream;
    using Tokens = std::list<std::string>;
    using Command = std::function<void(const Tokens&)>;

    inline Scriptler(void);
    inline Scriptler(const Scriptler&) = delete;
    inline Scriptler(Scriptler&&) = default;
    inline ~Scriptler(void) noexcept;
    inline Scriptler& operator=(const Scriptler&) = delete;
    inline Scriptler& operator=(Scriptler&&) = default;
    inline std::size_t CommandCount(void) const;
    inline std::list<std::string> CommandList(void) const;
    inline bool IsActive(void) const;
    inline bool IsDefaultSet(void) const;
    inline bool IsEmpty(void) const;
    inline void Set(const std::string& id, const Command& command = {});
    inline void SetDefault(const Command& command = {});
    inline void Start(Stream& scriptStream, const std::string& tokenPattern = "([^\"\\s]*\"[^\"]*\")|[^\"\\s]+",
      const char commandDeliminator = '\n');
    inline void Stop(void);

  private:
    enum class State
    {
      Active,
      Idle
    };

    std::atomic<State> mState;
    std::map<std::string, Command> mCommands;
    Command mDefaultCommand;
    mutable std::recursive_mutex mCommandsAccessLock;
  };
}

// IMPLEMENTATION

flib::Scriptler::Scriptler(void)
  : mState(State::Idle)
{
}

flib::Scriptler::~Scriptler(void) noexcept
{
  Stop();
}

std::size_t flib::Scriptler::CommandCount(void) const
{
  std::lock_guard<decltype(mCommandsAccessLock)> commandsAccessGuard(mCommandsAccessLock);
  return mCommands.size();
}

std::list<std::string> flib::Scriptler::CommandList(void) const
{
  std::lock_guard<decltype(mCommandsAccessLock)> commandsAccessGuard(mCommandsAccessLock);
  std::list<std::string> result;
  for (const auto& commandPair : mCommands)
  {
    result.push_back(commandPair.first);
  }
  return result;
}

bool flib::Scriptler::IsActive(void) const
{
  return State::Active == mState;
}

bool flib::Scriptler::IsDefaultSet(void) const
{
  std::lock_guard<decltype(mCommandsAccessLock)> commandsAccessGuard(mCommandsAccessLock);
  return mDefaultCommand ? true : false;
}

bool flib::Scriptler::IsEmpty(void) const
{
  std::lock_guard<decltype(mCommandsAccessLock)> commandsAccessGuard(mCommandsAccessLock);
  return mCommands.empty();
}

void flib::Scriptler::Set(const std::string& id, const Command& command)
{
  std::lock_guard<decltype(mCommandsAccessLock)> commandsAccessGuard(mCommandsAccessLock);
  if (command)
  {
    mCommands[id] = command;
  }
  else
  {
    if (0 == mCommands.erase(id))
    {
      throw std::invalid_argument("Nothing to erase - invalid command id");
    }
  }
}

void flib::Scriptler::SetDefault(const Command& command)
{
  std::lock_guard<decltype(mCommandsAccessLock)> commandsAccessGuard(mCommandsAccessLock);
  mDefaultCommand = command;
}

void flib::Scriptler::Start(Stream& scriptStream, const std::string& tokenPattern, const char commandDeliminator)
{
  mState = State::Active;
  std::string commandLine;
  std::regex tokenRegex(tokenPattern);
  Tokens tokens;
  Command command;
  while (State::Active == mState && !scriptStream.eof())
  {
    std::getline(scriptStream, commandLine, commandDeliminator);
    if (!scriptStream.good())
    {
      continue;
    }
    std::sregex_token_iterator tokenIt(commandLine.cbegin(), commandLine.cend(), tokenRegex);
    tokens = { tokenIt,{} };
    command = {};
    {
      std::lock_guard<decltype(mCommandsAccessLock)> commandsAccessGuard(mCommandsAccessLock);
      if (!tokens.empty())
      {
        auto it = mCommands.find(*tokens.cbegin());
        if (mCommands.cend() != it)
        {
          command = it->second;
        }
      }
      if (!command && mDefaultCommand)
      {
        command = mDefaultCommand;
      }
    }
    if (command)
    {
      command(tokens);
    }
  }
  mState = State::Idle;
}

void flib::Scriptler::Stop(void)
{
  mState = State::Idle;
}