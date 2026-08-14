#include "../as_you_type.h"
#include "../metadata_registry.h"
#include "../phone_number_finder.h"
#include "../phone_number_util.h"
#include "../types.h"

#include <iostream>
#include <string>

using namespace elitephone;

namespace {

int failures = 0;

void expectEqual(const std::string& label, const std::string& actual, const std::string& expected) {
  if (actual != expected) {
    ++failures;
    std::cout << "FAIL " << label << ": esperaba \"" << expected << "\", obtuve \"" << actual << "\"\n";
  } else {
    std::cout << "OK   " << label << "\n";
  }
}

void expectBool(const std::string& label, bool actual, bool expected) {
  expectEqual(label, actual ? "true" : "false", expected ? "true" : "false");
}

void expectCategory(const std::string& label, NumberCategory actual, NumberCategory expected) {
  expectEqual(label, std::to_string(static_cast<int>(actual)), std::to_string(static_cast<int>(expected)));
}

void registerAscensionIsland() {
  TerritoryMetadata territory;
  territory.id = "AC";
  territory.countryCode = 247;
  territory.internationalPrefix = "00";
  territory.generalDescPattern = "(?:[01589]\\d|[46])\\d{4}";

  NumberTypeDesc fixedLine;
  fixedLine.possibleLengths = {{5, 5}};
  fixedLine.exampleNumber = "62889";
  fixedLine.nationalNumberPattern = "6[2-467]\\d{3}";
  territory.fixedLine = fixedLine;

  NumberTypeDesc mobile;
  mobile.possibleLengths = {{5, 5}};
  mobile.exampleNumber = "40123";
  mobile.nationalNumberPattern = "4\\d{4}";
  territory.mobile = mobile;

  NumberTypeDesc uan;
  uan.possibleLengths = {{6, 6}};
  uan.exampleNumber = "542011";
  uan.nationalNumberPattern = "(?:0[1-9]|[1589]\\d)\\d{4}";
  territory.uan = uan;

  MetadataRegistry::instance().registerTerritory(std::move(territory));
}

void registerUnitedArabEmirates() {
  TerritoryMetadata territory;
  territory.id = "AE";
  territory.countryCode = 971;
  territory.mainCountryForCode = true;
  territory.internationalPrefix = "00";
  territory.nationalPrefix = "0";
  territory.generalDescPattern = "(?:[4-7]\\d|9[0-689])\\d{7}|800\\d{2,9}|[2-4679]\\d{7}";

  NumberTypeDesc fixedLine;
  fixedLine.possibleLengths = {{8, 8}};
  fixedLine.exampleNumber = "22345678";
  fixedLine.nationalNumberPattern = "[2-4679][2-8]\\d{6}";
  territory.fixedLine = fixedLine;

  NumberTypeDesc mobile;
  mobile.possibleLengths = {{9, 9}};
  mobile.exampleNumber = "501234567";
  mobile.nationalNumberPattern = "5[02-68]\\d{7}";
  territory.mobile = mobile;

  NumberTypeDesc tollFree;
  tollFree.possibleLengths = {{5, 12}};
  tollFree.exampleNumber = "800123456";
  tollFree.nationalNumberPattern = "400\\d{6}|800\\d{2,9}";
  territory.tollFree = tollFree;

  MetadataRegistry::instance().registerTerritory(std::move(territory));
}

void registerTransformRuleFixture() {
  TerritoryMetadata territory;
  territory.id = "XT";
  territory.countryCode = 999;
  territory.internationalPrefix = "00";
  territory.nationalPrefixForParsing = "0?(\\d{2})15";
  territory.nationalPrefixTransformRule = "9$1";
  territory.generalDescPattern = "9\\d{9}|\\d{10}";

  NumberTypeDesc mobile;
  mobile.possibleLengths = {{10, 10}};
  mobile.exampleNumber = "9111234567";
  mobile.nationalNumberPattern = "9\\d{9}";
  territory.mobile = mobile;

  NumberTypeDesc fixedLine;
  fixedLine.possibleLengths = {{10, 10}};
  fixedLine.exampleNumber = "1123456789";
  fixedLine.nationalNumberPattern = "[1-8]\\d{9}";
  territory.fixedLine = fixedLine;

  MetadataRegistry::instance().registerTerritory(std::move(territory));
}

void registerNanpFixture() {
  TerritoryMetadata us;
  us.id = "US";
  us.countryCode = 900;
  us.mainCountryForCode = true;
  us.internationalPrefix = "011";
  us.nationalPrefix = "1";
  us.generalDescPattern = "\\d{10}";
  NumberTypeDesc usFixed;
  usFixed.possibleLengths = {{10, 10}};
  usFixed.nationalNumberPattern = "2\\d{9}";
  us.fixedLine = usFixed;
  NumberTypeDesc usMobile;
  usMobile.possibleLengths = {{10, 10}};
  usMobile.nationalNumberPattern = "2\\d{9}";
  us.mobile = usMobile;
  MetadataRegistry::instance().registerTerritory(std::move(us));

  TerritoryMetadata bs;
  bs.id = "BS";
  bs.countryCode = 900;
  bs.leadingDigits = "242";
  bs.internationalPrefix = "011";
  bs.nationalPrefix = "1";
  bs.generalDescPattern = "\\d{10}";
  NumberTypeDesc bsFixed;
  bsFixed.possibleLengths = {{10, 10}};
  bsFixed.nationalNumberPattern = "242\\d{7}";
  bs.fixedLine = bsFixed;
  MetadataRegistry::instance().registerTerritory(std::move(bs));
}

void registerLiteralPrefixFormatFixture() {
  TerritoryMetadata territory;
  territory.id = "FM";
  territory.countryCode = 555;
  territory.internationalPrefix = "00";
  territory.nationalPrefix = "0";
  territory.nationalPrefixFormattingRule = "0$1";
  territory.generalDescPattern = "\\d{9}";

  NumberTypeDesc fixedLine;
  fixedLine.possibleLengths = {{9, 9}};
  fixedLine.exampleNumber = "123456789";
  fixedLine.nationalNumberPattern = "[1-9]\\d{8}";
  territory.fixedLine = fixedLine;

  NumberFormatRule rule;
  rule.pattern = "(\\d{3})(\\d{3})(\\d{3})";
  rule.format = "$1 $2 $3";
  territory.availableFormats = {rule};

  MetadataRegistry::instance().registerTerritory(std::move(territory));
}

void registerNpFgFormatFixture() {
  TerritoryMetadata territory;
  territory.id = "FN";
  territory.countryCode = 556;
  territory.internationalPrefix = "00";
  territory.nationalPrefix = "9";
  territory.nationalPrefixFormattingRule = "($NP$FG)";
  territory.generalDescPattern = "\\d{7}";

  NumberTypeDesc fixedLine;
  fixedLine.possibleLengths = {{7, 7}};
  fixedLine.exampleNumber = "1234567";
  fixedLine.nationalNumberPattern = "\\d{7}";
  territory.fixedLine = fixedLine;

  NumberFormatRule rule;
  rule.pattern = "(\\d{2})(\\d{5})";
  rule.format = "$1-$2";
  territory.availableFormats = {rule};

  MetadataRegistry::instance().registerTerritory(std::move(territory));
}

}

