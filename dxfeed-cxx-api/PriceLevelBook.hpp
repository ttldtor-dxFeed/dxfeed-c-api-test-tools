#pragma once

#include <DXFeed.h>

#include <functional>
#include <limits>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "StringConverter.hpp"

namespace dxf {

struct PriceLevel {
  double price = std::numeric_limits<double>::quiet_NaN();
  double size = std::numeric_limits<double>::quiet_NaN();
  std::int64_t time = 0;
};

struct PriceLevelChanges {
  std::vector<PriceLevel> asks;
  std::vector<PriceLevel> bids;
};

class PriceLevelBook final {
  dxf_snapshot_t snapshot_;
  std::string symbol_;
  std::string source_;
  std::size_t levelsNumber_;
  std::vector<PriceLevel> asks_;
  std::vector<PriceLevel> bids_;
  bool isValid_;
  std::mutex mutex_;

  std::function<void(const PriceLevelChanges&)> onNewSnapshot_;
  std::function<void(const PriceLevelChanges&, const PriceLevelChanges&, const PriceLevelChanges&)> onChange_;

  PriceLevelBook(std::string symbol, std::string source, std::size_t levelsNumber)
      : snapshot_{nullptr},
        symbol_{std::move(symbol)},
        source_{std::move(source)},
        levelsNumber_{levelsNumber},
        asks_(levelsNumber),
        bids_(levelsNumber),
        isValid_{false},
        mutex_{} {}

 public:
  void processSnapshotData(const dxf_snapshot_data_ptr_t snapshotData, int newSnapshot) {
    std::lock_guard<std::mutex> lk(mutex_);
  }

  ~PriceLevelBook() {
    if (isValid_) {
      dxf_close_price_level_book(snapshot_);
    }
  }

  static std::unique_ptr<PriceLevelBook> create(dxf_connection_t connection, const std::string& symbol, const std::string& source,
                               std::size_t levelsNumber) {
    auto plb = std::unique_ptr<PriceLevelBook>(new PriceLevelBook(symbol, source, levelsNumber));
    auto wSymbol = StringConverter::utf8ToWString(symbol);
    dxf_snapshot_t snapshot = nullptr;

    dxf_create_order_snapshot(connection, wSymbol.c_str(), source.c_str(), 0, &snapshot);

    dxf_attach_snapshot_inc_listener(
      snapshot,
      [](const dxf_snapshot_data_ptr_t snapshot_data, int new_snapshot, void* user_data) {
        static_cast<PriceLevelBook*>(user_data)->processSnapshotData(snapshot_data, new_snapshot);
      },
      plb.get());
    plb->isValid_ = true;

    return plb;
  }
};  // namespace dxf

}  // namespace dxf