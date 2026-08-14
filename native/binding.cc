#include <napi.h>

#include "core/as_you_type.h"
#include "core/metadata_registry.h"
#include "core/phone_number_finder.h"
#include "core/phone_number_util.h"
#include "generated/metadata.gen.h"

namespace {

bool gInitialized = false;

void EnsureInitialized() {
  if (!gInitialized) {
    elitephone::registerGeneratedMetadata();
    gInitialized = true;
  }
}

std::string CategoryToString(elitephone::NumberCategory category) {
  switch (category) {
    case elitephone::NumberCategory::FixedLine: return "fixedLine";
    case elitephone::NumberCategory::Mobile: return "mobile";
    case elitephone::NumberCategory::FixedLineOrMobile: return "fixedLineOrMobile";
    case elitephone::NumberCategory::Pager: return "pager";
    case elitephone::NumberCategory::TollFree: return "tollFree";
    case elitephone::NumberCategory::PremiumRate: return "premiumRate";
    case elitephone::NumberCategory::SharedCost: return "sharedCost";
    case elitephone::NumberCategory::PersonalNumber: return "personalNumber";
    case elitephone::NumberCategory::Voip: return "voip";
    case elitephone::NumberCategory::Uan: return "uan";
    case elitephone::NumberCategory::Voicemail: return "voicemail";
    default: return "unknown";
  }
}

Napi::Object BuildParseResultObject(Napi::Env env, const elitephone::ParseResult& result) {
  Napi::Object output = Napi::Object::New(env);
  output.Set("valid", Napi::Boolean::New(env, result.valid));
  output.Set("possible", Napi::Boolean::New(env, result.possible));
  output.Set("regionCode", Napi::String::New(env, result.regionCode));
  output.Set("countryCode", Napi::Number::New(env, result.countryCode));
  output.Set("nationalNumber", Napi::String::New(env, result.nationalNumber));
  output.Set("category", Napi::String::New(env, CategoryToString(result.category)));
  if (!result.e164.empty()) {
    output.Set("e164", Napi::String::New(env, result.e164));
  }
  if (!result.international.empty()) {
    output.Set("international", Napi::String::New(env, result.international));
  }
  if (!result.national.empty()) {
    output.Set("national", Napi::String::New(env, result.national));
  }
  if (!result.rfc3966.empty()) {
    output.Set("rfc3966", Napi::String::New(env, result.rfc3966));
  }
  if (!result.significant.empty()) {
    output.Set("significant", Napi::String::New(env, result.significant));
  }
  if (!result.error.empty()) {
    output.Set("error", Napi::String::New(env, result.error));
  }
  return output;
}

Napi::Value ParsePhoneNumber(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  EnsureInitialized();

  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "elite-phone: se espera un string como primer argumento")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string rawNumber = info[0].As<Napi::String>().Utf8Value();
  std::string defaultRegionCode;
  if (info.Length() > 1 && info[1].IsString()) {
    defaultRegionCode = info[1].As<Napi::String>().Utf8Value();
  }

  elitephone::ParseResult result = elitephone::parsePhoneNumber(rawNumber, defaultRegionCode);
  return BuildParseResultObject(env, result);
}

Napi::Value FindNumbers(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  EnsureInitialized();

  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "elite-phone: se espera un string como primer argumento")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string text = info[0].As<Napi::String>().Utf8Value();
  std::string defaultRegionCode;
  elitephone::MatchLeniency leniency = elitephone::MatchLeniency::Valid;
  int maxTries = -1;

  if (info.Length() > 1 && info[1].IsObject()) {
    Napi::Object options = info[1].As<Napi::Object>();
    if (options.Has("defaultRegionCode") && options.Get("defaultRegionCode").IsString()) {
      defaultRegionCode = options.Get("defaultRegionCode").As<Napi::String>().Utf8Value();
    }
    if (options.Has("leniency") && options.Get("leniency").IsString()) {
      std::string leniencyValue = options.Get("leniency").As<Napi::String>().Utf8Value();
      if (leniencyValue == "possible") {
        leniency = elitephone::MatchLeniency::Possible;
      }
    }
    if (options.Has("maxTries") && options.Get("maxTries").IsNumber()) {
      maxTries = options.Get("maxTries").As<Napi::Number>().Int32Value();
    }
  }

  auto matches = elitephone::findPhoneNumbers(text, defaultRegionCode, leniency, maxTries);

  Napi::Array output = Napi::Array::New(env, matches.size());
  for (size_t i = 0; i < matches.size(); ++i) {
    Napi::Object matchObject = Napi::Object::New(env);
    matchObject.Set("text", Napi::String::New(env, matches[i].text));
    matchObject.Set("start", Napi::Number::New(env, static_cast<double>(matches[i].start)));
    matchObject.Set("end", Napi::Number::New(env, static_cast<double>(matches[i].end)));
    matchObject.Set("phoneNumber", BuildParseResultObject(env, matches[i].phoneNumber));
    output.Set(static_cast<uint32_t>(i), matchObject);
  }
  return output;
}

