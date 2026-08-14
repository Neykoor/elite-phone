#pragma once

#include <optional>
#include <string>
#include <vector>

namespace elitephone {

struct PossibleLength {
  int min;
  int max;
};

struct NumberTypeDesc {
  std::vector<PossibleLength> possibleLengths;
  std::vector<PossibleLength> possibleLengthsLocalOnly;
  std::string exampleNumber;
  std::string nationalNumberPattern;
};

struct NumberFormatRule {
  std::string pattern;
  std::vector<std::string> leadingDigits;
  std::string format;
  std::string intlFormat;
  std::string nationalPrefixFormattingRule;
  bool nationalPrefixOptionalWhenFormatting = false;
  std::string carrierCodeFormattingRule;
};

struct TerritoryMetadata {
  std::string id;
  int countryCode = 0;
  bool mainCountryForCode = false;
  std::string leadingDigits;
  std::string preferredInternationalPrefix;
  std::string internationalPrefix;
  std::string nationalPrefix;
  std::string nationalPrefixForParsing;
  std::string nationalPrefixTransformRule;
  std::string nationalPrefixFormattingRule;
  bool nationalPrefixOptionalWhenFormatting = false;
  std::string carrierCodeFormattingRule;
  bool mobileNumberPortableRegion = false;
  std::string generalDescPattern;
  std::vector<NumberFormatRule> availableFormats;
  std::optional<NumberTypeDesc> noInternationalDialling;
  std::optional<NumberTypeDesc> fixedLine;
  std::optional<NumberTypeDesc> mobile;
  std::optional<NumberTypeDesc> pager;
  std::optional<NumberTypeDesc> tollFree;
  std::optional<NumberTypeDesc> premiumRate;
  std::optional<NumberTypeDesc> sharedCost;
  std::optional<NumberTypeDesc> personalNumber;
  std::optional<NumberTypeDesc> voip;
  std::optional<NumberTypeDesc> uan;
  std::optional<NumberTypeDesc> voicemail;
};

}