int main() {
  registerAscensionIsland();
  registerUnitedArabEmirates();
  registerTransformRuleFixture();
  registerNanpFixture();
  registerLiteralPrefixFormatFixture();
  registerNpFgFormatFixture();

  {
    ParseResult r = parsePhoneNumber("+24762889", "");
    expectEqual("AC intl regionCode", r.regionCode, "AC");
    expectEqual("AC intl nationalNumber", r.nationalNumber, "62889");
    expectCategory("AC intl category", r.category, NumberCategory::FixedLine);
    expectBool("AC intl valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("62889", "AC");
    expectCategory("AC national fixedLine category", r.category, NumberCategory::FixedLine);
    expectBool("AC national fixedLine valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("40123", "AC");
    expectCategory("AC national mobile category", r.category, NumberCategory::Mobile);
    expectBool("AC national mobile valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("542011", "AC");
    expectCategory("AC national uan category", r.category, NumberCategory::Uan);
    expectBool("AC national uan valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("99999", "AC");
    expectBool("AC invalid number", r.valid, false);
  }
  {
    ParseResult r = parsePhoneNumber("022345678", "AE");
    expectEqual("AE strips national prefix", r.nationalNumber, "22345678");
    expectCategory("AE fixedLine after strip", r.category, NumberCategory::FixedLine);
    expectBool("AE fixedLine valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("+97122345678", "");
    expectEqual("AE intl regionCode", r.regionCode, "AE");
    expectEqual("AE intl nationalNumber", r.nationalNumber, "22345678");
    expectBool("AE intl valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("501234567", "AE");
    expectCategory("AE mobile without prefix", r.category, NumberCategory::Mobile);
    expectBool("AE mobile valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("800123456", "AE");
    expectCategory("AE tollFree variable length", r.category, NumberCategory::TollFree);
    expectBool("AE tollFree valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("123", "AE");
    expectBool("AE too short invalid", r.valid, false);
  }
  {
    ParseResult r = parsePhoneNumber("011151234567", "XT");
    expectEqual("XT transform with leading 0", r.nationalNumber, "9111234567");
    expectCategory("XT transform category", r.category, NumberCategory::Mobile);
    expectBool("XT transform valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("11151234567", "XT");
    expectEqual("XT transform without leading 0", r.nationalNumber, "9111234567");
    expectBool("XT transform without leading 0 valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("1123456789", "XT");
    expectEqual("XT plain fixed line untouched", r.nationalNumber, "1123456789");
    expectCategory("XT plain fixed line category", r.category, NumberCategory::FixedLine);
    expectBool("XT plain fixed line valid", r.valid, true);
  }
  {
    ParseResult r = parsePhoneNumber("2123456789", "US");
    expectCategory("US fixedLineOrMobile", r.category, NumberCategory::FixedLineOrMobile);
    expectBool("US fixedLineOrMobile valid", r.valid, true);
    expectEqual("US e164", r.e164, "+9002123456789");
  }
  {
    ParseResult r = parsePhoneNumber("+9002423456789", "");
    expectEqual("NANP leadingDigits disambiguation", r.regionCode, "BS");
  }
  {
    ParseResult r = parsePhoneNumber("+9002123456789", "");
    expectEqual("NANP mainCountryForCode fallback", r.regionCode, "US");
  }
  {
    ParseResult r = parsePhoneNumber("123456789", "FM");
    expectBool("FM valid", r.valid, true);
    expectEqual("FM national con prefijo literal", r.national, "0123 456 789");
    expectEqual("FM international", r.international, "+555 123 456 789");
    expectEqual("FM e164", r.e164, "+555123456789");
    expectEqual("FM rfc3966", r.rfc3966, "tel:+555-123-456-789");
    expectEqual("FM significant", r.significant, "123456789");
  }
  {
    ParseResult r = parsePhoneNumber("1234567", "FN");
    expectBool("FN valid", r.valid, true);
    expectEqual("FN national con $NP/$FG", r.national, "(912)-34567");
    expectEqual("FN international sin prefijo", r.international, "+556 12-34567");
  }
  {
    std::string text = "Contactame al 123 456 789 o al fijo antiguo 000, gracias.";
    auto matches = findPhoneNumbers(text, "FM", MatchLeniency::Valid, -1);
    expectEqual("findNumbers cantidad", std::to_string(matches.size()), "1");
    if (!matches.empty()) {
      expectEqual("findNumbers texto", matches[0].text, "123 456 789");
      expectEqual("findNumbers start", std::to_string(matches[0].start), "14");
      expectEqual("findNumbers end", std::to_string(matches[0].end), "25");
      expectBool("findNumbers valid interno", matches[0].phoneNumber.valid, true);
    }
  }
  {
    std::string text = "Escribime al 123456789 o tambien al 987654321, cualquiera sirve.";
    auto matches = findPhoneNumbers(text, "FM", MatchLeniency::Valid, -1);
    expectEqual("findNumbers dos matches", std::to_string(matches.size()), "2");
  }
  {
    std::string text = "codigo interno room123456789no es telefono";
    auto matches = findPhoneNumbers(text, "FM", MatchLeniency::Valid, -1);
    expectEqual("findNumbers respeta limites de palabra", std::to_string(matches.size()), "0");
  }
  {
    std::string text = "Probá con 001234 en Ascensión.";
    auto strict = findPhoneNumbers(text, "AC", MatchLeniency::Valid, -1);
    auto loose = findPhoneNumbers(text, "AC", MatchLeniency::Possible, -1);
    expectEqual("findNumbers valid excluye posible-no-valido", std::to_string(strict.size()), "0");
    expectEqual("findNumbers possible incluye posible-no-valido", std::to_string(loose.size()), "1");
  }
  {
    std::string text = "123456789 y 987654321 y 111222333";
    auto matches = findPhoneNumbers(text, "FM", MatchLeniency::Valid, 2);
    expectEqual("findNumbers respeta maxTries", std::to_string(matches.size()), "2");
  }
  {
    AsYouType ayt("FM");
    ayt.inputDigit("0");
    ayt.inputDigit("1");
    ayt.inputDigit("2");
    std::string afterThree = ayt.inputDigit("3");
    expectEqual("AsYouType FM tras 4 digitos (incluye prefijo)", afterThree, "0123");
    ayt.inputDigit("4");
    ayt.inputDigit("5");
    ayt.inputDigit("6");
    ayt.inputDigit("7");
    ayt.inputDigit("8");
    std::string final = ayt.inputDigit("9");
    expectEqual("AsYouType FM coincide con formatNational estatico", final, "0123 456 789");
  }
  {
    AsYouType ayt("");
    ayt.inputDigit("+");
    ayt.inputDigit("5");
    ayt.inputDigit("5");
    std::string afterCode = ayt.inputDigit("5");
    expectEqual("AsYouType intl resuelve countryCode 555", afterCode, "+555 ");
    ayt.inputDigit("1");
    ayt.inputDigit("2");
    ayt.inputDigit("3");
    ayt.inputDigit("4");
    ayt.inputDigit("5");
    ayt.inputDigit("6");
    ayt.inputDigit("7");
    ayt.inputDigit("8");
    std::string final = ayt.inputDigit("9");
    expectEqual("AsYouType intl coincide con formatInternational estatico", final, "+555 123 456 789");
  }
  {
    AsYouType ayt("FM");
    ayt.inputDigit("0123");
    ayt.reset();
    std::string afterReset = ayt.inputDigit("9");
    expectEqual("AsYouType reset limpia estado", afterReset, "09");
  }

  std::cout << "\n" << (failures == 0 ? "TODO OK" : std::to_string(failures) + " FALLOS") << "\n";
  return failures == 0 ? 0 : 1;
}
