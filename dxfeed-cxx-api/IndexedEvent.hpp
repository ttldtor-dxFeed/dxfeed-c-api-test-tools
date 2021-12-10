#pragma once

#include <cstdint>
#include <string>

#include "EventType.hpp"

namespace dxf {

struct IndexedEventSource {
  static const IndexedEventSource DEFAULT;

  std::uint32_t id;
  std::string name;
};

const IndexedEventSource IndexedEventSource::DEFAULT = IndexedEventSource{0, "DEFAULT"};

template <typename SymbolType>
struct IndexedEvent : public virtual EventType<SymbolType> {
  static const std::uint32_t TX_PENDING = 0x01;
  static const std::uint32_t REMOVE_EVENT = 0x02;
  static const std::uint32_t SNAPSHOT_BEGIN = 0x04;
  static const std::uint32_t SNAPSHOT_END = 0x08;
  static const std::uint32_t SNAPSHOT_SNIP = 0x10;
  static const std::uint32_t SNAPSHOT_MODE = 0x40;

  virtual IndexedEventSource getSource() = 0;

  virtual std::uint32_t getEventFlags() = 0;

  virtual void setEventFlags(std::uint32_t eventFlags) = 0;

  virtual std::uint64_t getIndex() = 0;

  virtual void setIndex(std::uint64_t index) = 0;
};

}  // namespace dxf