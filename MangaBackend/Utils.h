#pragma once
#include <regex>
#include <map>
#include <string>

namespace utility
{
inline std::map<std::string, std::string> Parse(const std::string & query)
{
  std::map<std::string, std::string> data;
  std::regex                         pattern("([\\w+%]+)=([^&]*)");
  auto                               words_begin = std::sregex_iterator(query.begin(), query.end(), pattern);
  auto                               words_end   = std::sregex_iterator();

  for (std::sregex_iterator i = words_begin; i != words_end; i++)
  {
    std::string key   = (*i)[1].str();
    std::string value = (*i)[2].str();
    data[key]         = value;
  }

  return data;
}
} // namespace utility
