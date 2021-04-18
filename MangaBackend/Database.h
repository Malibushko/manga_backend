#pragma once
#include <string_view>

class IDatabase
{
public:
  virtual bool Open(std::string_view _DatabaseName) = 0;

  virtual ~IDatabase(){};
};
