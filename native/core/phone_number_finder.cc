#include "phone_number_finder.h"

#include <cctype>

namespace elitephone {

namespace {

const int kMinCandidateDigits = 2;
const int kMaxCandidateDigits = 17;

bool isDigitChar(char c) {
  return c >= '0' && c <= '9';
}

bool isCandidateChar(char c) {
  return isDigitChar(c) || c == '+' || c == '-' || c == '.' || c == '(' || c == ')' || c == ' ' || c == '\t';
}

bool isBoundaryBlocking(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) != 0;
}

std::string trimCandidate(const std::string& raw, size_t* trimmedStart) {
  size_t begin = 0;
  size_t end = raw.size();
  while (begin < end && !(isDigitChar(raw[begin]) || raw[begin] == '+')) {
    ++begin;
  }
  while (end > begin && !isDigitChar(raw[end - 1])) {
    --end;
  }
  *trimmedStart = begin;
  return raw.substr(begin, end - begin);
}

int countDigits(const std::string& value) {
  int count = 0;
  for (char c : value) {
    if (isDigitChar(c)) ++count;
  }
  return count;
}

}

std::vector<PhoneNumberMatch> findPhoneNumbers(
    const std::string& text,
    const std::string& defaultRegionCode,
    MatchLeniency leniency,
    int maxTries) {
  std::vector<PhoneNumberMatch> results;
  size_t i = 0;
  int triesLeft = maxTries;

  while (i < text.size() && triesLeft != 0) {
    if (!(isDigitChar(text[i]) || text[i] == '+')) {
      ++i;
      continue;
    }
    if (i > 0 && isBoundaryBlocking(text[i - 1])) {
      ++i;
      continue;
    }

    size_t j = i;
    while (j < text.size() && isCandidateChar(text[j])) {
      ++j;
    }

    std::string rawCandidate = text.substr(i, j - i);
    size_t trimmedOffset = 0;
    std::string candidate = trimCandidate(rawCandidate, &trimmedOffset);

    size_t candidateStart = i + trimmedOffset;
    size_t candidateEnd = candidateStart + candidate.size();
    bool boundaryOk = candidateEnd >= text.size() || !isBoundaryBlocking(text[candidateEnd]);

    if (!candidate.empty() && boundaryOk) {
      int digitCount = countDigits(candidate);
      if (digitCount >= kMinCandidateDigits && digitCount <= kMaxCandidateDigits) {
        if (triesLeft > 0) --triesLeft;
        ParseResult parsed = parsePhoneNumber(candidate, defaultRegionCode);
        bool accept = leniency == MatchLeniency::Possible ? parsed.possible : parsed.valid;
        if (accept) {
          PhoneNumberMatch match;
          match.text = candidate;
          match.start = candidateStart;
          match.end = candidateEnd;
          match.phoneNumber = parsed;
          results.push_back(std::move(match));
        }
      }
    }

    i = j > i ? j : i + 1;
  }

  return results;
}

}
