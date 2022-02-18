#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1

#include <fmt/format.h>

#include <PriceLevelBook.hpp>
#include <iostream>
#include <utility>

int main(int argc, char *argv[]) {
  if (argc < 5) {
    std::cout << "Usage:\n  plb-tester <endpoint> <symbol> <source> <number of levels>\n\n";

    return 0;
  }

  auto endpoint = argv[1];
  auto symbol = argv[2];
  auto source = argv[3];
  auto numberOfLevels = std::stoull(argv[4]);

  dxf_connection_t connection = nullptr;
  dxf_create_connection(endpoint, nullptr, nullptr, nullptr, nullptr, nullptr, &connection);
  auto plb = dxf::PriceLevelBook::create(connection, symbol, source, numberOfLevels);
  plb->setOnNewBook([](const dxf::PriceLevelChanges &priceLevelChanges) {
    fmt::print("\n{:^77}\n", "The New Book");
    fmt::print("{:-^77}\n", "-");
    fmt::print("{:<18} {:<18} | {:<18} {:<18}\n", " Ask", " Size", " Bid", " Size");
    fmt::print("{:-^38}|{:-^38}\n", "-", "-");
    for (std::size_t i = 0; i < (std::max)(priceLevelChanges.asks.size(), priceLevelChanges.bids.size()); i++) {
      if (i < priceLevelChanges.asks.size()) {
        fmt::print("{:<18.6g} {:<18.6g} |", priceLevelChanges.asks[i].price, priceLevelChanges.asks[i].size);
      } else {
        fmt::print("{:^38}|", ' ');
      }

      if (i < priceLevelChanges.bids.size()) {
        fmt::print(" {:<18.6g} {:<18.6g}\n", priceLevelChanges.bids[i].price, priceLevelChanges.bids[i].size);
      } else {
        fmt::print("{:^38}\n", ' ');
      }
    }
  });

  plb->setOnBookUpdate([](const dxf::PriceLevelChanges &priceLevelChanges) {
    fmt::print("\n{:^77}\n", "The Book Update");
    fmt::print("{:-^77}\n", "-");
    fmt::print("{:<18} {:<18} | {:<18} {:<18}\n", " Ask", " Size", " Bid", " Size");
    fmt::print("{:-^38}|{:-^38}\n", "-", "-");
    for (std::size_t i = 0; i < (std::max)(priceLevelChanges.asks.size(), priceLevelChanges.bids.size()); i++) {
      if (i < priceLevelChanges.asks.size()) {
        fmt::print("{:<18.6g} {:<18.6g} |", priceLevelChanges.asks[i].price, priceLevelChanges.asks[i].size);
      } else {
        fmt::print("{:^38}|", ' ');
      }

      if (i < priceLevelChanges.bids.size()) {
        fmt::print(" {:<18.6g} {:<18.6g}\n", priceLevelChanges.bids[i].price, priceLevelChanges.bids[i].size);
      } else {
        fmt::print("{:^38}\n", ' ');
      }
    }
  });

  plb->setOnIncrementalChange([](const dxf::PriceLevelChangesSet &changesSet) {
    if (!changesSet.additions.asks.empty() || !changesSet.additions.bids.empty()) {
      fmt::print("\n{:^77}\n", "Additions");
      fmt::print("{:-^77}\n", "-");
      fmt::print("{:<18} {:<18} | {:<18} {:<18}\n", " Ask", " Size", " Bid", " Size");
      fmt::print("{:-^38}|{:-^38}\n", "-", "-");
      for (std::size_t i = 0; i < (std::max)(changesSet.additions.asks.size(), changesSet.additions.bids.size()); i++) {
        if (i < changesSet.additions.asks.size()) {
          fmt::print("{:<18.6g} {:<18.6g} |", changesSet.additions.asks[i].price, changesSet.additions.asks[i].size);
        } else {
          fmt::print("{:^38}|", ' ');
        }

        if (i < changesSet.additions.bids.size()) {
          fmt::print(" {:<18.6g} {:<18.6g}\n", changesSet.additions.bids[i].price, changesSet.additions.bids[i].size);
        } else {
          fmt::print("{:^38}\n", ' ');
        }
      }
    }

    if (!changesSet.updates.asks.empty() || !changesSet.updates.bids.empty()) {
      fmt::print("\n{:^77}\n", "Updates");
      fmt::print("{:-^77}\n", "-");
      fmt::print("{:<18} {:<18} | {:<18} {:<18}\n", " Ask", " Size", " Bid", " Size");
      fmt::print("{:-^38}|{:-^38}\n", "-", "-");
      for (std::size_t i = 0; i < (std::max)(changesSet.updates.asks.size(), changesSet.updates.bids.size()); i++) {
        if (i < changesSet.updates.asks.size()) {
          fmt::print("{:<18.6g} {:<18.6g} |", changesSet.updates.asks[i].price, changesSet.updates.asks[i].size);
        } else {
          fmt::print("{:^38}|", ' ');
        }

        if (i < changesSet.updates.bids.size()) {
          fmt::print(" {:<18.6g} {:<18.6g}\n", changesSet.updates.bids[i].price, changesSet.updates.bids[i].size);
        } else {
          fmt::print("{:^38}\n", ' ');
        }
      }
    }

    if (!changesSet.removals.asks.empty() || !changesSet.removals.bids.empty()) {
      fmt::print("\n{:^77}\n", "Removals");
      fmt::print("{:-^77}\n", "-");
      fmt::print("{:<18} {:<18} | {:<18} {:<18}\n", " Ask", " Size", " Bid", " Size");
      fmt::print("{:-^38}|{:-^38}\n", "-", "-");
      for (std::size_t i = 0; i < (std::max)(changesSet.removals.asks.size(), changesSet.removals.bids.size()); i++) {
        if (i < changesSet.removals.asks.size()) {
          fmt::print("{:<18.6g} {:<18.6g} |", changesSet.removals.asks[i].price, changesSet.removals.asks[i].size);
        } else {
          fmt::print("{:^38}|", ' ');
        }

        if (i < changesSet.removals.bids.size()) {
          fmt::print(" {:<18.6g} {:<18.6g}\n", changesSet.removals.bids[i].price, changesSet.removals.bids[i].size);
        } else {
          fmt::print("{:^38}\n", ' ');
        }
      }
    }

  });

  std::cin.get();
  dxf_close_connection(connection);
}