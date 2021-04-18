#pragma once
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

class Query;

class QueryRouterBase
{

public: // Interface
  using response_t = boost::beast::http::response<boost::beast::http::string_body>;
  using request_t  = boost::beast::http::request<boost::beast::http::string_body>;

  virtual response_t Route(const request_t &_Query) const = 0;
};
