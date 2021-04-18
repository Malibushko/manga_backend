#include "Query.h"

Query::Query(std::string_view _Query, uint8_t _AccessModifier)
  : m_Pattern{_Query}, m_Query{m_Pattern}, m_AccessModifier{_AccessModifier}
{
}

bool Query::CanProcessQuery(std::string_view _Query) const
{
  if (!m_Pattern.size())
    return false;

  return std::regex_search(_Query.begin(), _Query.end(), m_Query);
}

size_t Query::GetMatchRank(const std::string & _Query) const
{
  std::smatch Match;
  std::regex_match(_Query, Match, m_Query);
  return Match.size();
}
