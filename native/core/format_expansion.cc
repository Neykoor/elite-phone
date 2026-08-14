#include "format_expansion.h"

namespace elitephone {

std::string expandPrefixRule(const std::string& rule, const std::string& nationalPrefix) {
  std::string result;
  for (size_t i = 0; i < rule.size();) {
    if (rule.compare(i, 3, "$NP") == 0) {
      result += nationalPrefix;
      i += 3;
    } else if (rule.compare(i, 3, "$FG") == 0) {
      result += "$1";
      i += 3;
    } else {
      result.push_back(rule[i]);
      ++i;
    }
  }
  return result;
}

std::string substituteFirstPlaceholder(const std::string& templateStr, const std::string& replacement) {
  size_t pos = templateStr.find("$1");
  if (pos == std::string::npos) return templateStr;
  return templateStr.substr(0, pos) + replacement + templateStr.substr(pos + 2);
}

}
