{
  "targets": [
    {
      "target_name": "elite_phone_native",
      "sources": [
        "native/binding.cc",
        "native/core/metadata_registry.cc",
        "native/core/phone_number_util.cc",
        "native/core/phone_number_formatter.cc",
        "native/core/phone_number_finder.cc",
        "native/core/format_expansion.cc",
        "native/core/as_you_type.cc",
        "native/generated/metadata.gen.cc"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
      "cflags_cc": ["-std=c++17"],
      "xcode_settings": {
        "CLANG_CXX_LANGUAGE_STANDARD": "c++17"
      }
    }
  ]
}
