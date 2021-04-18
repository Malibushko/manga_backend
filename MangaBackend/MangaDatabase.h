#pragma once
#include <sqlite_orm/sqlite_orm.h>
#include "Database.h"
#include "Structs.h"

namespace details
{
inline auto CreateStorage(std::string_view _Database)
{
  using namespace sqlite_orm;
  return make_storage(
    _Database.data(),
    make_table("Users", make_column("ID", &User::Id, autoincrement(), primary_key()),
               make_column("Login", &User::Login), make_column("PasswordHash", &User::PasswordHash),
               make_column("UserType", &User::UserType)),
    make_table("Manga", make_column("ID", &Manga::Id, autoincrement(), primary_key()),
               make_column("DataBlobID", &Manga::DataBlobId), make_column("UploadDate", &Manga::UploadDate),
               make_column("UploadUserID", &Manga::UploadUserId),
               make_column("LastChangeDate", &Manga::LastChangeDate, default_value(0)),
               make_column("AccessModifier", &Manga::AccessModifier),
               foreign_key(&Manga::DataBlobId).references(&MangaBlob::Id),
               foreign_key(&Manga::UploadUserId).references(&User::Id)),
    make_table("MangaBlob", make_column("ID", &MangaBlob::Id, autoincrement(), primary_key()),
               make_column("FileName", &MangaBlob::FileName), make_column("Data", &MangaBlob::Data)),
    make_table("Sessions", make_column("ID", &SessionData::Id, autoincrement(), primary_key()),
               make_column("UserId", &SessionData::UserId), make_column("ExpiresAt", &SessionData::ExpiresAt),
               make_column("AuthToken", &SessionData::AuthToken),
               foreign_key(&SessionData::UserId).references(&User::Id)));
}
} // namespace details

class MangaDatabase : public IDatabase
{
public: // Using declarations
  using storage_t = std::invoke_result_t<decltype(&details::CreateStorage), std::string_view>;

public: // Interface
  MangaDatabase(const MangaDatabase &) = delete;
  MangaDatabase(MangaDatabase &&)      = delete;
  MangaDatabase & operator=(const MangaDatabase &) = delete;
  MangaDatabase & operator=(MangaDatabase &&) = delete;

  static MangaDatabase & Instance();

  bool Open(std::string_view _Database) override;

  std::shared_ptr<storage_t> & GetStorage();

protected: // Service
  MangaDatabase();
  ~MangaDatabase() override;

private:
  std::shared_ptr<storage_t> m_Storage;
};
