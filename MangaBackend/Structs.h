#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

enum USER_TYPE : uint8_t
{
  ANONYMOUS = 1 << 0,
  REGULAR   = 1 << 1,
  ADMIN     = 1 << 2,
};

struct User
{
  size_t      Id;
  size_t      UserType;
  std::string Login;
  std::string PasswordHash;

  static std::optional<User> fromJSON(std::string_view Json);
  std::string                toJson() const;
};

struct Manga
{
  size_t  Id;
  size_t  DataBlobId;
  int64_t UploadDate;
  int64_t LastChangeDate;
  size_t  UploadUserId;
  size_t  AccessModifier;

  static std::optional<Manga> fromJSON(std::string_view Json);
  std::string                 toJson() const;
};

struct MangaBlob
{
  size_t            Id;
  std::string       FileName;
  std::vector<char> Data;

  static std::optional<MangaBlob> fromJSON(std::string_view Json);
  std::string                     toJson() const;
};
struct AuthToken
{
  size_t  UserId;
  int64_t ExpiresAt;
  uint8_t AccessModifier;

  static std::optional<AuthToken> fromString(const std::string & Token);
  std::string                     toString() const;
};

struct SessionData
{
  size_t      Id;
  size_t      UserId;
  size_t      ExpiresAt;
  std::string AuthToken;

  std::string SetToken(uint8_t AccessModifier);

  static std::optional<SessionData> fromJSON(std::string_view Json);
  std::string                       toJson() const;
};

struct ResponseJson
{
  size_t      Error;
  std::string Message;

  std::string toJson() const;
};
