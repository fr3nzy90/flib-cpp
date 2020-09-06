#pragma once

#include <string>

namespace testing
{
  class iplugin
  {
  public:
    virtual ~iplugin(void) = default;

    virtual std::string name(void) const = 0;
  };
}