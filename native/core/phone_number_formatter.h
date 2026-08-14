#pragma once

#include "types.h"

#include <string>

namespace elitephone {

enum class NumberFormatStyle {
  E164,
  International,
  National,
  Rfc3966,
  Significant,
};

std::string formatPhoneNumber(
    const std::string& nationalNumber,
    int countryCode,
    const TerritoryMetadata& territory,
    NumberFormatStyle style);

}
