#pragma once
#include <optional>
#include <string>
#include <variant>
#include <regex>
#include <string_view>

#include "Structs.h"
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

class Query
{
public: // Interface
  Query(std::string_view _Query, uint8_t AccessModifier = static_cast<uint8_t>(USER_TYPE::ANONYMOUS));

  bool CanProcessQuery(std::string_view _Query) const;

  size_t GetMatchRank(const std::string & _Query) const;

  friend bool operator==(const Query & lhs, const Query & rhs)
  {
    return lhs.m_Pattern == rhs.m_Pattern;
  }

  friend bool operator!=(const Query & lhs, const Query & rhs)
  {
    return !(lhs == rhs);
  }

private:
  std::string m_Pattern;
  std::regex  m_Query;
  uint8_t     m_AccessModifier;
};
