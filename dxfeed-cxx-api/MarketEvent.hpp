#pragma once

#include <cstdint>
#include <string>

#include "EventType.hpp"

namespace dxf {

class MarketEvent : public virtual EventType<std::string> {
  std::string eventSymbol_{};
  std::uint64_t eventTime_{};

 protected:
  MarketEvent() = default;

  explicit MarketEvent(std::string eventSymbol) : eventSymbol_{std::move(eventSymbol)} {}

 public:
  [[nodiscard]] const std::string &getEventSymbol() const override { return eventSymbol_; }

  void setEventSymbol(const std::string &eventSymbol) override { eventSymbol_ = eventSymbol; }

  std::uint64_t getEventTime() override { return eventTime_; }

  void setEventTime(std::uint64_t eventTime) override { eventTime_ = eventTime; }
};

}  // namespace dxf