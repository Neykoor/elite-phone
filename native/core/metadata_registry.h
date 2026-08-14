#pragma once

#include "types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace elitephone {

class MetadataRegistry {
 public:
  static MetadataRegistry& instance();

  void registerTerritory(TerritoryMetadata territory);
  void clear();

  const TerritoryMetadata* findByRegionCode(const std::string& regionCode) const;
  std::vector<const TerritoryMetadata*> findByCountryCode(int countryCode) const;
  const TerritoryMetadata* mainTerritoryForCountryCode(int countryCode) const;
  const TerritoryMetadata* chooseTerritoryForCountryCode(int countryCode, const std::string& remainingDigits) const;
  std::vector<std::string> allRegionCodes() const;

 private:
  std::vector<TerritoryMetadata> territories_;
  std::unordered_map<std::string, size_t> byRegionCode_;
  std::unordered_map<int, std::vector<size_t>> byCountryCode_;
};

}
