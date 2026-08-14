#pragma once

#include "phone_number_util.h"

#include <string>
#include <vector>

namespace elitephone {

enum class MatchLeniency {
  Possible,
  Valid,
};

struct PhoneNumberMatch {
  std::string text;
  size_t start;
  size_t end;
  ParseResult phoneNumber;
};

std::vector<PhoneNumberMatch> findPhoneNumbers(
    const std::string& text,
    const std::string& defaultRegionCode,
    MatchLeniency leniency,
    int maxTries);

}
