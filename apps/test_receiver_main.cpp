#include <iostream>

#include "fix/TestReceiver.hpp"

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: test_receiver <orig_rx.fix> <port>\n";
    return 1;
  }

  const std::string rx_file = argv[1];
  const uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

  fix::TestReceiver receiver(rx_file, port);
  if (!receiver.Run()) {
    std::cerr << "Receiver encountered an error.\n";
    return 1;
  }

  std::cout << "Receiver completed.\n";
  return 0;
}