Napi::Value GetSupportedRegionCodes(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  EnsureInitialized();
  auto codes = elitephone::MetadataRegistry::instance().allRegionCodes();
  Napi::Array output = Napi::Array::New(env, codes.size());
  for (size_t i = 0; i < codes.size(); ++i) {
    output.Set(static_cast<uint32_t>(i), Napi::String::New(env, codes[i]));
  }
  return output;
}

Napi::Value GetCountryCodeForRegionCode(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  EnsureInitialized();
  if (info.Length() < 1 || !info[0].IsString()) {
    return Napi::Number::New(env, 0);
  }
  std::string regionCode = info[0].As<Napi::String>().Utf8Value();
  const auto* territory = elitephone::MetadataRegistry::instance().findByRegionCode(regionCode);
  return Napi::Number::New(env, territory != nullptr ? territory->countryCode : 0);
}

Napi::Value GetRegionCodeForCountryCode(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  EnsureInitialized();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    return Napi::String::New(env, "");
  }
  int countryCode = info[0].As<Napi::Number>().Int32Value();
  const auto* territory = elitephone::MetadataRegistry::instance().mainTerritoryForCountryCode(countryCode);
  return Napi::String::New(env, territory != nullptr ? territory->id : "");
}

Napi::String NativeVersion(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), "0.4.0");
}

class NativeAsYouType : public Napi::ObjectWrap<NativeAsYouType> {
 public:
  static Napi::Function GetClass(Napi::Env env) {
    return DefineClass(env, "AsYouType", {
      InstanceMethod("inputDigit", &NativeAsYouType::InputDigit),
      InstanceMethod("reset", &NativeAsYouType::Reset),
      InstanceMethod("currentOutput", &NativeAsYouType::CurrentOutput),
    });
  }

  NativeAsYouType(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<NativeAsYouType>(info), impl_("") {
    EnsureInitialized();
    std::string regionCode;
    if (info.Length() > 0 && info[0].IsString()) {
      regionCode = info[0].As<Napi::String>().Utf8Value();
    }
    impl_ = elitephone::AsYouType(regionCode);
  }

 private:
  elitephone::AsYouType impl_;

  Napi::Value InputDigit(const Napi::CallbackInfo& info) {
    std::string input;
    if (info.Length() > 0 && info[0].IsString()) {
      input = info[0].As<Napi::String>().Utf8Value();
    }
    return Napi::String::New(info.Env(), impl_.inputDigit(input));
  }

  Napi::Value Reset(const Napi::CallbackInfo& info) {
    impl_.reset();
    return info.Env().Undefined();
  }

  Napi::Value CurrentOutput(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), impl_.currentOutput());
  }
};

}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "nativeVersion"), Napi::Function::New(env, NativeVersion));
  exports.Set(Napi::String::New(env, "parsePhoneNumber"), Napi::Function::New(env, ParsePhoneNumber));
  exports.Set(Napi::String::New(env, "findNumbers"), Napi::Function::New(env, FindNumbers));
  exports.Set("AsYouType", NativeAsYouType::GetClass(env));
  exports.Set(Napi::String::New(env, "getSupportedRegionCodes"), Napi::Function::New(env, GetSupportedRegionCodes));
  exports.Set(
      Napi::String::New(env, "getCountryCodeForRegionCode"),
      Napi::Function::New(env, GetCountryCodeForRegionCode));
  exports.Set(
      Napi::String::New(env, "getRegionCodeForCountryCode"),
      Napi::Function::New(env, GetRegionCodeForCountryCode));
  return exports;
}

NODE_API_MODULE(elite_phone_native, Init)
