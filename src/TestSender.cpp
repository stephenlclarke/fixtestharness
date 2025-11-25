#include "fix/TestSender.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include "fix/FixMessage.hpp"

namespace fix {

TestSender::TestSender(std::string tx_path, std::string expected_rx_path,
                       std::string host, uint16_t port)
    : tx_messages_(ReadMessagesFromFile(tx_path)),
      expected_rx_messages_(ReadMessagesFromFile(expected_rx_path)),
      host_(std::move(host)),
      port_(port) {}

bool TestSender::SendLine(int sock, const std::string &line) const {
  std::string payload = line;
  if (payload.empty() || payload.back() != '\n') {
    payload.push_back('\n');
  }
  const char *data = payload.data();
  size_t remaining = payload.size();
  while (remaining > 0) {
    ssize_t sent = ::send(sock, data, remaining, 0);
    if (sent <= 0) {
      return false;
    }
    data += sent;
    remaining -= static_cast<size_t>(sent);
  }
  return true;
}

bool TestSender::ReceiveLine(int sock, std::string &line) const {
  auto *mutable_buffer = const_cast<std::string *>(&rx_buffer_);
  while (true) {
    auto pos = mutable_buffer->find('\n');
    if (pos != std::string::npos) {
      line = mutable_buffer->substr(0, pos);
      mutable_buffer->erase(0, pos + 1);
      return true;
    }

    char chunk[512];
    ssize_t received = ::recv(sock, chunk, sizeof(chunk), 0);
    if (received <= 0) {
      return false;
    }
    mutable_buffer->append(chunk, static_cast<size_t>(received));
  }
}

bool TestSender::ValidateResponse(const std::string &expected,
                                  const std::string &actual) const {
  FixMessage expected_msg = FixMessage::Parse(expected);
  FixMessage actual_msg = FixMessage::Parse(actual);
  return Equivalent(expected_msg, actual_msg, ignored_tags_);
}

bool TestSender::Run() {
  differences_.clear();
  if (tx_messages_.empty()) {
    differences_.push_back("No TX messages to send.");
    return false;
  }
  if (expected_rx_messages_.size() < tx_messages_.size()) {
    differences_.push_back("Expected RX messages fewer than TX messages.");
    return false;
  }

  struct addrinfo hints {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  std::string port_str = std::to_string(port_);
  struct addrinfo *res = nullptr;
  if (getaddrinfo(host_.c_str(), port_str.c_str(), &hints, &res) != 0) {
    differences_.push_back("Failed to resolve host.");
    return false;
  }

  int sock = -1;
  for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
    sock = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sock < 0) {
      continue;
    }
    if (::connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
      break;
    }
    ::close(sock);
    sock = -1;
  }
  freeaddrinfo(res);

  if (sock < 0) {
    differences_.push_back("Failed to connect to receiver.");
    return false;
  }

  bool ok = true;
  for (size_t i = 0; i < tx_messages_.size(); ++i) {
    if (!SendLine(sock, tx_messages_[i])) {
      differences_.push_back("Failed to send TX message " + std::to_string(i));
      ok = false;
      break;
    }
    std::string response;
    if (!ReceiveLine(sock, response)) {
      differences_.push_back("Failed to read RX message " + std::to_string(i));
      ok = false;
      break;
    }
    if (!ValidateResponse(expected_rx_messages_[i], response)) {
      std::ostringstream oss;
      oss << "Mismatch for message " << i << ": expected [" << expected_rx_messages_[i]
          << "] got [" << response << "]";
      differences_.push_back(oss.str());
      ok = false;
    }
  }

  ::close(sock);
  return ok;
}

}  // namespace fix
