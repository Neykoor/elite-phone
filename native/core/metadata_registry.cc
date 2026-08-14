#include "metadata_registry.h"

#include <regex>

namespace elitephone {

MetadataRegistry& MetadataRegistry::instance() {
  static MetadataRegistry registry;
  return registry;
}

void MetadataRegistry::registerTerritory(TerritoryMetadata territory) {
  size_t index = territories_.size();
  byRegionCode_[territory.id] = index;
  byCountryCode_[territory.countryCode].push_back(index);
  territories_.push_back(std::move(territory));
}

void MetadataRegistry::clear() {
  territories_.clear();
  byRegionCode_.clear();
  byCountryCode_.clear();
}

const TerritoryMetadata* MetadataRegistry::findByRegionCode(const std::string& regionCode) const {
  auto it = byRegionCode_.find(regionCode);
  if (it == byRegionCode_.end()) return nullptr;
  return &territories_[it->second];
}

std::vector<const TerritoryMetadata*> MetadataRegistry::findByCountryCode(int countryCode) const {
  std::vector<const TerritoryMetadata*> result;
  auto it = byCountryCode_.find(countryCode);
  if (it == byCountryCode_.end()) return result;
  result.reserve(it->second.size());
  for (size_t index : it->second) {
    result.push_back(&territories_[index]);
  }
  return result;
}

const TerritoryMetadata* MetadataRegistry::mainTerritoryForCountryCode(int countryCode) const {
  auto it = byCountryCode_.find(countryCode);
  if (it == byCountryCode_.end()) return nullptr;
  for (size_t index : it->second) {
    if (territories_[index].mainCountryForCode) return &territories_[index];
  }
  if (it->second.empty()) return nullptr;
  return &territories_[it->second.front()];
}

std::vector<std::string> MetadataRegistry::allRegionCodes() const {
  std::vector<std::string> result;
  result.reserve(territories_.size());
  for (const auto& territory : territories_) {
    result.push_back(territory.id);
  }
  return result;
}

const TerritoryMetadata* MetadataRegistry::chooseTerritoryForCountryCode(
    int countryCode, const std::string& remainingDigits) const {
  auto it = byCountryCode_.find(countryCode);
  if (it == byCountryCode_.end() || it->second.empty()) return nullptr;
  if (it->second.size() == 1) return &territories_[it->second.front()];

  for (size_t index : it->second) {
    const TerritoryMetadata& candidate = territories_[index];
    if (candidate.leadingDigits.empty()) continue;
    try {
      std::regex leadingRegex("^(?:" + candidate.leadingDigits + ")");
      if (std::regex_search(remainingDigits, leadingRegex)) {
        return &candidate;
      }
    } catch (const std::regex_error&) {
    }
  }
  for (size_t index : it->second) {
    if (territories_[index].mainCountryForCode) return &territories_[index];
  }
  return &territories_[it->second.front()];
}

}
