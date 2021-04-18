#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include <memory>
#include <set>

#include "QueryRouter.h"
#include "MangaDatabase.h"

class Session;

class Server
{
public: // Interface
  Server(uint16_t _Port);
  ~Server();

  void Start();

  void Stop();

  void SetRouter(std::shared_ptr<QueryRouterBase> _Router);

protected: // Service
  void DoAccept();

  void OnAccept(boost::beast::error_code _Error, boost::asio::ip::tcp::socket _Socket);

  void OnRead();

  void OnWrite();

  std::shared_ptr<QueryRouterBase> CreateRouter() const;

private: // Members
  std::string m_RootPath;

  std::set<std::unique_ptr<Session>> m_Sessions;

  std::shared_ptr<QueryRouterBase> m_QueryRouter;

  boost::asio::io_context        m_IO;
  boost::asio::ip::tcp::acceptor m_Acceptor;
};
