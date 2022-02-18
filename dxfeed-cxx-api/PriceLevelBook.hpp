#pragma once

#include <DXFeed.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "StringConverter.hpp"

namespace dxf {

struct OrderData {
  dxf_long_t index = 0;
  double price = std::numeric_limits<double>::quiet_NaN();
  double size = std::numeric_limits<double>::quiet_NaN();
  dxf_long_t time = 0;
  dxf_order_side_t side = dxf_osd_undefined;
};

struct PriceLevel {
  double price = std::numeric_limits<double>::quiet_NaN();
  double size = std::numeric_limits<double>::quiet_NaN();
  std::int64_t time = 0;

  friend bool operator<(const PriceLevel& a, const PriceLevel& b) {
    if (std::isnan(b.price)) return true;
    if (std::isnan(a.price)) return false;

    return a.price < b.price;
  }
};

struct PriceLevelChanges {
  std::vector<PriceLevel> asks{};
  std::vector<PriceLevel> bids{};
};

struct PriceLevelChangesSet {
  PriceLevelChanges additions{};
  PriceLevelChanges updates{};
  PriceLevelChanges removals{};
};

class PriceLevelBook final {
  dxf_snapshot_t snapshot_;
  std::string symbol_;
  std::string source_;
  std::size_t levelsNumber_;
  std::set<PriceLevel> asks_;
  std::set<PriceLevel> bids_;
  std::unordered_map<dxf_long_t, OrderData> orderDataSnapshot_;
  bool isValid_;
  std::mutex mutex_;

  std::function<void(const PriceLevelChanges&)> onNewBook_;
  std::function<void(const PriceLevelChanges&)> onBookUpdate_;
  std::function<void(const PriceLevelChangesSet&)> onIncrementalChange_;

  static bool isZeroPriceLevel(const PriceLevel& pl) {
    return std::abs(pl.size) < std::numeric_limits<double>::epsilon();
  };

  PriceLevelBook(std::string symbol, std::string source, std::size_t levelsNumber)
      : snapshot_{nullptr},
        symbol_{std::move(symbol)},
        source_{std::move(source)},
        levelsNumber_{levelsNumber},
        asks_{},
        bids_{},
        orderDataSnapshot_{},
        isValid_{false},
        mutex_{} {}

  // Process the tx\snapshot data, converts it to PL changes. Also, changes the orderDataSnapshot_
  PriceLevelChanges convertToUpdates(const dxf_snapshot_data_ptr_t snapshotData) {
    assert(snapshotData->records_count != 0);
    assert(snapshotData->event_type != dx_eid_order);

    std::set<PriceLevel> askUpdates{};
    std::set<PriceLevel> bidUpdates{};

    auto orders = reinterpret_cast<const dxf_order_t*>(snapshotData->records);

    auto isOrderRemoval = [](const dxf_order_t& o) {
      return (o.event_flags & dxf_ef_remove_event) != 0 || o.size == 0 || std::isnan(o.size);
    };

    for (std::size_t i = 0; i < snapshotData->records_count; i++) {
      auto order = orders[i];
      auto removal = isOrderRemoval(order);
      auto foundOrder = orderDataSnapshot_.find(order.index);

      if (foundOrder == orderDataSnapshot_.end()) {
        if (removal) {
          continue;
        }

        // ADD (Order)
        auto& updatesSide = order.side == dxf_osd_buy ? bidUpdates : askUpdates;
        auto priceLevelChange = PriceLevel{order.price, order.size, order.time};
        auto foundPriceLevel = updatesSide.find(priceLevelChange);

        if (foundPriceLevel != updatesSide.end()) {
          // UPDATE (PL) : remove + insert
          priceLevelChange.size = foundPriceLevel->size + priceLevelChange.size;
          updatesSide.erase(foundPriceLevel);
        }

        // INSERT (PL)
        updatesSide.insert(priceLevelChange);
        orderDataSnapshot_[order.index] = OrderData{order.index, order.price, order.size, order.time, order.side};
      } else {
        auto& updatesSide = foundOrder->second.side == dxf_osd_buy ? bidUpdates : askUpdates;

        if (removal) {
          // REMOVE (Order)
          auto priceLevelChange = PriceLevel{foundOrder->second.price, -foundOrder->second.size, order.time};
          auto foundPriceLevel = updatesSide.find(priceLevelChange);

          if (foundPriceLevel != updatesSide.end()) {
            // REMOVE (PL)
            priceLevelChange.size = foundPriceLevel->size + priceLevelChange.size;
            updatesSide.erase(foundPriceLevel);
          }

          if (!isZeroPriceLevel(priceLevelChange)) {
            // UPDATE (PL)
            updatesSide.insert(priceLevelChange);
          }

          orderDataSnapshot_.erase(foundOrder->second.index);
        } else {
          // UPDATE (Order)
          auto priceLevelChange = PriceLevel{order.price, order.size, order.time};
          auto foundPriceLevel = updatesSide.find(priceLevelChange);

          if (foundPriceLevel != updatesSide.end()) {
            // UPDATE (PL)
            priceLevelChange.size = foundPriceLevel->size + priceLevelChange.size;
            updatesSide.erase(foundPriceLevel);
          }

          // INSERT (PL)
          updatesSide.insert(priceLevelChange);
          orderDataSnapshot_[foundOrder->second.index] =
            OrderData{order.index, order.price, order.size, order.time, order.side};
        }
      }
    }

    return {std::vector<PriceLevel>(askUpdates.begin(), askUpdates.end()),
            std::vector<PriceLevel>(bidUpdates.rbegin(), bidUpdates.rend())};
  }

