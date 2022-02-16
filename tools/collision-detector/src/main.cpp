#include <DXFeed.h>
#include <EventData.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <StringConverter.hpp>

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
#include <fstream>

dxf_ulong_t dx_symbol_name_hasher(dxf_const_string_t symbol_name) {
  return static_cast<dxf_ulong_t>(std::hash<std::wstring>{}(symbol_name));
}

#define SNAPSHOT_KEY_SOURCE_MASK 0xFFFFFFu

dxf_ulong_t dx_new_snapshot_key(dx_record_info_id_t record_info_id, dxf_const_string_t symbol,
                                dxf_const_string_t order_source) {
  dxf_ulong_t symbol_hash = dx_symbol_name_hasher(symbol);
  dxf_ulong_t order_source_hash = (order_source == nullptr ? 0u : dx_symbol_name_hasher(order_source));
  return ((dxf_ulong_t)record_info_id << 56u) |
    ((dxf_ulong_t)symbol_hash << 24u) |
    (order_source_hash & SNAPSHOT_KEY_SOURCE_MASK);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage:\n  collision-detector <ipf-file-path>\n\n";

    return 0;
  }

  std::string ipfFile = argv[1];

  std::unordered_map<dxf_ulong_t, std::vector<std::string>> stats{};

  std::ifstream f{ipfFile};

  auto counter = 0ULL;

  for (std::string line; std::getline(f, line); ) {
    if (line.empty() || line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

    auto found = line.find_first_of(',');

    if (found == std::string::npos) continue;

    auto start = found + 1;

    if (start == line.size()) continue;

    found = line.find_first_of(",\n\r", start);

    auto end = (found == std::string::npos) ? line.size() - 1 : found - 1;

    counter++;
    auto symbol = line.substr(start, end - start + 1);
    auto wSymbol = dxf::StringConverter::utf8ToWString(symbol);
    auto key = dx_new_snapshot_key(dx_rid_candle, wSymbol.c_str(), nullptr);

    if (stats.contains(key)) {
      stats[key].emplace_back(symbol);
    } else {
      stats[key] = {symbol};
    }
  }

  std::cout << counter << "\n\n";

  for (auto [k, v] : stats) {
    if (v.size() > 1) {
      std::cout << k << ":\n  ";

      for (const auto& s : v) {
        std::cout << s << ",";
      }

      std::cout << "\n";
    }
  }
}