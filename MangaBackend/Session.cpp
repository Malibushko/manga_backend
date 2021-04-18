#include "Session.h"
#include <boost/beast/http.hpp>
#include <boost/asio/dispatch.hpp>

#include <spdlog/spdlog.h>

Session::Session(boost::asio::ip::tcp::socket && _Socket, handler_t _Handler)
  : m_SocketStream(std::move(_Socket)), m_Handler(_Handler)
{

  boost::asio::dispatch(m_SocketStream.get_executor(), boost::beast::bind_front_handler(&Session::Read, this));
}

Session::~Session()
{
  Close();
}

void Session::Read()
{
  m_RequestBody.body_limit(INT_MAX);

  m_SocketStream.expires_after(std::chrono::seconds(30));

  // Read a request
  boost::beast::http::async_read(m_SocketStream, m_Buffer, m_RequestBody,
                                 boost::beast::bind_front_handler(&Session::OnRead, this));
}

void Session::Write(std::shared_ptr<response_t> _Response)
{
  boost::beast::http::async_write(m_SocketStream, *_Response,
                                  boost::beast::bind_front_handler(&Session::OnWrite, this));
}

void Session::Close()
{
  m_SocketStream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
}

void Session::OnRead(boost::beast::error_code _Error, size_t /*_BytesRead*/)
{
  if (_Error == boost::beast::http::error::end_of_stream)
    return Close();

  if (_Error)
  {
    spdlog::error("Error occured while reading: {}", _Error.message());
  }
  else
  {
    m_Response = std::make_shared<response_t>(m_Handler(std::move(m_RequestBody.get())));
    Write(m_Response);
  }
}

void Session::OnWrite(boost::beast::error_code _Error, size_t /*_BytesWritten*/)
{
  if (_Error)
  {
    spdlog::error("Error occured while writing: {}", _Error.message());
  }
  else
  {
    Close();
  }
}
