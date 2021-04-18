#include "QueryRouter.h"
#include <boost/beast/version.hpp>

QueryRouter::QueryRouter()
{
}

QueryRouterBase::response_t QueryRouter::Route(const request_t & _Query) const
{
  std::string _QueryPath = _Query.target().to_string();

  for (const auto & [Query, Handler] : m_RouteTable)
    if (Query.CanProcessQuery(_QueryPath))
      return Handler(_Query);

  return CreateDefaultErrorResponse();
}

bool QueryRouter::AddRoute(const Query & _Query, const std::function<response_t(const request_t &)> & _Handler)
{
  auto ItemIter = std::find_if(m_RouteTable.begin(), m_RouteTable.end(),
                               [_Query](const auto & QueryAndHandler) { return QueryAndHandler.first == _Query; });

  if (ItemIter == m_RouteTable.end())
  {
    m_RouteTable.emplace_back(_Query, _Handler);
    return false;
  }
  else
  {
    ItemIter->second = _Handler;
    return true;
  }
}

bool QueryRouter::RemoveRoute(const Query & _Query)
{
  for (auto it = m_RouteTable.begin(); it != m_RouteTable.end(); ++it)
  {
    if (it->first == _Query)
    {
      m_RouteTable.erase(it);
      return true;
    }
  }
  return false;
}

QueryRouterBase::response_t QueryRouter::CreateDefaultErrorResponse() const
{
  using namespace boost::beast::http;

  response<string_body> Response{status::not_implemented, 11};
  Response.set(field::server, BOOST_BEAST_VERSION_STRING);
  Response.set(field::content_type, "text/html");
  Response.keep_alive(false);
  Response.body() = "Not implemented.";
  Response.prepare_payload();

  return Response;
}
