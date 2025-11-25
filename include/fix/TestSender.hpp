/**
 * Sends a sequence of FIX messages and validates responses.
 */
#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace fix {

class TestSender {
 public:
  TestSender(std::string tx_path, std::string expected_rx_path,
             std::string host, uint16_t port);

  bool Run();
  const std::vector<std::string> &differences() const { return differences_; }

 private:
  bool SendLine(int sock, const std::string &line) const;
  bool ReceiveLine(int sock, std::string &line) const;
  bool ValidateResponse(const std::string &expected,
                        const std::string &actual) const;

  std::vector<std::string> tx_messages_;
  std::vector<std::string> expected_rx_messages_;
  std::string rx_buffer_;
  std::string host_;
  uint16_t port_;
  std::vector<std::string> differences_;
  std::unordered_set<std::string> ignored_tags_{"52", "60", "75", "10"};
};

}  // namespace fix
