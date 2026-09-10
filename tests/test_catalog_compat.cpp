#include "LeoMiniGamesDeveloperPlatform/CatalogCompat.h"
#include <QJsonDocument>
#include <cassert>
using namespace LeoMiniGames::DeveloperPlatform;
int main(){
  const auto d=QJsonDocument::fromJson(R"({"data":[{"id":"demo","name":"Demo"}]})");
  const auto a=CatalogCompat::extractItems(d); assert(a.size()==1);
  const auto o=CatalogCompat::normalizeItem(a[0].toObject());
  assert(o.value("downloads").isDouble()); assert(o.value("size_bytes").isDouble());
  assert(o.value("icon").toString()=="DEM"); return 0;
}
