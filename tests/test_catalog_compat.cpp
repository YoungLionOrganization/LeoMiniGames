#include "LeoMiniGamesDeveloperPlatform/CatalogCompat.h"
#include <QJsonDocument>
#include <QDebug>
#define CHECK(condition) do { if (!(condition)) qFatal("FAIL: %s", #condition); } while (false)
using namespace LeoMiniGames::DeveloperPlatform;
int main(){
  const auto d=QJsonDocument::fromJson(R"({"data":[{"id":"demo","name":"Demo"}]})");
  const auto a=CatalogCompat::extractItems(d); CHECK(a.size()==1);
  const auto o=CatalogCompat::normalizeItem(a[0].toObject());
  CHECK(o.value("downloads").isDouble()); CHECK(o.value("size_bytes").isDouble());
  CHECK(o.value("icon").toString()=="DEM"); return 0;
}
