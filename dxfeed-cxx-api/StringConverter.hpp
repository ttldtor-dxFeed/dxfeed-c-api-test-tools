#pragma once

#include <string>

#ifdef _MSC_FULL_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <codecvt>

namespace dxf {

struct StringConverter {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wstringConvert;

  static std::wstring utf8ToWString(const std::string &utf8) noexcept {
    try {
      return wstringConvert.from_bytes(utf8);
    } catch (...) {
      return {};
    }
  }

  static std::wstring utf8ToWString(const char *utf8) noexcept {
    if (utf8 == nullptr) {
      return {};
    }

    try {
      return wstringConvert.from_bytes(utf8);
    } catch (...) {
      return {};
    }
  }

  static wchar_t utf8ToWChar(char c) noexcept {
    if (c == '\0') {
      return L'\0';
    }

    return utf8ToWString(std::string(1, c))[0];
  }

  static std::string wStringToUtf8(const std::wstring &utf16) noexcept {
    try {
      return wstringConvert.to_bytes(utf16);
    } catch (...) {
      return {};
    }
  }

  static std::string wStringToUtf8(const wchar_t *utf16) noexcept {
    if (utf16 == nullptr) {
      return {};
    }

    try {
      return wstringConvert.to_bytes(utf16);
    } catch (...) {
      return {};
    }
  }

  static char wCharToUtf8(wchar_t c) noexcept {
    if (c == L'\0') {
      return '\0';
    }

    return wStringToUtf8(std::wstring(1, c))[0];
  }
};

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> StringConverter::wstringConvert{};

}

#ifdef _MSC_FULL_VER
#pragma warning(pop)
#endif
