#pragma once

#include "TestAPI.hpp"

class TestClass
  : public TestAPI
{
public:
  TestClass(void);
  ~TestClass(void);

  void SayHi(void) override;
};