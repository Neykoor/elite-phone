#include "as_you_type.h"
#include "format_expansion.h"
#include "metadata_registry.h"

#include <algorithm>
#include <regex>
#include <vector>

namespace elitephone {

namespace {

int computeGroupLength(const std::string& content) {
  int total = 0;
  size_t i = 0;
  while (i < content.size()) {
    if (content.compare(i, 2, "\\d") == 0) {
      i += 2;
      if (i < content.size() && content[i] == '{') {
        size_t close = content.find('}', i);
        if (close != std::string::npos) {
          std::string inside = content.substr(i + 1, close - i - 1);
          size_t comma = inside.find(',');
          std::string lastNumber = comma == std::string::npos ? inside : inside.substr(comma + 1);
          std::string firstNumber = comma == std::string::npos ? inside : inside.substr(0, comma);
          std::string chosen = !lastNumber.empty() ? lastNumber : firstNumber;
          total += chosen.empty() ? 1 : std::stoi(chosen);
          i = close + 1;
          continue;
        }
      }
      total += 1;
      continue;
    }
    ++i;
  }
  return total > 0 ? total : 1;
}

std::vector<int> extractGroupLengths(const std::string& pattern) {
  std::vector<int> lengths;
  int depth = 0;
  std::string currentGroup;
  bool inGroup = false;
  for (char c : pattern) {
    if (c == '(') {
      if (depth == 0) {
        inGroup = true;
        currentGroup.clear();
      } else if (inGroup) {
        currentGroup.push_back(c);
      }
      ++depth;
    } else if (c == ')') {
      --depth;
      if (depth == 0 && inGroup) {
        lengths.push_back(computeGroupLength(currentGroup));
        inGroup = false;
      } else if (inGroup) {
        currentGroup.push_back(c);
      }
    } else if (inGroup) {
      currentGroup.push_back(c);
    }
  }
  return lengths;
}

const NumberFormatRule* chooseLiveFormatRule(const std::string& digitsSoFar, const TerritoryMetadata& territory) {
  for (const auto& candidate : territory.availableFormats) {
    if (candidate.leadingDigits.empty()) return &candidate;
    const std::string& lastLeading = candidate.leadingDigits.back();
    try {
      std::regex leadingRegex("^(?:" + lastLeading + ")");
      if (std::regex_search(digitsSoFar, leadingRegex)) return &candidate;
    } catch (const std::regex_error&) {
      continue;
    }
  }
  return nullptr;
}

}

AsYouType::AsYouType(std::string defaultRegionCode) : defaultRegionCode_(std::move(defaultRegionCode)) {}

void AsYouType::reset() {
  international_ = false;
  territoryResolved_ = false;
  sawExplicitPrefix_ = false;
  countryCode_ = 0;
  countryCodeDigits_.clear();
  nationalDigits_.clear();
  territory_ = nullptr;
}

bool AsYouType::tryResolveCountryCode() {
  for (size_t len = 1; len <= 3 && len <= countryCodeDigits_.size(); ++len) {
    int candidate = std::stoi(countryCodeDigits_.substr(0, len));
    auto matches = MetadataRegistry::instance().findByCountryCode(candidate);
    if (!matches.empty()) {
      territory_ = MetadataRegistry::instance().mainTerritoryForCountryCode(candidate);
      if (territory_ == nullptr) territory_ = matches.front();
      countryCode_ = candidate;
      territoryResolved_ = true;
      nationalDigits_ = countryCodeDigits_.substr(len);
      return true;
    }
  }
  return false;
}

std::string AsYouType::inputDigit(const std::string& input) {
  for (char c : input) {
    if (c == '+' && nationalDigits_.empty() && countryCodeDigits_.empty() && !territoryResolved_) {
      international_ = true;
      continue;
    }
    if (c < '0' || c > '9') continue;

    if (international_ && !territoryResolved_) {
      countryCodeDigits_.push_back(c);
      tryResolveCountryCode();
      continue;
    }

    if (!territoryResolved_ && !international_) {
      territory_ = MetadataRegistry::instance().findByRegionCode(defaultRegionCode_);
      if (territory_ != nullptr) {
        countryCode_ = territory_->countryCode;
      }
      territoryResolved_ = true;
    }

    if (!international_ && !sawExplicitPrefix_ && nationalDigits_.empty() && territory_ != nullptr &&
        territory_->nationalPrefix.size() == 1 && territory_->nationalPrefix[0] == c) {
      sawExplicitPrefix_ = true;
      continue;
    }

    nationalDigits_.push_back(c);
  }
  return render();
}

std::string AsYouType::render() const {
  if (territory_ == nullptr) {
    if (international_) return "+" + countryCodeDigits_;
    return nationalDigits_;
  }

  const NumberFormatRule* rule = chooseLiveFormatRule(nationalDigits_, *territory_);
  if (rule == nullptr) {
    std::string prefix = international_ ? ("+" + std::to_string(countryCode_) + " ") : "";
    return prefix + nationalDigits_;
  }

  std::string composedTemplate;
  if (international_) {
    composedTemplate = !rule->intlFormat.empty() ? rule->intlFormat : rule->format;
  } else {
    composedTemplate = rule->format;
    std::string prefixRule = !rule->nationalPrefixFormattingRule.empty()
        ? rule->nationalPrefixFormattingRule
        : territory_->nationalPrefixFormattingRule;
    if (!prefixRule.empty() && !territory_->nationalPrefix.empty()) {
      std::string expanded = expandPrefixRule(prefixRule, territory_->nationalPrefix);
      composedTemplate = substituteFirstPlaceholder(rule->format, expanded);
    }
  }

  std::vector<int> groupLengths = extractGroupLengths(rule->pattern);
  std::string remaining = nationalDigits_;
  std::vector<std::string> groups;
  for (int length : groupLengths) {
    size_t take = std::min(static_cast<size_t>(length), remaining.size());
    groups.push_back(remaining.substr(0, take));
    remaining = remaining.substr(take);
  }
  if (!remaining.empty() && !groups.empty()) {
    groups.back() += remaining;
  }

  std::string filled = composedTemplate;
  for (size_t i = 0; i < groups.size(); ++i) {
    std::string token = "$" + std::to_string(i + 1);
    size_t pos = filled.find(token);
    if (pos != std::string::npos) {
      filled = filled.substr(0, pos) + groups[i] + filled.substr(pos + token.size());
    }
  }
  for (size_t i = groups.size() + 1; i <= groupLengths.size(); ++i) {
    std::string token = "$" + std::to_string(i);
    size_t pos = filled.find(token);
    if (pos != std::string::npos) {
      filled = filled.substr(0, pos) + filled.substr(pos + token.size());
    }
  }

  size_t lastDigitPos = std::string::npos;
  for (size_t i = 0; i < filled.size(); ++i) {
    if (filled[i] >= '0' && filled[i] <= '9') lastDigitPos = i;
  }
  std::string body = lastDigitPos == std::string::npos ? "" : filled.substr(0, lastDigitPos + 1);

  if (international_) {
    return "+" + std::to_string(countryCode_) + " " + body;
  }
  return body;
}

std::string AsYouType::currentOutput() const {
  return render();
}

}
