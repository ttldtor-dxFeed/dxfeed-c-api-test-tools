#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1

#include <DXFeed.h>
#include <EventData.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <fstream>

#include "StringConverter.hpp"

inline std::string formatTime(long long timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S") {
  return fmt::format(fmt::format("{{:{}}}", format), fmt::gmtime(static_cast<std::time_t>(timestamp)));
}

inline std::string formatTimestampWithMillis(long long timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S") {
  long long ms = timestamp % 1000;

  return fmt::format("{}.{:0>3}", formatTime(timestamp / 1000, format), ms);
}

inline std::string formatLocalTime(long long timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S") {
  return fmt::format(fmt::format("{{:{}}}", format), fmt::localtime(static_cast<std::time_t>(timestamp)));
}

inline std::string formatLocalTimestampWithMillis(long long timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S") {
  long long ms = timestamp % 1000;

  return fmt::format("{}.{:0>3}", formatLocalTime(timestamp / 1000, format), ms);
}

std::atomic<std::size_t> eventCounter = 0;
std::atomic<bool> check = true;
std::atomic<bool> stop = false;


int main(int argc, char* argv[]) {
  if (argc < 4) {
    std::cout << "Usage:\n  bench <endpoint> <event type> <symbol>\n\n";

    return 0;
  }

  auto eventTypeStringToMask = std::unordered_map<std::string, unsigned>{
    {"Trade", DXF_ET_TRADE},
    {"TRADE", DXF_ET_TRADE},
    {"Quote", DXF_ET_QUOTE},
    {"QUOTE", DXF_ET_QUOTE},
    {"Summary", DXF_ET_SUMMARY},
    {"SUMMARY", DXF_ET_SUMMARY},
    {"Profile", DXF_ET_PROFILE},
    {"PROFILE", DXF_ET_PROFILE},
    {"Order", DXF_ET_ORDER},
    {"ORDER", DXF_ET_ORDER},
    {"TimeAndSale", DXF_ET_TIME_AND_SALE},
    {"TIME_AND_SALE", DXF_ET_TIME_AND_SALE},
  };

  auto endpoint = argv[1];
  auto eventType = argv[2];
  auto symbol = argv[3];

  dxf_connection_t connection = nullptr;
  dxf_create_connection(endpoint, nullptr, nullptr, nullptr, nullptr, nullptr, &connection);
  dxf_subscription_t sub = nullptr;
  dxf_create_subscription(connection, static_cast<int>(eventTypeStringToMask[eventType]), &sub);
  dxf_attach_event_listener(
    sub,
    [](int eventType, dxf_const_string_t, const dxf_event_data_t* data, int dataCount, void*) {
      static long previousPrice = 0;

      if (eventType != DXF_ET_TIME_AND_SALE) return;

      auto timeAndSales = reinterpret_cast<const dxf_time_and_sale_t*>(data);

      for (std::size_t i = 0; i < dataCount; i++) {
        const auto& tns = timeAndSales[i];
        auto price = static_cast<long>(tns.price);

        if (previousPrice != 0) {
          if (price - previousPrice > 1) {
            check = false;
          }
        }

        eventCounter++;
        previousPrice = price;
      }
    },
    nullptr);

  auto wSymbol = dxf::StringConverter::utf8ToWString(symbol);
  dxf_add_symbol(sub, wSymbol.c_str());

  auto th = std::thread([] {
    using namespace std::chrono_literals;

    std::ofstream of{"bench.csv"};

    auto start = std::chrono::system_clock::now();

    while (!stop) {
      auto current = std::chrono::system_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count();

      if (elapsed >= 1000) {
        auto avg = static_cast<double>(eventCounter) * 1000.0 / static_cast<double>(elapsed);
        eventCounter = 0;

        auto nowString = formatLocalTimestampWithMillis(
          std::chrono::duration_cast<std::chrono::milliseconds>(current.time_since_epoch()).count(), "%H:%M:%S");
        fmt::print("{}\n", nowString);
        fmt::print("Current speed: {:0.0f} events per second\n", avg);
        fmt::print("Event check: {}\n", check);
        of << nowString << "," << fmt::format("{:0.0f}", avg) << std::endl;

        start = current;
      }

      std::this_thread::sleep_for(10ms);
    }
  });

  std::cin.get();
  dxf_close_connection(connection);
  stop = true;
  th.join();
}