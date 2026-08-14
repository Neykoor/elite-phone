#pragma once

#include <string>

namespace elitephone {

std::string expandPrefixRule(const std::string& rule, const std::string& nationalPrefix);
std::string substituteFirstPlaceholder(const std::string& templateStr, const std::string& replacement);

}
