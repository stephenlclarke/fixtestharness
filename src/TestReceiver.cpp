#include "fix/TestReceiver.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "fix/FixMessage.hpp"

namespace fix {

TestReceiver::TestReceiver(std::string rx_path, uint16_t port)
    : responses_(ReadMessagesFromFile(rx_path)), port_(port) {}

bool TestReceiver::BindAndListen(int &server_sock) {
  server_sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0) {
    return false;
  }

  int opt = 1;
  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port_);

  if (bind(server_sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) <
      0) {
    ::close(server_sock);
    return false;
  }

  if (listen(server_sock, 1) < 0) {
    ::close(server_sock);
    return false;
  }

  return true;
}

bool TestReceiver::SendLine(int sock, const std::string &line) const {
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

bool TestReceiver::HandleClient(int client_sock) {
  std::string buffer;
  char chunk[512];
  size_t response_index = 0;

  while (true) {
    ssize_t received = ::recv(client_sock, chunk, sizeof(chunk), 0);
    if (received <= 0) {
      break;
    }
    buffer.append(chunk, static_cast<size_t>(received));
    std::size_t pos;
    while ((pos = buffer.find('\n')) != std::string::npos) {
      std::string line = buffer.substr(0, pos);
      buffer.erase(0, pos + 1);
      if (response_index < responses_.size()) {
        if (!SendLine(client_sock, responses_[response_index])) {
          return false;
        }
        ++response_index;
      }
    }
  }

  return true;
}

bool TestReceiver::Run() {
  int server_sock = -1;
  if (!BindAndListen(server_sock)) {
    return false;
  }

  sockaddr_in client_addr {};
  socklen_t client_len = sizeof(client_addr);
  int client_sock =
      ::accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr),
               &client_len);
  if (client_sock < 0) {
    ::close(server_sock);
    return false;
  }

  bool result = HandleClient(client_sock);
  ::close(client_sock);
  ::close(server_sock);
  return result;
}

}  // namespace fix
