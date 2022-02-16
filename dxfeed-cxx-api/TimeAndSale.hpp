#pragma once

#include <DXFeed.h>

#include <cstdint>
#include <string>

#include "MarketEvent.hpp"
#include "OrderScope.hpp"
#include "OrderSide.hpp"
#include "StringConverter.hpp"
#include "TimeSeriesEvent.hpp"

namespace dxf {

enum class TimeAndSaleType : int { NEW = 0, CORRECTION = 1, CANCEL = 2 };

class TimeAndSale final : public MarketEvent, public TimeSeriesEvent<std::string> {
  std::uint32_t eventFlags_{};
  std::uint64_t index_{};
  std::uint64_t time_{};
  char exchangeCode_{};
  double price_{std::numeric_limits<double>::quiet_NaN()};
  double size_{std::numeric_limits<double>::quiet_NaN()};
  double bidPrice_{std::numeric_limits<double>::quiet_NaN()};
  double askPrice_{std::numeric_limits<double>::quiet_NaN()};
  std::string exchangeSaleConditions_{};
  std::int32_t flags_{};
  std::string buyer_{};
  std::string seller_{};
  OrderSide side_{};
  TimeAndSaleType type_{};
  bool isValidTick_ = false;
  bool isEthTrade_ = false;
  char tradeThroughExempt_ = false;
  bool isSpreadLeg_ = false;
  OrderScope scope_{};

 public:
  TimeAndSale() = default;

  explicit TimeAndSale(const std::string &eventSymbol) : MarketEvent(eventSymbol) {}

  explicit TimeAndSale(const std::string &eventSymbol, const dxf_time_and_sale_t &tns)
      : MarketEvent(eventSymbol),
        eventFlags_{tns.event_flags},
        index_{static_cast<uint64_t>(tns.index)},
        time_{static_cast<uint64_t>(tns.time)},
        exchangeCode_{StringConverter::wCharToUtf8(tns.exchange_code)},
        price_{tns.price},
        size_{tns.size},
        bidPrice_{tns.bid_price},
        askPrice_{tns.ask_price},
        exchangeSaleConditions_(StringConverter::wStringToUtf8(tns.exchange_sale_conditions)),
        flags_{tns.raw_flags},
        buyer_(StringConverter::wStringToUtf8(tns.buyer)),
        seller_(StringConverter::wStringToUtf8(tns.seller)),
        side_{static_cast<OrderSide>(tns.side)},
        type_{static_cast<TimeAndSaleType>(tns.type)},
        isValidTick_{static_cast<bool>(tns.is_valid_tick)},
        isEthTrade_{static_cast<bool>(tns.is_eth_trade)},
        tradeThroughExempt_{StringConverter::wCharToUtf8(tns.trade_through_exempt)},
        isSpreadLeg_{static_cast<bool>(tns.is_spread_leg)},
        scope_{static_cast<OrderScope>(tns.scope)} {}

  [[nodiscard]] const std::string &getEventSymbol() const override { return MarketEvent::getEventSymbol(); }

  void setEventSymbol(const std::string &eventSymbol) override { MarketEvent::setEventSymbol(eventSymbol); }

  uint64_t getEventTime() override { return MarketEvent::getEventTime(); }

  void setEventTime(std::uint64_t eventTime) override { MarketEvent::setEventTime(eventTime); }

  std::uint32_t getEventFlags() override { return eventFlags_; }

  void setEventFlags(std::uint32_t eventFlags) override { eventFlags_ = eventFlags; }

  std::uint64_t getIndex() override { return index_; }

  void setIndex(std::uint64_t index) override { index_ = index; }

  [[nodiscard]] std::uint64_t getTime() const { return time_; }

  void setTime(std::uint64_t time) { time_ = time; }

  [[nodiscard]] char getExchangeCode() const { return exchangeCode_; }

  void setExchangeCode(char exchangeCode) { exchangeCode_ = exchangeCode; }

  [[nodiscard]] double getPrice() const { return price_; }

  void setPrice(double price) { price_ = price; }

  [[nodiscard]] double getSize() const { return size_; }

  void setSize(double size) { size_ = size; }

  [[nodiscard]] double getBidPrice() const { return bidPrice_; }

  void setBidPrice(double bidPrice) { bidPrice_ = bidPrice; }

  [[nodiscard]] double getAskPrice() const { return askPrice_; }

  void setAskPrice(double askPrice) { askPrice_ = askPrice; }

  [[nodiscard]] const std::string &getExchangeSaleConditions() const { return exchangeSaleConditions_; }

  void setExchangeSaleConditions(const std::string &exchangeSaleConditions) {
    exchangeSaleConditions_ = exchangeSaleConditions;
  }

  [[nodiscard]] int32_t getFlags() const { return flags_; }

  void setFlags(int32_t flags) { flags_ = flags; }

  [[nodiscard]] const std::string &getBuyer() const { return buyer_; }

  void setBuyer(const std::string &buyer) { buyer_ = buyer; }

  [[nodiscard]] const std::string &getSeller() const { return seller_; }

  void setSeller(const std::string &seller) { seller_ = seller; }

  [[nodiscard]] OrderSide getSide() const { return side_; }

  void setSide(OrderSide side) { side_ = side; }

  [[nodiscard]] TimeAndSaleType getType() const { return type_; }

  void setType(TimeAndSaleType type) { type_ = type; }

  [[nodiscard]] bool isValidTick1() const { return isValidTick_; }

  void setIsValidTick(bool isValidTick) { isValidTick_ = isValidTick; }

  [[nodiscard]] bool isEthTrade1() const { return isEthTrade_; }

  void setIsEthTrade(bool isEthTrade) { isEthTrade_ = isEthTrade; }

  [[nodiscard]] char getTradeThroughExempt() const { return tradeThroughExempt_; }

  void setTradeThroughExempt(char tradeThroughExempt) { tradeThroughExempt_ = tradeThroughExempt; }

  [[nodiscard]] bool isSpreadLeg1() const { return isSpreadLeg_; }

  void setIsSpreadLeg(bool isSpreadLeg) { isSpreadLeg_ = isSpreadLeg; }

  [[nodiscard]] OrderScope getScope() const { return scope_; }

  void setScope(OrderScope scope) { scope_ = scope; }

  ~TimeAndSale() override = default;
};

}