  //TODO: price levels number
  PriceLevelChangesSet applyUpdates(const PriceLevelChanges& priceLevelUpdates) {
    PriceLevelChanges additions{};
    PriceLevelChanges updates{};
    PriceLevelChanges removals{};

    for (auto updateAsk : priceLevelUpdates.asks) {
      auto found = asks_.find(updateAsk);

      if (found == asks_.end()) {
        asks_.insert(updateAsk);
        additions.asks.push_back(updateAsk);
      } else {
        auto newPriceLevelChange = *found;

        newPriceLevelChange.size += updateAsk.size;
        newPriceLevelChange.time = updateAsk.time;

        if (isZeroPriceLevel(newPriceLevelChange)) {
          removals.asks.push_back(*found);
          asks_.erase(found);
        } else {
          updates.asks.push_back(newPriceLevelChange);
          asks_.erase(found);
          asks_.insert(newPriceLevelChange);
        }
      }
    }

    for (auto updateBid : priceLevelUpdates.bids) {
      auto found = bids_.find(updateBid);

      if (found == bids_.end()) {
        bids_.insert(updateBid);
        additions.bids.push_back(updateBid);
      } else {
        auto newPriceLevelChange = *found;

        newPriceLevelChange.size += updateBid.size;
        newPriceLevelChange.time = updateBid.time;

        if (isZeroPriceLevel(newPriceLevelChange)) {
          removals.bids.push_back(*found);
          bids_.erase(found);
        } else {
          updates.bids.push_back(newPriceLevelChange);
          bids_.erase(found);
          bids_.insert(newPriceLevelChange);
        }
      }
    }

    return {additions, updates, removals};
  }

 public:
  // TODO: move to another thread
  void processSnapshotData(const dxf_snapshot_data_ptr_t snapshotData, int newSnapshot) {
    std::lock_guard<std::mutex> lk(mutex_);

    auto newSnap = newSnapshot != 0;

    if (newSnap) {
      asks_.clear();
      bids_.clear();
      orderDataSnapshot_.clear();
    }

    if (snapshotData->records_count == 0) {
      if (newSnap && onNewBook_) {
        onNewBook_({});
      }

      return;
    }

    auto updates = convertToUpdates(snapshotData);
    auto resultingChangesSet = applyUpdates(updates);

    if (newSnap) {
      if (onNewBook_) {
        onNewBook_(PriceLevelChanges{std::vector<PriceLevel>{asks_.begin(), asks_.end()},
                                     std::vector<PriceLevel>{bids_.rbegin(), bids_.rend()}});
      }
    } else {
      if (onIncrementalChange_) {
        onIncrementalChange_(resultingChangesSet);
      }

      if (onBookUpdate_) {
        onBookUpdate_(PriceLevelChanges{std::vector<PriceLevel>{asks_.begin(), asks_.end()},
                                        std::vector<PriceLevel>{bids_.rbegin(), bids_.rend()}});
      }
    }
  }

  ~PriceLevelBook() {
    if (isValid_) {
      dxf_close_price_level_book(snapshot_);
    }
  }

  static std::unique_ptr<PriceLevelBook> create(dxf_connection_t connection, const std::string& symbol,
                                                const std::string& source, std::size_t levelsNumber) {
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

  void setOnNewBook(std::function<void(const PriceLevelChanges&)> onNewBookHandler) {
    onNewBook_ = std::move(onNewBookHandler);
  }

  void setOnBookUpdate(std::function<void(const PriceLevelChanges&)> onBookUpdateHandler) {
    onBookUpdate_ = std::move(onBookUpdateHandler);
  }

  void setOnIncrementalChange(std::function<void(const PriceLevelChangesSet&)> onIncrementalChangeHandler) {
    onIncrementalChange_ = std::move(onIncrementalChangeHandler);
  }
};

}  // namespace dxf