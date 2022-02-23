#pragma once

#include <DXFeed.h>
#include <fmt/format.h>

#include <algorithm>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>
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
static inline constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

struct OrderData {
  dxf_long_t index = 0;
  double price = NaN;
  double size = NaN;
  dxf_long_t time = 0;
  dxf_order_side_t side = dxf_osd_undefined;
};

struct PriceLevel {
  double price = NaN;
  double size = NaN;
  std::int64_t time = 0;
};

struct AskPriceLevel : PriceLevel {
  friend bool operator<(const AskPriceLevel& a, const AskPriceLevel& b) {
    if (std::isnan(b.price)) return true;
    if (std::isnan(a.price)) return false;

    return a.price < b.price;
  }
};

struct BidPriceLevel : PriceLevel {
  friend bool operator<(const BidPriceLevel& a, const BidPriceLevel& b) {
    if (std::isnan(b.price)) return false;
    if (std::isnan(a.price)) return true;

    return a.price > b.price;
  }
};

struct PriceLevelChanges {
  std::vector<AskPriceLevel> asks{};
  std::vector<BidPriceLevel> bids{};
};

struct PriceLevelChangesSet {
  PriceLevelChanges additions{};
  PriceLevelChanges updates{};
  PriceLevelChanges removals{};
};

namespace bmi = boost::multi_index;

using PriceLevelContainer = std::set<PriceLevel>;

class PriceLevelBook final {
  dxf_snapshot_t snapshot_;
  std::string symbol_;
  std::string source_;
  std::size_t levelsNumber_;

  /*
   * Since we are working with std::set, which does not imply random access, we have to keep the iterators lastAsk,
   * lastBid for the case with a limit on the number of PLs. These iterators, as well as the look-ahead functions, are
   * used to find out if we are performing operations on PL within the range given to the user.
   */
  std::set<AskPriceLevel> asks_;
  std::set<AskPriceLevel>::iterator lastAsk_;
  std::set<BidPriceLevel> bids_;
  std::set<BidPriceLevel>::iterator lastBid_;
  std::unordered_map<dxf_long_t, OrderData> orderDataSnapshot_;
  bool isValid_;
  std::mutex mutex_;

  std::function<void(const PriceLevelChanges&)> onNewBook_;
  std::function<void(const PriceLevelChanges&)> onBookUpdate_;
  std::function<void(const PriceLevelChangesSet&)> onIncrementalChange_;

  static bool isZeroPriceLevel(const PriceLevel& pl) {
    return std::abs(pl.size) < std::numeric_limits<double>::epsilon();
  };

  static bool areEqualPrices(double price1, double price2) {
    return std::abs(price1 - price2) < std::numeric_limits<double>::epsilon();
  }

  PriceLevelBook(std::string symbol, std::string source, std::size_t levelsNumber = 0)
      : snapshot_{nullptr},
        symbol_{std::move(symbol)},
        source_{std::move(source)},
        levelsNumber_{levelsNumber},
        asks_{},
        lastAsk_{asks_.end()},
        bids_{},
        lastBid_{bids_.end()},
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

    auto processOrderAddition = [&bidUpdates, &askUpdates](const dxf_order_t& order) {
      auto& updatesSide = order.side == dxf_osd_buy ? bidUpdates : askUpdates;
      auto priceLevelChange = PriceLevel{order.price, order.size, order.time};
      auto foundPriceLevel = updatesSide.find(priceLevelChange);

      if (foundPriceLevel != updatesSide.end()) {
        priceLevelChange.size = foundPriceLevel->size + priceLevelChange.size;
        updatesSide.erase(foundPriceLevel);
      }

      updatesSide.insert(priceLevelChange);
    };

    auto processOrderRemoval = [&bidUpdates, &askUpdates](const dxf_order_t& order, const OrderData& foundOrderData) {
      auto& updatesSide = foundOrderData.side == dxf_osd_buy ? bidUpdates : askUpdates;
      auto priceLevelChange = PriceLevel{foundOrderData.price, -foundOrderData.size, order.time};
      auto foundPriceLevel = updatesSide.find(priceLevelChange);

      if (foundPriceLevel != updatesSide.end()) {
        priceLevelChange.size = foundPriceLevel->size + priceLevelChange.size;
        updatesSide.erase(foundPriceLevel);
      }

      if (!isZeroPriceLevel(priceLevelChange)) {
        updatesSide.insert(priceLevelChange);
      }
    };

