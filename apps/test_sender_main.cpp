#include <iostream>
#include <string>

#include "fix/TestSender.hpp"

int main(int argc, char* argv[]) {
  if (argc < 5) {
    std::cerr << "Usage: test_sender <orig_tx.fix> <orig_rx.fix> <host> <port>\n";
    return 1;
  }

  const std::string tx_file = argv[1];
  const std::string rx_file = argv[2];
  const std::string host = argv[3];
  const uint16_t port = static_cast<uint16_t>(std::stoi(argv[4]));

  fix::TestSender sender(tx_file, rx_file, host, port);
  if (!sender.Run()) {
    for (const auto& diff : sender.differences()) {
      std::cerr << diff << "\n";
    }
    return 1;
  }

  std::cout << "All messages validated successfully.\n";
  return 0;
}
