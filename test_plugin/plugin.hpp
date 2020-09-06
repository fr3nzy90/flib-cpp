#pragma once

#include "iplugin.hpp"

namespace testing
{
  class plugin
    : public iplugin
  {
  public:
    plugin(const std::string& module);

    std::string name(void) const override;

  private:
    std::string m_module;
  };
}