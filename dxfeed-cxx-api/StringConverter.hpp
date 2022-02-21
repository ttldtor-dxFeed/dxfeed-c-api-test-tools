#pragma once

#include <string>
#include <boost/locale/encoding_utf.hpp>

namespace dxf {

struct StringConverter {
  static std::wstring utf8ToWString(const std::string &utf8) noexcept {
    return boost::locale::conv::utf_to_utf<wchar_t>(utf8);
  }

  static std::wstring utf8ToWString(const char *utf8) noexcept {
    if (utf8 == nullptr) {
      return {};
    }

    return boost::locale::conv::utf_to_utf<wchar_t>(utf8);
  }

  static wchar_t utf8ToWChar(char c) noexcept {
    if (c == '\0') {
      return L'\0';
    }

    return utf8ToWString(std::string(1, c))[0];
  }

  static std::string wStringToUtf8(const std::wstring &utf16) noexcept {
    return boost::locale::conv::utf_to_utf<char>(utf16);
  }

  static std::string wStringToUtf8(const wchar_t *utf16) noexcept {
    if (utf16 == nullptr) {
      return {};
    }

    return boost::locale::conv::utf_to_utf<char>(utf16);
  }

  static char wCharToUtf8(wchar_t c) noexcept {
    if (c == L'\0') {
      return '\0';
    }

    return wStringToUtf8(std::wstring(1, c))[0];
  }
};

}

