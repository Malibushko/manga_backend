#include "Server.h"

#include "Structs.h"
#include "Session.h"
#include "Utils.h"

#include "utility/Base64.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace details
{
auto CreateDefaultResponse()
{
  return [](const QueryRouter::request_t & _Req) {
    using namespace boost::beast::http;
    QueryRouter::response_t Response{status::ok, _Req.version()};
    Response.set(field::server, BOOST_BEAST_VERSION_STRING);
    Response.set(field::content_type, "application/json");
    Response.keep_alive(_Req.keep_alive());
    return Response;
  };
}

template <class T, uint8_t AccessModifier = static_cast<int>(USER_TYPE::ANONYMOUS)>
auto CreateDefaultGetResponse(const QueryRouter::request_t & _Req)
{
  using namespace boost::beast::http;

  QueryRouter::response_t Response{status::ok, _Req.version()};
  ResponseJson            Json{0, "OK"};
  auto                    Params = utility::Parse(_Req.target().to_string());

  if (AccessModifier != USER_TYPE::ANONYMOUS)
  {
    if (!Params.count("AuthToken"))
    {
      Json.Error   = static_cast<int>(status::unauthorized);
      Json.Message = "Missing 'AuthToken' field that is required for this request";
    }
    auto   Token   = AuthToken::fromString(Params["AuthToken"]);
    auto & Storage = MangaDatabase::Instance().GetStorage();
    using namespace sqlite_orm;

    if (!Token || !Storage->get_all<SessionData>(where(c(&SessionData::AuthToken) == Token->toString())).size() ||
        Token->ExpiresAt < ::time(0))
    {
      Json.Error   = static_cast<int>(status::bad_request);
      Json.Message = "AuthToken is invalid.";
    }
    else
    {
      if (Token->AccessModifier < static_cast<uint8_t>(AccessModifier))
      {
        Json.Error   = static_cast<int>(status::unauthorized);
        Json.Message = "You dont have permissions for this request";
      }
    }
  }
  if (!Json.Error)
  {
    if (auto Id = Params.find("id"); Id != Params.end())
    {
      using namespace sqlite_orm;
      auto & Storage = MangaDatabase::Instance().GetStorage();

      auto Result = Storage->get_all<T>(where(c(&T::Id) == Id->second));
      if (Result.size() == 1)
      {
        Json.Message = Result[0].toJson();
      }
      else
      {
        Json.Error   = static_cast<int>(status::not_found);
        Json.Message = "File with specified ID not found";
      }
    }
    else
    {
      Json.Error   = static_cast<int>(status::bad_request);
      Json.Message = "Invalid query";
    }
  }

  Response.body() = Json.toJson();
  Response.prepare_payload();
  return Response;
}

template <class T, uint8_t AccessModifier = static_cast<int>(USER_TYPE::ANONYMOUS)>
QueryRouter::response_t CreateDefaultInsertResponse(const QueryRouter::request_t & _Req)
{
  using namespace boost::beast::http;

  QueryRouter::response_t Response(CreateDefaultResponse()(_Req));
  ResponseJson            Json{0, "OK"};

  if (_Req.method() != verb::post)
  {
    Json.Error   = static_cast<int>(status::bad_request);
    Json.Message = "Only POST method is allowed for this operation";
  }
  else
  {
    T Object{};
    try
    {
      using namespace sqlite_orm;

      if (AccessModifier != USER_TYPE::ANONYMOUS)
      {
        nlohmann::json Parsed = nlohmann::json::parse(_Req.body());
        if (!Parsed.contains("AuthToken"))
        {
          Json.Error   = static_cast<int>(status::unauthorized);
          Json.Message = "Missing 'AuthToken' field that is required for this request";
        }
        auto   Token   = AuthToken::fromString(Parsed["AuthToken"].get<std::string>());
        auto & Storage = MangaDatabase::Instance().GetStorage();

        if (!Token || Storage->get_all<SessionData>(where(c(&SessionData::AuthToken) == Token->toString())).size())
        {
          Json.Error   = static_cast<int>(status::bad_request);
          Json.Message = "AuthToken is invalid.";
        }
        else
        {
          if (Token->AccessModifier < static_cast<uint8_t>(AccessModifier) || Token->ExpiresAt < ::time(0))
          {
            Json.Error   = static_cast<int>(status::unauthorized);
            Json.Message = "You dont have permissions for this request";
          }
        }
      }
      if (!Json.Error)
        Object = *T::fromJSON(std::string_view{_Req.body().data(), _Req.body().size()});
    }
    catch (std::exception & _Exception)
    {
      spdlog::error("Error occured while processing record: {}", _Exception.what());
      Json.Error   = static_cast<int>(status::bad_request);
      Json.Message = "Invalid or wrong JSON";
    }

    try
    {
      auto & Storage = MangaDatabase::Instance().GetStorage();

      Storage->insert<T>(Object);
    }
    catch (std::exception & /*_Exception*/)
    {
      Json.Error   = static_cast<int>(status::conflict);
      Json.Message = "Record with the same attributes already exist";
    }
  }

  Response.body() = Json.toJson();
  Response.prepare_payload();

  return Response;
}

} // namespace details
Server::Server(uint16_t _Port) : m_Acceptor(m_IO)
{
  using namespace boost::asio;

  ip::tcp::endpoint Endpoint(ip::tcp::v4(), _Port);

  m_Acceptor.open(Endpoint.protocol());
  m_Acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
  m_Acceptor.bind(Endpoint);

  SetRouter(CreateRouter());
}

