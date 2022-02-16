#include <PriceLevelBook.hpp>

#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 5) {
    std::cout << "Usage:\n  plb-tester <endpoint> <symbol> <source> <number of levels>\n\n";

    return 0;
  }

  dxf_connection_t connection = nullptr;
  dxf_create_connection("demo.dxfeed.com:7300", nullptr, nullptr, nullptr, nullptr, nullptr, &connection);
  auto plb = dxf::PriceLevelBook::create(connection, "AAPL", "AGGREGATE_ASK", 10);

  std::cin.get();
  dxf_close_connection(connection);
}