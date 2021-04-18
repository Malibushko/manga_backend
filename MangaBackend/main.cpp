#include "Server.h"
#include "MangaDatabase.h"
#include <spdlog/spdlog.h>
int main(int argc, char * argv[])
{
  Server s(80);

  if (MangaDatabase::Instance().Open("C:\\Users\\malib\\source\\repos\\MangaBackend\\db.sqlite"))
    s.Start();
  else
    return 1;
}
