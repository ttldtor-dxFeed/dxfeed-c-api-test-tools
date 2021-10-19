#include <DXFeed.h>
#include <EventData.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <future>
#include <locale>
#include <string>
#include <utility>
#include <limits>
#include <vector>
#include <codecvt>

#ifdef _MSC_FULL_VER
#	pragma warning( push )
#	pragma warning( disable : 4244 )
#endif

struct StringConverter {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wstringConvert;

  static std::wstring utf8ToWString(const std::string &utf8) noexcept {
    try {
      return wstringConvert.from_bytes(utf8);
    } catch (...) {
      return {};
    }
  }

  static std::wstring utf8ToWString(const char* utf8) noexcept {
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

  static std::string wStringToUtf8(const wchar_t* utf16) noexcept {
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


#ifdef _MSC_FULL_VER
#	pragma warning( pop )
#endif

template<typename SymbolType>
struct EventType {
  [[nodiscard]] virtual const SymbolType &getEventSymbol() const = 0;

  virtual void setEventSymbol(const SymbolType &eventSymbol) = 0;

  virtual std::uint64_t getEventTime() {
    return 0;
  }

  virtual void setEventTime(std::uint64_t eventTime) {
  }

  virtual ~EventType() = default;
};

struct IndexedEventSource {
  static const IndexedEventSource DEFAULT;

  std::uint32_t id;
  std::string name;
};

const IndexedEventSource IndexedEventSource::DEFAULT = IndexedEventSource{0, "DEFAULT"};

template<typename SymbolType>
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

template<typename SymbolType>
struct TimeSeriesEvent : public virtual IndexedEvent<SymbolType> {
  IndexedEventSource getSource() override {
    return IndexedEventSource::DEFAULT;
  }
};

class MarketEvent : public virtual EventType<std::string> {
  std::string eventSymbol_{};
  std::uint64_t eventTime_{};

protected:
  MarketEvent() = default;

  explicit MarketEvent(std::string eventSymbol) : eventSymbol_{std::move(eventSymbol)} {
  }

public:
  [[nodiscard]] const std::string &getEventSymbol() const override {
    return eventSymbol_;
  }

  void setEventSymbol(const std::string &eventSymbol) override {
    eventSymbol_ = eventSymbol;
  }

  std::uint64_t getEventTime() override {
    return eventTime_;
  }

  void setEventTime(std::uint64_t eventTime) override {
    eventTime_ = eventTime;
  }
};

enum class OrderSide : int {
  UNDEFINED = 0,
  BUY = 1,
  SELL = 2
};

enum class TimeAndSaleType : int {
  NEW = 0,
  CORRECTION = 1,
  CANCEL = 2
};

enum class OrderScope : int {
  COMPOSITE = 0,
  REGIONAL = 1,
  AGGREGATE = 2,
  ORDER = 3
};

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

  explicit TimeAndSale(const std::string &eventSymbol, const dxf_time_and_sale_t &tns) :
    MarketEvent(eventSymbol), eventFlags_{tns.event_flags}, index_{static_cast<uint64_t>(tns.index)},
    time_{static_cast<uint64_t>(tns.time)}, exchangeCode_{StringConverter::wCharToUtf8(tns.exchange_code)},
    price_{tns.price}, size_{tns.size}, bidPrice_{tns.bid_price}, askPrice_{tns.ask_price},
    exchangeSaleConditions_(StringConverter::wStringToUtf8(tns.exchange_sale_conditions)), flags_{tns.raw_flags},
    buyer_(StringConverter::wStringToUtf8(tns.buyer)), seller_(StringConverter::wStringToUtf8(tns.seller)),
    side_{static_cast<OrderSide>(tns.side)}, type_{static_cast<TimeAndSaleType>(tns.type)},
    isValidTick_{static_cast<bool>(tns.is_valid_tick)},
    isEthTrade_{static_cast<bool>(tns.is_eth_trade)},
    tradeThroughExempt_{StringConverter::wCharToUtf8(tns.trade_through_exempt)},
    isSpreadLeg_{static_cast<bool>(tns.is_spread_leg)},
    scope_{static_cast<OrderScope>(tns.scope)} {}

  std::uint32_t getEventFlags() override {
    return eventFlags_;
  }

  void setEventFlags(std::uint32_t eventFlags) override {
    eventFlags_ = eventFlags;
  }

  std::uint64_t getIndex() override {
    return index_;
  }

  void setIndex(std::uint64_t index) override {
    index_ = index;
  }

  [[nodiscard]] std::uint64_t getTime() const {
    return time_;
  }

  void setTime(std::uint64_t time) {
    time_ = time;
  }

  [[nodiscard]] char getExchangeCode() const {
    return exchangeCode_;
  }

  void setExchangeCode(char exchangeCode) {
    exchangeCode_ = exchangeCode;
  }

  [[nodiscard]] double getPrice() const {
    return price_;
  }

  void setPrice(double price) {
    price_ = price;
  }

  [[nodiscard]] double getSize() const {
    return size_;
  }

  void setSize(double size) {
    size_ = size;
  }

  [[nodiscard]] double getBidPrice() const {
    return bidPrice_;
  }

  void setBidPrice(double bidPrice) {
    bidPrice_ = bidPrice;
  }

  [[nodiscard]] double getAskPrice() const {
    return askPrice_;
  }

  void setAskPrice(double askPrice) {
    askPrice_ = askPrice;
  }

  [[nodiscard]] const std::string &getExchangeSaleConditions() const {
    return exchangeSaleConditions_;
  }

  void setExchangeSaleConditions(const std::string &exchangeSaleConditions) {
    exchangeSaleConditions_ = exchangeSaleConditions;
  }

  [[nodiscard]] int32_t getFlags() const {
    return flags_;
  }

  void setFlags(int32_t flags) {
    flags_ = flags;
  }

  [[nodiscard]] const std::string &getBuyer() const {
    return buyer_;
  }

  void setBuyer(const std::string &buyer) {
    buyer_ = buyer;
  }

  [[nodiscard]] const std::string &getSeller() const {
    return seller_;
  }

  void setSeller(const std::string &seller) {
    seller_ = seller;
  }

  [[nodiscard]] OrderSide getSide() const {
    return side_;
  }

  void setSide(OrderSide side) {
    side_ = side;
  }

  [[nodiscard]] TimeAndSaleType getType() const {
    return type_;
  }

  void setType(TimeAndSaleType type) {
    type_ = type;
  }

  [[nodiscard]] bool isValidTick1() const {
    return isValidTick_;
  }

  void setIsValidTick(bool isValidTick) {
    isValidTick_ = isValidTick;
  }

  [[nodiscard]] bool isEthTrade1() const {
    return isEthTrade_;
  }

  void setIsEthTrade(bool isEthTrade) {
    isEthTrade_ = isEthTrade;
  }

  [[nodiscard]] char getTradeThroughExempt() const {
    return tradeThroughExempt_;
  }

  void setTradeThroughExempt(char tradeThroughExempt) {
    tradeThroughExempt_ = tradeThroughExempt;
  }

  [[nodiscard]] bool isSpreadLeg1() const {
    return isSpreadLeg_;
  }

  void setIsSpreadLeg(bool isSpreadLeg) {
    isSpreadLeg_ = isSpreadLeg;
  }

  [[nodiscard]] OrderScope getScope() const {
    return scope_;
  }

  void setScope(OrderScope scope) {
    scope_ = scope;
  }

  ~TimeAndSale() override = default;
};

class SimpleTimeAndSaleDataProvider {
  std::atomic<bool> disconnected_ = false;
  std::mutex eventsMutex_{};
  std::unordered_map<std::string, std::vector<TimeAndSale>> events_{};
  std::mutex cvMutex_{};
  std::condition_variable cv_{};

public:

  SimpleTimeAndSaleDataProvider() = default;

  std::future<std::unordered_map<std::string, std::vector<TimeAndSale>>>
  run(const std::string &address, const std::vector<std::string> &symbols) {
    return std::async(std::launch::async, [this, address, symbols]() {
      dxf_connection_t con = nullptr;
      auto res = dxf_create_connection(address.c_str(), [](dxf_connection_t c, void *data) {
        static_cast<decltype(this)>(data)->disconnected_ = true;
        static_cast<decltype(this)>(data)->cv_.notify_one();
      }, nullptr, nullptr, nullptr, static_cast<void *>(this), &con);

      if (res == DXF_FAILURE) {
        return events_;
      }

      dxf_subscription_t sub = nullptr;
      res = dxf_create_subscription_timed(con, DXF_ET_TIME_AND_SALE, 0, &sub);

      if (res == DXF_FAILURE) {
        return events_;
      }

      dxf_attach_event_listener(sub, [](int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData,
                                        int, void *userData) {
        if (eventType == DXF_ET_TIME_AND_SALE) {
          const auto *tns = reinterpret_cast<const dxf_time_and_sale_t *>(eventData);
          auto *that = static_cast<SimpleTimeAndSaleDataProvider *>(userData);
          auto symbol = StringConverter::wStringToUtf8(std::wstring(symbolName));
          auto timeAndSale = TimeAndSale(symbol, *tns);

          std::lock_guard guard(that->eventsMutex_);
          if (auto found = that->events_.find(symbol); found != that->events_.end()) {
            found->second.emplace_back(timeAndSale);
          } else {
            that->events_[symbol] = {timeAndSale};
          }
        }
      }, static_cast<void *>(this));

      std::vector<std::wstring> wSymbols(symbols.size());
      std::transform(symbols.begin(), symbols.end(), wSymbols.begin(),
                     [](auto s) { return StringConverter::utf8ToWString(s); });

      for (const auto& ws : wSymbols) {
        res = dxf_add_symbol(sub, ws.c_str());

        if (res == DXF_FAILURE) {
          return events_;
        }
      }

      {
        std::unique_lock lk(cvMutex_);
        cv_.wait(lk, [this] {
          bool disconnected = this->disconnected_;

          return disconnected;
        });
      }

      return events_;
    });
  }
};

int main(int argc, char *argv[]) {
  dxf_load_config_from_string("logger.level = \"debug\"\n");
  dxf_initialize_logger_v2("mt-reader.log", true, true, true, true);

  SimpleTimeAndSaleDataProvider provider{};
  SimpleTimeAndSaleDataProvider provider2{};

  auto f = provider.run(argv[1], {"/ESZ21:XCME"});
  auto f2 = provider2.run(argv[2], {"/ESZ21:XCME"});

  for (auto[s, v]: f.get()) {
    std::cout << s << "[" << v.size() << "]\n";
  }

  for (auto[s, v]: f2.get()) {
    std::cout << s << "[" << v.size() << "]\n";
  }

  return 0;
}