    for (std::size_t i = 0; i < snapshotData->records_count; i++) {
      auto order = orders[i];

      fmt::print("O:ind={},pr={},sz={},sd={}\n", order.index, order.price, order.size,
                 order.side == dxf_osd_buy ? "buy" : "sell");

      auto removal = isOrderRemoval(order);
      auto foundOrderDataIt = orderDataSnapshot_.find(order.index);

      if (foundOrderDataIt == orderDataSnapshot_.end()) {
        if (removal) {
          continue;
        }

        processOrderAddition(order);
        orderDataSnapshot_[order.index] = OrderData{order.index, order.price, order.size, order.time, order.side};
      } else {
        const auto& foundOrderData = foundOrderDataIt->second;

        if (removal) {
          processOrderRemoval(order, foundOrderData);
          orderDataSnapshot_.erase(foundOrderData.index);
        } else {
          if (order.side != foundOrderData.side) {
            processOrderRemoval(order, foundOrderData);
          }

          processOrderAddition(order);
          orderDataSnapshot_[foundOrderData.index] =
            OrderData{order.index, order.price, order.size, order.time, order.side};
        }
      }
    }

    return {std::vector<AskPriceLevel>{askUpdates.begin(), askUpdates.end()},
            std::vector<BidPriceLevel>{bidUpdates.rbegin(), bidUpdates.rend()}};
  }

  PriceLevelChangesSet applyUpdates(const PriceLevelChanges& priceLevelUpdates) {
    PriceLevelChanges additions{};
    PriceLevelChanges updates{};
    PriceLevelChanges removals{};

    // We generate lists of additions, updates, removals
    for (const auto& updateAsk : priceLevelUpdates.asks) {
      auto found = asks_.find(updateAsk);

      if (found == asks_.end()) {
        additions.asks.push_back(updateAsk);
      } else {
        auto newPriceLevelChange = *found;

        newPriceLevelChange.size += updateAsk.size;
        newPriceLevelChange.time = updateAsk.time;

        if (isZeroPriceLevel(newPriceLevelChange)) {
          removals.asks.push_back(*found);
        } else {
          updates.asks.push_back(newPriceLevelChange);
        }
      }
    }

    for (const auto& updateBid : priceLevelUpdates.bids) {
      auto found = bids_.find(updateBid);

      if (found == bids_.end()) {
        additions.bids.push_back(updateBid);
      } else {
        auto newPriceLevelChange = *found;

        newPriceLevelChange.size += updateBid.size;
        newPriceLevelChange.time = updateBid.time;

        if (isZeroPriceLevel(newPriceLevelChange)) {
          removals.bids.push_back(*found);
        } else {
          updates.bids.push_back(newPriceLevelChange);
        }
      }
    }

    std::set<PriceLevel> askRemovals{};
    std::set<PriceLevel> bidRemovals{};
    std::set<PriceLevel> askAdditions{};
    std::set<PriceLevel> bidAdditions{};
    std::set<PriceLevel> askUpdates{};
    std::set<PriceLevel> bidUpdates{};

    auto nthAsk = [this]() {
      auto result = lastAsk_;

      return ++result;
    };

    auto nthBid = [this]() {
      auto result = lastBid_;

      return --result;
    };

    for (const auto& askRemoval : removals.asks) {
      if (asks_.empty()) continue;

      auto found = asks_.find(askRemoval);

      if (levelsNumber_ == 0) {
        askRemovals.insert(askRemoval);
        asks_.erase(found);
        lastAsk_ = asks_.end();

        continue;
      }

      auto removed = false;

      // Determine what will be the removal given the number of price levels.
      if (asks_.size() <= levelsNumber_ || askRemoval.price < nthAsk()->price) {
        askRemovals.insert(askRemoval);
        removed = true;
      }

      // Determine what will be the shift in price levels after removal.
      if (removed && asks_.size() > levelsNumber_) {
        askAdditions.insert(*nthAsk());
      }

      // Determine the adjusted last ask price (NaN -- asks.end)
      AskPriceLevel newLastAskPL{};

      if (removed) { // askRemoval.price <= lastAsk.price
        if (nthAsk() != asks_.end()) { // there is another ask after last
          newLastAskPL.price = nthAsk()->price;
        } else {
          if (askRemoval.price < lastAsk_->price) {
            newLastAskPL.price = lastAsk_->price;
          } else if (lastAsk_ != asks_.begin()) {
            newLastAskPL.price = (--lastAsk_)->price;
          }
        }
      } else {
        newLastAskPL.price = lastAsk_->price;
      }

      asks_.erase(found);

      if (std::isnan(newLastAskPL.price)) {
        lastAsk_ = asks_.end();
      } else {
        lastAsk_ = asks_.find(newLastAskPL);
      }
    }

    for (const auto& askAddition : additions.asks) {
      if (levelsNumber_ == 0) {
        askAdditions.insert(askAddition);
        asks_.insert(askAddition);
        lastAsk_ = asks_.end();

        continue;
      }

      auto added = false;

      // We determine what will be the addition of the price level, taking into account the possible quantity.
      if (asks_.size() < levelsNumber_ || askAddition.price < lastAsk_->price) {
        askAdditions.insert(askAddition);
        added = true;
      }

      // We determine what will be the shift after adding
      if (added && asks_.size() >= levelsNumber_) {
        const auto& toRemove = *lastAsk_;

        // We take into account the possibility that the previously added price level will be deleted.
        if (askAdditions.count(toRemove) > 0) {
          askAdditions.erase(toRemove);
        } else {
          askRemovals.insert(toRemove);
        }
      }

      // Determine the adjusted last ask price (NaN -- asks.end)
      AskPriceLevel newLastAskPL = *lastAsk_;

      if (added) {
        if (lastAsk_ == asks_.end()) { // empty
          newLastAskPL.price = askAddition.price;
        } else {
          if (askAddition.price < lastAsk_->price) { // add before the last
            if (asks_.size() >= levelsNumber_) {
              newLastAskPL.price = askAddition.price;

              if (lastAsk_ != asks_.begin()) {
                auto prev = lastAsk_;
                --prev;

                if (askAddition.price < prev->price) {
                  newLastAskPL.price = prev->price;
                }
              }
            }
          } else { // add after the last
            newLastAskPL.price = askAddition.price;
          }
        }
      }

      asks_.insert(askAddition);
    }

    for (const auto& askUpdate : updates.asks) {
      if (levelsNumber_ == 0 || asks_.count(askUpdate.price) > 0) {
        askUpdates.insert(askUpdate);
      }

      asks_.erase(askUpdate.price);
      asks_.insert(askUpdate);
    }

    for (const auto& bidRemoval : removals.bids) {
      if (bids_.empty()) continue;

      // Determine what will be the removal given the number of price levels.
      if (levelsNumber_ == 0 || bids_.size() <= levelsNumber_ ||
          bidRemoval.price > bids_.get<1>()[bids_.size() - 1 - levelsNumber_].price) {
        bidRemovals.insert(bidRemoval);
      }

      // Determine what will be the shift in price levels after removal.
      if (levelsNumber_ != 0 && bids_.size() > levelsNumber_ &&
          bidRemoval.price > bids_.get<1>()[bids_.size() - 1 - levelsNumber_].price) {
        bidAdditions.insert(bids_.get<1>()[bids_.size() - 1 - levelsNumber_]);
      }

      // remove price level by price
      bids_.erase(bidRemoval.price);
    }

    for (const auto& bidAddition : additions.bids) {
      // We determine what will be the addition of the price level, taking into account the possible quantity.
      if (levelsNumber_ == 0 || bids_.size() < levelsNumber_ ||
          bidAddition.price > bids_.get<1>()[bids_.size() - levelsNumber_].price) {
        bidAdditions.insert(bidAddition);
      }

      // We determine what will be the shift after adding
      if (levelsNumber_ != 0 && bids_.size() >= levelsNumber_ &&
          bidAddition.price > bids_.get<1>()[bids_.size() - levelsNumber_].price) {
        const auto& toRemove = bids_.get<1>()[bids_.size() - levelsNumber_];

        // We take into account the possibility that the previously added price level will be deleted.
        if (bidAdditions.count(toRemove) > 0) {
          bidAdditions.erase(toRemove);
        } else {
          bidRemovals.insert(toRemove);
        }
      }

      bids_.insert(bidAddition);
    }

    for (const auto& bidUpdate : updates.bids) {
      if (levelsNumber_ == 0 || bids_.count(bidUpdate.price) > 0) {
        bidUpdates.insert(bidUpdate);
      }

      bids_.erase(bidUpdate.price);
      bids_.insert(bidUpdate);
    }

    return {PriceLevelChanges{std::vector<PriceLevel>{askAdditions.begin(), askAdditions.end()},
                              std::vector<PriceLevel>{bidAdditions.rbegin(), bidAdditions.rend()}},
            PriceLevelChanges{std::vector<PriceLevel>{askUpdates.begin(), askUpdates.end()},
                              std::vector<PriceLevel>{bidUpdates.rbegin(), bidUpdates.rend()}},
            PriceLevelChanges{std::vector<PriceLevel>{askRemovals.begin(), askRemovals.end()},
                              std::vector<PriceLevel>{bidRemovals.rbegin(), bidRemovals.rend()}}};
  }

  [[nodiscard]] std::vector<PriceLevel> getAsks() const {
    return {asks_.get<1>().begin(),
            (levelsNumber_ == 0 || asks_.get<1>().size() <= levelsNumber_) ? asks_.get<1>().end() : asks_.get<1>().begin() + levelsNumber_};
  }

  [[nodiscard]] std::vector<PriceLevel> getBids() const {
    return {bids_.get<1>().rbegin(),
            (levelsNumber_ == 0 || bids_.get<1>().size() <= levelsNumber_) ? bids_.get<1>().rend() : bids_.get<1>().rbegin() + levelsNumber_};
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
        onNewBook_(PriceLevelChanges{getAsks(), getBids()});
      }
    } else {
      if (onIncrementalChange_) {
        onIncrementalChange_(resultingChangesSet);
      }

      if (onBookUpdate_) {
        onBookUpdate_(PriceLevelChanges{getAsks(), getBids()});
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