Server::~Server()
{
}

void Server::Start()
{
  m_Acceptor.listen();

  DoAccept();

  m_IO.run();
}

void Server::SetRouter(std::shared_ptr<QueryRouterBase> _Router)
{
  m_QueryRouter = _Router;
}

void Server::DoAccept()
{
  m_Acceptor.async_accept(boost::asio::make_strand(m_IO),
                          [this](auto &&... args) { OnAccept(std::forward<decltype(args)>(args)...); });
}

void Server::OnAccept(boost::beast::error_code _Error, boost::asio::ip::tcp::socket _Socket)
{
  using namespace boost::beast::http;

  if (!_Error)
  {
    m_Sessions.emplace(
      std::make_unique<Session>(std::move(_Socket), [&](request<string_body> && _Request) -> response<string_body> {
        return m_QueryRouter->Route(std::move(_Request));
      }));
  }

  DoAccept();
}

std::shared_ptr<QueryRouterBase> Server::CreateRouter() const
{
  using namespace boost::beast::http;

  auto Router = std::make_shared<QueryRouter>();

  Router->AddRoute(Query{"/user/register/*"}, details::CreateDefaultInsertResponse<User>);

  Router->AddRoute(Query{"/user/get*"}, details::CreateDefaultGetResponse<User, static_cast<int>(USER_TYPE::ADMIN)>);

  Router->AddRoute(Query{"/user/login*"}, [](const QueryRouter::request_t & _Req) {
    QueryRouter::response_t Response = details::CreateDefaultResponse()(_Req);
    ResponseJson            Json{0, "OK"};
    auto                    Params = utility::Parse(_Req.target().to_string());

    for (const char * RequiredParam : {"login", "password_hash"})
    {
      if (!Params.count(RequiredParam))
      {
        Json.Error   = static_cast<int>(status::bad_request);
        Json.Message = "Missing field in query string";
        break;
      }
    }

    if (!Json.Error)
    {
      using namespace sqlite_orm;

      auto & Storage = MangaDatabase::Instance().GetStorage();

      auto Users = Storage->get_all<User>(
        where(c(&User::Login) == Params["login"] && c(&User::PasswordHash) == Params["password_hash"]));

      if (Users.size() < 1)
      {
        Json.Error   = static_cast<int>(status::not_found);
        Json.Message = "User with the specified login and password not found.";
      }
      else
      {
        SessionData UserSession;
        UserSession.UserId    = Users[0].Id;
        UserSession.ExpiresAt = ::time(nullptr) + 86400;
        UserSession.SetToken(USER_TYPE::ADMIN);
        try
        {
          UserSession.Id = Storage->insert<SessionData>(UserSession);

          Json.Message = UserSession.toJson();
        }
        catch (std::exception & _Exception)
        {
          spdlog::error("An exception was caught while creating session: {}", _Exception.what());

          Json.Error   = static_cast<int>(status::internal_server_error);
          Json.Message = "Error occured while creating session. Try again later.";
        }
      }
    }
    Response.body() = Json.toJson();

    Response.prepare_payload();

    return Response;
  });

  Router->AddRoute(Query{"/manga_blob/upload*"},
                   details::CreateDefaultInsertResponse<MangaBlob, static_cast<int>(USER_TYPE::ADMIN)>);

  Router->AddRoute(Query{"/manga_blob/get*"}, details::CreateDefaultGetResponse<MangaBlob>);

  Router->AddRoute(Query{"/manga/get*"}, details::CreateDefaultGetResponse<Manga>);

  Router->AddRoute(Query{"/manga/create*"}, details::CreateDefaultInsertResponse<Manga>);

  return Router;
}
