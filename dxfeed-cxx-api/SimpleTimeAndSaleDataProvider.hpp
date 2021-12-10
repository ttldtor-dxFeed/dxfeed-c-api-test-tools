#pragma once

#include <DXFeed.h>
#include <unordered_map>
#include <future>
#include <thread>
#include <mutex>
#include "TimeAndSale.hpp"

namespace dxf {

struct SimpleTimeAndSaleDataProvider {
  using ResultType = std::unordered_map<std::string, std::vector<TimeAndSale>>;
  using ResultFutureType = std::future<ResultType>;

  SimpleTimeAndSaleDataProvider() = default;

  static ResultFutureType run(const std::string &address, const std::vector<std::string> &symbols, int timeout = 0) {
    return std::async(std::launch::async, [address, symbols, timeout]() {
      struct Impl {
        std::atomic<bool> disconnected_ = false;
        std::mutex eventsMutex_{};
        ResultType events_{};
        std::mutex cvMutex_{};
        std::condition_variable cv_{};
      } impl;

      dxf_connection_t con = nullptr;
      auto res = dxf_create_connection(
        address.c_str(),
        [](dxf_connection_t c, void *data) {
          static_cast<Impl *>(data)->disconnected_ = true;
          static_cast<Impl *>(data)->cv_.notify_one();
        },
        nullptr, nullptr, nullptr, static_cast<void *>(&impl), &con);

      if (res == DXF_FAILURE) {
        return impl.events_;
      }

      dxf_subscription_t sub = nullptr;
      res = dxf_create_subscription_timed(con, DXF_ET_TIME_AND_SALE, 0, &sub);

      if (res == DXF_FAILURE) {
        return impl.events_;
      }

      dxf_attach_event_listener(
        sub,
        [](int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData, int, void *userData) {
          if (eventType == DXF_ET_TIME_AND_SALE) {
            const auto *tns = reinterpret_cast<const dxf_time_and_sale_t *>(eventData);
            auto *implPtr = static_cast<Impl *>(userData);
            auto symbol = StringConverter::wStringToUtf8(std::wstring(symbolName));
            auto timeAndSale = TimeAndSale(symbol, *tns);

            std::lock_guard guard(implPtr->eventsMutex_);
            if (auto found = implPtr->events_.find(symbol); found != implPtr->events_.end()) {
              found->second.emplace_back(timeAndSale);
            } else {
              implPtr->events_[symbol] = {timeAndSale};
            }
          }
        },
        static_cast<void *>(&impl));

      std::vector<std::wstring> wSymbols(symbols.size());
      std::transform(symbols.begin(), symbols.end(), wSymbols.begin(),
                     [](auto s) { return StringConverter::utf8ToWString(s); });

      for (const auto &ws : wSymbols) {
        res = dxf_add_symbol(sub, ws.c_str());

        if (res == DXF_FAILURE) {
          return impl.events_;
        }
      }

      {
        std::unique_lock lk(impl.cvMutex_);
        if (timeout == 0) {
          impl.cv_.wait(lk, [&impl] {
            bool disconnected = impl.disconnected_;

            return disconnected;
          });
        } else {
          impl.cv_.wait_for(lk, std::chrono::milliseconds(timeout), [&impl] {
            bool disconnected = impl.disconnected_;

            return disconnected;
          });
        }
      }

      dxf_close_subscription(sub);
      dxf_close_connection(con);

      return impl.events_;
    });
  }
};

}