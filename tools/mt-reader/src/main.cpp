#include <DXFeed.h>
#include <EventData.h>

#include <atomic>
#include <chrono>
#include <codecvt>
#include <future>
#include <iostream>
#include <limits>
#include <locale>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <SimpleTimeAndSaleDataProvider.hpp>

int main(int argc, char *argv[]) {
  dxf_load_config_from_string("logger.level = \"debug\"\n");
  dxf_initialize_logger_v2("mt-reader.log", true, true, true, false);

  for (auto [s, v] : dxf::SimpleTimeAndSaleDataProvider::run(argv[1], {"/ESZ21:XCME", "/FESX211217:XEUR", "AAPL"}).get()) {
    std::cout << s << "[" << v.size() << "]\n";
  }

  auto f = dxf::SimpleTimeAndSaleDataProvider::run(argv[1], {"/ESZ21:XCME", "/FESX211217:XEUR", "AAPL"});
  auto f2 = dxf::SimpleTimeAndSaleDataProvider::run(argv[2], {"/ESZ21:XCME", "/FESX211217:XEUR", "AAPL"});

  for (auto [s, v] : f.get()) {
    std::cout << s << "[" << v.size() << "]\n";
  }

  for (auto [s, v] : f2.get()) {
    std::cout << s << "[" << v.size() << "]\n";
  }

  return 0;
}