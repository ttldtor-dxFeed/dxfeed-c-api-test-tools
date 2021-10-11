#include <DXFeed.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>

int main() {
  std::atomic<bool> disconnected = false;
  dxf_connection_t con = nullptr;

  dxf_load_config_from_string("network.heartbeatTimeout = 16\nnetwork.reestablishConnections = false\n");

  auto result = dxf_create_connection(
    "demo.dxfeed.com:7300", [](dxf_connection_t connection, void *data) {
      std::cerr << "Disconnected\n";
      *(static_cast<std::atomic<bool> *>(data)) = true;
    }, nullptr,
    nullptr, nullptr, static_cast<void *>(&disconnected), &con);

  if (result == DXF_FAILURE) {
    return -1;
  }

  dxf_snapshot_t snap = nullptr;
  result = dxf_create_order_snapshot(con, L"IBM", "NTV", 1, &snap);

  if (result == DXF_FAILURE) {
    dxf_close_connection(con);

    return -1;
  }

  result = dxf_attach_snapshot_listener(
    snap,
    [](auto snapshotData, void *) {
      std::cout << snapshotData->records_count << std::endl;
    },
    nullptr);

  if (result == DXF_FAILURE) {
    dxf_close_snapshot(snap);
    dxf_close_connection(con);

    return -1;
  }

  using namespace std::chrono_literals;

  while (true) {
    if (disconnected) {
      using namespace std::chrono_literals;
      std::cerr << "Sleeping\n";
      std::this_thread::sleep_for(10s);
      std::cerr << "Closing connection\n";
      dxf_close_connection(con);
      std::cerr << "Closed\n";

      break;
    }

    std::this_thread::sleep_for(100ms);
  }

  return 0;
}