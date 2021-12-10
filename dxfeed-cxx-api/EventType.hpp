#pragma once

#include <cstdint>

namespace dxf {

template <typename SymbolType>
struct EventType {
  [[nodiscard]] virtual const SymbolType &getEventSymbol() const = 0;

  virtual void setEventSymbol(const SymbolType &eventSymbol) = 0;

  virtual std::uint64_t getEventTime() { return 0; }

  virtual void setEventTime(std::uint64_t eventTime) {}

  virtual ~EventType() = default;
};

}  // namespace dxf