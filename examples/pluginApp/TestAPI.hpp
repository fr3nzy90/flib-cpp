#pragma once

class TestAPI
{
public:
  virtual ~TestAPI(void) = default;
  virtual void SayHi(void) = 0;
};