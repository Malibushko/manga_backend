#pragma once
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/parser.hpp>

#include <functional>
#include <memory>

namespace bb = boost::beast;

class Session
{
public:
  using handler_t =
    std::function<bb::http::response<bb::http::string_body>(boost::beast::http::request<bb::http::string_body> &&)>;

  using response_t = bb::http::response<bb::http::string_body>;

  Session(boost::asio::ip::tcp::socket && _Socket, handler_t _Handler);

  ~Session();

  void Read();
  void Write(std::shared_ptr<response_t> _Response);

  void Close();

protected:
  void OnRead(boost::beast::error_code _Error, std::size_t _BytesRead);
  void OnWrite(boost::beast::error_code _Error, std::size_t _BytesWritten);

private:
  bb::http::request_parser<bb::http::string_body> m_RequestBody;
  bb::tcp_stream                                  m_SocketStream;
  bb::flat_buffer                                 m_Buffer;
  handler_t                                       m_Handler;
  std::shared_ptr<response_t>                     m_Response;
};
