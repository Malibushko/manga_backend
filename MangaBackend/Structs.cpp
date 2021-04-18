#include "Structs.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "utility/Base64.h"

std::optional<User> User::fromJSON(std::string_view Json)
{
  try
  {
    if (auto Parsed = nlohmann::json::parse(Json); !Parsed.is_null())
    {
      User Object;
      Object.Login        = Parsed["Login"].get<std::string>();
      Object.PasswordHash = Parsed["PasswordHash"].get<std::string>();

      if (Parsed.contains("ID"))
        Object.Id = Parsed["ID"].get<size_t>();

      if (Parsed.contains("UserType"))
        Object.UserType = Parsed["UserType"].get<size_t>();
      else
        Object.UserType = static_cast<size_t>(USER_TYPE::REGULAR);

      return Object;
    }
  }
  catch (std::exception & _Exception)
  {
    spdlog::error("Exception was thrown while parsing JSON: {}", _Exception.what());
  }

  return std::nullopt;
}

std::string User::toJson() const
{
  nlohmann::json Json;
  Json["ID"]           = Id;
  Json["Login"]        = Login;
  Json["PasswordHash"] = PasswordHash;
  Json["UserType"]     = UserType;

  return Json.dump();
}

std::string ResponseJson::toJson() const
{
  nlohmann::json Json;

  Json["Error"]   = Error;
  Json["Message"] = Message;

  return Json.dump();
}

std::optional<MangaBlob> MangaBlob::fromJSON(std::string_view Json)
{
  try
  {
    if (auto Parsed = nlohmann::json::parse(Json); !Parsed.is_null())
    {
      MangaBlob Object;

      if (Parsed.contains("ID"))
        Object.Id = Parsed["ID"].get<size_t>();

      Object.FileName = Parsed["FileName"].get<std::string>();
      Object.Data     = Parsed["Data"].get<std::vector<char>>();
      return Object;
    }
  }
  catch (std::exception & _Exception)
  {
    spdlog::error("Exception was thrown while parsing JSON: {}", _Exception.what());
  }

  return std::nullopt;
}

std::string MangaBlob::toJson() const
{
  nlohmann::json Json;

  Json["ID"]       = Id;
  Json["FileName"] = FileName;
  Json["Data"]     = Data;

  return Json.dump();
}

std::optional<Manga> Manga::fromJSON(std::string_view Json)
{
  auto  Parsed = nlohmann::json::parse(Json);
  Manga Object;
  Object.Id             = Parsed["ID"].get<size_t>();
  Object.AccessModifier = Parsed["AccessModifier"].get<size_t>();

  if (Parsed.contains("DataBlobID"))
    Object.DataBlobId = Parsed["DataBlobID"].get<size_t>();
  if (Parsed.contains("UploadDate"))
    Object.UploadDate = Parsed["UploadDate"].get<int64_t>();
  if (Parsed.contains("LastChangeDate"))
    Object.LastChangeDate = Parsed["LastChangeDate"].get<int64_t>();
  if (Parsed.contains("UploadUserID"))
    Object.Id = Parsed["UploadUserID"].get<size_t>();

  return Object;
}

std::string Manga::toJson() const
{
  nlohmann::json Json;

  Json["ID"]             = Id;
  Json["DataBlobID"]     = DataBlobId;
  Json["UploadDate"]     = UploadDate;
  Json["UploadUserID"]   = UploadUserId;
  Json["LastChangeDate"] = LastChangeDate;
  Json["AccessModifier"] = AccessModifier;

  return Json.dump();
}

std::string SessionData::SetToken(uint8_t AccessModifier)
{
  ::AuthToken Token_;
  Token_.AccessModifier = AccessModifier;
  Token_.ExpiresAt      = ExpiresAt;
  Token_.UserId         = UserId;

  AuthToken = Token_.toString();
  return AuthToken;
}

std::optional<SessionData> SessionData::fromJSON(std::string_view Json)
{
  auto Parsed = nlohmann::json::parse(Json);

  SessionData Object;
  Object.Id        = Parsed["ID"].get<size_t>();
  Object.UserId    = Parsed["UserID"].get<size_t>();
  Object.ExpiresAt = Parsed["ExpiresAt"].get<size_t>();
  Object.AuthToken = Parsed["AuthToken"].get<std::string>();

  return Object;
}

std::string SessionData::toJson() const
{
  nlohmann::json Json;

  Json["ID"]        = Id;
  Json["UserID"]    = UserId;
  Json["ExpiresAt"] = ExpiresAt;
  Json["AuthToken"] = AuthToken;

  return Json.dump();
}

std::optional<AuthToken> AuthToken::fromString(const std::string & Token)
{
  std::string Decoded;
  macaron::Base64::Decode(Token, Decoded);

  if (Decoded.size() != (sizeof(size_t) * 2 + 1) * 8)
    return std::nullopt;

  AuthToken Obj{};

  std::memcpy((void *)&Obj.UserId, Decoded.data(), sizeof(size_t));
  std::memcpy((void *)&Obj.ExpiresAt, Decoded.data() + sizeof(size_t), sizeof(size_t));
  std::memcpy((void *)&Obj.AccessModifier, Decoded.data() + sizeof(size_t) * 2, sizeof(uint8_t));

  return Obj;
}

std::string AuthToken::toString() const
{
  char token[(sizeof(size_t) * 2 + 1) * 8]{};

  std::memcpy(token, (void *)&UserId, sizeof(size_t));
  std::memcpy(token + sizeof(size_t), (void *)&ExpiresAt, sizeof(size_t));
  std::memcpy(token + sizeof(size_t) * 2, (void *)&AccessModifier, sizeof(uint8_t));

  return macaron::Base64::Encode(std::string{token, std::size(token)});
}
