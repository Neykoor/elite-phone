#include "phone_number_util.h"
#include "metadata_registry.h"
#include "phone_number_formatter.h"

#include <cctype>
#include <optional>
#include <regex>
#include <utility>
#include <vector>

namespace elitephone {

namespace {

std::string keepDigitsAndLeadingPlus(const std::string& input) {
  std::string result;
  for (char c : input) {
    if (c == '+' && result.empty()) {
      result.push_back(c);
    } else if (std::isdigit(static_cast<unsigned char>(c))) {
      result.push_back(c);
    }
  }
  return result;
}

bool fullMatch(const std::string& pattern, const std::string& value) {
  if (pattern.empty() || value.empty()) return false;
  try {
    std::regex re("^(?:" + pattern + ")$");
    return std::regex_match(value, re);
  } catch (const std::regex_error&) {
    return false;
  }
}

bool lengthIsPossible(const std::vector<PossibleLength>& lengths, size_t length) {
  if (lengths.empty()) return true;
  for (const auto& range : lengths) {
    if (static_cast<int>(length) >= range.min && static_cast<int>(length) <= range.max) {
      return true;
    }
  }
  return false;
}

std::string applyTransformRule(const std::string& rule, const std::smatch& match) {
  std::string result;
  for (size_t i = 0; i < rule.size(); ++i) {
    if (rule[i] == '$' && i + 1 < rule.size() && std::isdigit(static_cast<unsigned char>(rule[i + 1]))) {
      size_t groupIndex = static_cast<size_t>(rule[i + 1] - '0');
      if (groupIndex < match.size() && match[groupIndex].matched) {
        result += match[groupIndex].str();
      }
      ++i;
    } else {
      result.push_back(rule[i]);
    }
  }
  return result;
}

std::string stripNationalPrefix(const std::string& nationalCandidate, const TerritoryMetadata& territory) {
  std::string prefixPattern = !territory.nationalPrefixForParsing.empty()
      ? territory.nationalPrefixForParsing
      : territory.nationalPrefix;
  if (prefixPattern.empty()) {
    return nationalCandidate;
  }

  std::regex prefixRegex;
  try {
    prefixRegex = std::regex("^(?:" + prefixPattern + ")");
  } catch (const std::regex_error&) {
    return nationalCandidate;
  }

  std::smatch match;
  if (!std::regex_search(nationalCandidate, match, prefixRegex)) {
    return nationalCandidate;
  }

  std::string matchedSpan = match[0].str();
  std::string remainder = nationalCandidate.substr(matchedSpan.size());

  std::string stripped;
  if (!territory.nationalPrefixTransformRule.empty() && match.size() > 1 && match[1].matched) {
    stripped = applyTransformRule(territory.nationalPrefixTransformRule, match) + remainder;
  } else {
    stripped = remainder;
  }

  bool strippedValid = fullMatch(territory.generalDescPattern, stripped);
  bool originalValid = fullMatch(territory.generalDescPattern, nationalCandidate);
  if (originalValid && !strippedValid) {
    return nationalCandidate;
  }
  return stripped;
}

using MemberDesc = std::optional<NumberTypeDesc> TerritoryMetadata::*;

struct CategoryEntry {
  NumberCategory category;
  MemberDesc member;
};

const std::vector<CategoryEntry>& categoryTable() {
  static const std::vector<CategoryEntry> table = {
      {NumberCategory::Pager, &TerritoryMetadata::pager},
      {NumberCategory::TollFree, &TerritoryMetadata::tollFree},
      {NumberCategory::PremiumRate, &TerritoryMetadata::premiumRate},
      {NumberCategory::SharedCost, &TerritoryMetadata::sharedCost},
      {NumberCategory::PersonalNumber, &TerritoryMetadata::personalNumber},
      {NumberCategory::Voip, &TerritoryMetadata::voip},
      {NumberCategory::Uan, &TerritoryMetadata::uan},
      {NumberCategory::Voicemail, &TerritoryMetadata::voicemail},
  };
  return table;
}

struct ClassifiedMatch {
  NumberCategory category = NumberCategory::Unknown;
  const NumberTypeDesc* desc = nullptr;
};

ClassifiedMatch classify(const std::string& nationalNumber, const TerritoryMetadata& territory) {
  bool fixedLineMatch = territory.fixedLine.has_value() &&
      fullMatch(territory.fixedLine->nationalNumberPattern, nationalNumber);
  bool mobileMatch = territory.mobile.has_value() &&
      fullMatch(territory.mobile->nationalNumberPattern, nationalNumber);

  if (fixedLineMatch && mobileMatch) {
    return {NumberCategory::FixedLineOrMobile, &territory.fixedLine.value()};
  }
  if (fixedLineMatch) {
    return {NumberCategory::FixedLine, &territory.fixedLine.value()};
  }
  if (mobileMatch) {
    return {NumberCategory::Mobile, &territory.mobile.value()};
  }

  for (const auto& entry : categoryTable()) {
    const auto& desc = territory.*entry.member;
    if (desc.has_value() && fullMatch(desc->nationalNumberPattern, nationalNumber)) {
      return {entry.category, &desc.value()};
    }
  }
  return {};
}

}

ParseResult parsePhoneNumber(const std::string& rawNumber, const std::string& defaultRegionCode) {
  ParseResult result;

  std::string cleaned = keepDigitsAndLeadingPlus(rawNumber);
  if (cleaned.empty() || cleaned == "+") {
    result.error = "empty_number";
    return result;
  }

  const TerritoryMetadata* territory = nullptr;
  std::string nationalCandidate;

  if (cleaned.front() == '+') {
    std::string afterPlus = cleaned.substr(1);
    for (int len = 1; len <= 3 && territory == nullptr; ++len) {
      if (afterPlus.size() < static_cast<size_t>(len)) break;
      int candidateCode = std::stoi(afterPlus.substr(0, static_cast<size_t>(len)));
      std::string rest = afterPlus.substr(static_cast<size_t>(len));
      const TerritoryMetadata* chosen =
          MetadataRegistry::instance().chooseTerritoryForCountryCode(candidateCode, rest);
      if (chosen != nullptr) {
        territory = chosen;
        result.countryCode = candidateCode;
        nationalCandidate = rest;
      }
    }
    if (territory == nullptr) {
      result.error = "unknown_country_code";
      return result;
    }
  } else {
    if (defaultRegionCode.empty()) {
      result.error = "region_code_required";
      return result;
    }
    territory = MetadataRegistry::instance().findByRegionCode(defaultRegionCode);
    if (territory == nullptr) {
      result.error = "unknown_region_code";
      return result;
    }
    result.countryCode = territory->countryCode;
    nationalCandidate = cleaned;
  }

  std::string nationalNumber = stripNationalPrefix(nationalCandidate, *territory);
  result.regionCode = territory->id;
  result.nationalNumber = nationalNumber;

  ClassifiedMatch matched = classify(nationalNumber, *territory);
  result.category = matched.category;

  bool generalValid = fullMatch(territory->generalDescPattern, nationalNumber);

  if (matched.desc != nullptr) {
    result.possible = lengthIsPossible(matched.desc->possibleLengths, nationalNumber.size());
  } else {
    result.possible = generalValid && nationalNumber.size() >= 2 && nationalNumber.size() <= 15;
  }

  result.valid = generalValid && matched.category != NumberCategory::Unknown && result.possible;
  if (result.valid) {
    result.e164 = formatPhoneNumber(nationalNumber, result.countryCode, *territory, NumberFormatStyle::E164);
    result.international = formatPhoneNumber(nationalNumber, result.countryCode, *territory, NumberFormatStyle::International);
    result.national = formatPhoneNumber(nationalNumber, result.countryCode, *territory, NumberFormatStyle::National);
    result.rfc3966 = formatPhoneNumber(nationalNumber, result.countryCode, *territory, NumberFormatStyle::Rfc3966);
    result.significant = formatPhoneNumber(nationalNumber, result.countryCode, *territory, NumberFormatStyle::Significant);
  }
  if (!result.valid && result.error.empty()) {
    result.error = "no_match";
  } else {
    result.error.clear();
  }

  return result;
}

}
