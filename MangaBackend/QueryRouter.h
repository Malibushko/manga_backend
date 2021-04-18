#pragma once
#include <functional>
#include <map>
#include <optional>

#include "Query.h"
#include "QueryRouterBase.h"

class QueryRouter : public QueryRouterBase
{
public:
  QueryRouter();

  response_t Route(const request_t & _Query) const override;

  bool AddRoute(const Query & _Query, const std::function<response_t(const request_t &)> & _Handler);

  bool RemoveRoute(const Query & _Query);

protected:
  response_t CreateDefaultErrorResponse() const;

private:
  std::string                                                                 m_DocRoot;
  std::vector<std::pair<Query, std::function<response_t(const request_t &)>>> m_RouteTable;
};
