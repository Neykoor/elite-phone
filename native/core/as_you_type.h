#pragma once

#include "types.h"

#include <string>

namespace elitephone {

class AsYouType {
 public:
  explicit AsYouType(std::string defaultRegionCode);

  std::string inputDigit(const std::string& input);
  void reset();
  std::string currentOutput() const;

 private:
  std::string defaultRegionCode_;
  bool international_ = false;
  bool territoryResolved_ = false;
  bool sawExplicitPrefix_ = false;
  int countryCode_ = 0;
  std::string countryCodeDigits_;
  std::string nationalDigits_;
  const TerritoryMetadata* territory_ = nullptr;

  bool tryResolveCountryCode();
  std::string render() const;
};

}
