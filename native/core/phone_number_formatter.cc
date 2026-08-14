#include "phone_number_formatter.h"
#include "format_expansion.h"

#include <regex>

namespace elitephone {

namespace {

const NumberFormatRule* chooseFormatRule(
    const std::string& nationalNumber, const TerritoryMetadata& territory, bool forInternational) {
  for (const auto& rule : territory.availableFormats) {
    if (forInternational && rule.intlFormat == "NA") continue;

    if (!rule.leadingDigits.empty()) {
      const std::string& lastLeading = rule.leadingDigits.back();
      try {
        std::regex leadingRegex("^(?:" + lastLeading + ")");
        if (!std::regex_search(nationalNumber, leadingRegex)) continue;
      } catch (const std::regex_error&) {
        continue;
      }
    }

    try {
      std::regex patternRegex("^(?:" + rule.pattern + ")$");
      if (std::regex_match(nationalNumber, patternRegex)) {
        return &rule;
      }
    } catch (const std::regex_error&) {
      continue;
    }
  }
  return nullptr;
}

std::string applyFormat(const std::string& nationalNumber, const NumberFormatRule& rule, const std::string& composedTemplate) {
  try {
    std::regex patternRegex("^(?:" + rule.pattern + ")$");
    return std::regex_replace(nationalNumber, patternRegex, composedTemplate);
  } catch (const std::regex_error&) {
    return nationalNumber;
  }
}

std::string formatNational(const std::string& nationalNumber, const TerritoryMetadata& territory) {
  const NumberFormatRule* rule = chooseFormatRule(nationalNumber, territory, false);
  if (rule == nullptr) return nationalNumber;

  std::string prefixRule = !rule->nationalPrefixFormattingRule.empty()
      ? rule->nationalPrefixFormattingRule
      : territory.nationalPrefixFormattingRule;

  std::string composedTemplate = rule->format;
  if (!prefixRule.empty() && !territory.nationalPrefix.empty()) {
    std::string expanded = expandPrefixRule(prefixRule, territory.nationalPrefix);
    composedTemplate = substituteFirstPlaceholder(rule->format, expanded);
  }
  return applyFormat(nationalNumber, *rule, composedTemplate);
}

std::string formatInternationalBody(const std::string& nationalNumber, const TerritoryMetadata& territory) {
  const NumberFormatRule* rule = chooseFormatRule(nationalNumber, territory, true);
  if (rule == nullptr) return nationalNumber;
  const std::string& templateStr = !rule->intlFormat.empty() ? rule->intlFormat : rule->format;
  return applyFormat(nationalNumber, *rule, templateStr);
}

std::string toRfc3966Separators(const std::string& formatted) {
  std::string result;
  for (char c : formatted) {
    if (c == ' ' || c == '.') {
      result.push_back('-');
    } else if (c == '(' || c == ')') {
      continue;
    } else {
      result.push_back(c);
    }
  }
  return result;
}

}

std::string formatPhoneNumber(
    const std::string& nationalNumber,
    int countryCode,
    const TerritoryMetadata& territory,
    NumberFormatStyle style) {
  switch (style) {
    case NumberFormatStyle::E164:
      return "+" + std::to_string(countryCode) + nationalNumber;
    case NumberFormatStyle::National:
      return formatNational(nationalNumber, territory);
    case NumberFormatStyle::International:
      return "+" + std::to_string(countryCode) + " " + formatInternationalBody(nationalNumber, territory);
    case NumberFormatStyle::Rfc3966:
      return "tel:+" + std::to_string(countryCode) + "-" + toRfc3966Separators(formatInternationalBody(nationalNumber, territory));
    case NumberFormatStyle::Significant:
      return nationalNumber;
  }
  return nationalNumber;
}

}
