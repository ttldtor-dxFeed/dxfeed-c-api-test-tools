#pragma once

#include <DXFeed.h>
#include <fmt/format.h>

#include <string>

#include "StringConverter.hpp"

namespace dxf {

struct Error {
  const int code;
  const std::wstring description;

  static Error getLast() {
    int code;
    dxf_const_string_t description;

    if (dxf_get_last_error(&code, &description) == DXF_SUCCESS) {
      return {code, description};
    }

    return {-1, {}};
  }

  [[nodiscard]] std::string toString() const {
    if (description.empty()) {
      return "";
    }

    return fmt::format("[{}] {}", code, StringConverter::wStringToUtf8(description));
  }

  template <typename OutStream>
  friend OutStream &operator<<(OutStream &os, const Error &error) {
    os << error.toString();

    return os;
  }
};

}  // namespace dxf