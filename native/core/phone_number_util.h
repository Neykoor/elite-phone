#pragma once

#include "types.h"

#include <string>

namespace elitephone {

enum class NumberCategory {
  Unknown,
  FixedLine,
  Mobile,
  FixedLineOrMobile,
  Pager,
  TollFree,
  PremiumRate,
  SharedCost,
  PersonalNumber,
  Voip,
  Uan,
  Voicemail,
};

struct ParseResult {
  bool valid = false;
  bool possible = false;
  std::string regionCode;
  int countryCode = 0;
  std::string nationalNumber;
  std::string e164;
  std::string international;
  std::string national;
  std::string rfc3966;
  std::string significant;
  NumberCategory category = NumberCategory::Unknown;
  std::string error;
};

ParseResult parsePhoneNumber(const std::string& rawNumber, const std::string& defaultRegionCode);

}
