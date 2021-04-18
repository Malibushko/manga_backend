#include "MangaDatabase.h"
#include <spdlog/spdlog.h>

bool MangaDatabase::Open(std::string_view _Database)
{
  m_Storage = std::make_shared<storage_t>(details::CreateStorage(_Database));

  auto Schema = m_Storage->sync_schema();

#ifndef NDEBUG
  spdlog::info("Database tables count: {}", Schema.size());
  for (const auto & it : Schema)
  {
    spdlog::info("Table {}", it.first);
  }
#endif

  return Schema.size();
}

std::shared_ptr<MangaDatabase::storage_t> & MangaDatabase::GetStorage()
{
  return m_Storage;
}

MangaDatabase::MangaDatabase() : m_Storage{nullptr}
{
}

MangaDatabase & MangaDatabase::Instance()
{
  static MangaDatabase Database;
  return Database;
}

MangaDatabase::~MangaDatabase()
{
}
