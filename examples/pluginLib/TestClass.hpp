#pragma once

#include <string>

#include "TestAPI.hpp"

class TestClass
  : public TestAPI
{
public:
  TestClass(const std::string& name);
  ~TestClass(void);

  void SayHi(void) override;
private:
  const std::string mName;
};