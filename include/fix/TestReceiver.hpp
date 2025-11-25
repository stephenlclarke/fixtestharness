/**
 * Simple FIX echo receiver that responds with pre-captured messages.
 */
#pragma once

#include <string>
#include <vector>

namespace fix {

class TestReceiver {
 public:
  TestReceiver(std::string rx_path, uint16_t port);
  bool Run();
  uint16_t port() const { return port_; }

 private:
  bool BindAndListen(int &server_sock);
  bool HandleClient(int client_sock);
  bool SendLine(int sock, const std::string &line) const;

  std::vector<std::string> responses_;
  uint16_t port_;
};

}  // namespace fix
