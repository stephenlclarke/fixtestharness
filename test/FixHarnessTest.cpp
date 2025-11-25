#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "fix/FixLogProcessor.hpp"
#include "fix/FixMessage.hpp"
#include "fix/TestReceiver.hpp"
#include "fix/TestSender.hpp"

namespace fs = std::filesystem;

namespace {

struct TestCase {
  std::string name;
  std::function<void()> fn;
};

std::vector<TestCase> &Registry() {
  static std::vector<TestCase> tests;
  return tests;
}

void Register(const std::string &name, std::function<void()> fn) {
  Registry().push_back({name, std::move(fn)});
}

void Assert(bool condition, const std::string &msg) {
  if (!condition) {
    throw std::runtime_error(msg);
  }
}

#define TEST(name)                              \
  void name();                                  \
  bool name##_registered = [] {                 \
    Register(#name, name);                      \
    return true;                                \
  }();                                          \
  void name()

uint16_t ReservePort() {
  int sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return 0;
  }
  sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;
  if (bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    ::close(sock);
    return 0;
  }
  socklen_t len = sizeof(addr);
  if (getsockname(sock, reinterpret_cast<sockaddr *>(&addr), &len) < 0) {
    ::close(sock);
    return 0;
  }
  uint16_t port = ntohs(addr.sin_port);
  ::close(sock);
  return port;
}

fs::path WriteTempFile(const fs::path &dir, const std::string &name,
                       const std::vector<std::string> &lines) {
  fs::create_directories(dir);
  fs::path path = dir / name;
  std::ofstream out(path);
  for (const auto &l : lines) {
    out << l << '\n';
  }
  return path;
}

TEST(ParseAndEquivalent) {
  std::string raw1 =
      "8=FIX.4.4|49=ALICE|56=BOB|35=A|52=20241125-17:00:00.123|10=123";
  std::string raw2 =
      "8=FIX.4.4\x01""49=ALICE\x01""56=BOB\x01""35=A\x01""52=20241125-18:00:00.999\x01""10=234";
  fix::FixMessage msg1 = fix::FixMessage::Parse(raw1);
  fix::FixMessage msg2 = fix::FixMessage::Parse(raw2);
  Assert(msg1.Get("49") == "ALICE", "SenderCompID not parsed");
  std::unordered_set<std::string> ignored{"52", "10"};
  Assert(fix::Equivalent(msg1, msg2, ignored),
         "Messages differing only by timestamps should match");
}

TEST(SessionFilter) {
  std::vector<std::string> lines = {
      "INFO 8=FIX.4.4|49=SENDER|56=TARGET|34=1|52=20241125-17:00:00.000|35=A|10=001",
      "INFO 8=FIX.4.4|49=TARGET|56=SENDER|34=1|52=20241125-17:00:00.100|35=A|10=002",
      "INFO 8=FIX.4.4|49=OTHER|56=SENDER|34=2|52=20241125-17:00:01.000|35=A|10=003"};

  fs::path tmp = fs::temp_directory_path() / "fixtestharness" / "session";
  fs::create_directories(tmp);
  fs::path log_file = tmp / "log.fix";
  std::ofstream out(log_file);
  for (const auto &l : lines) {
    out << l << '\n';
  }
  out.close();

  fix::SessionMessages session =
      fix::FixLogProcessor::ExtractSessionMessages(log_file.string(), "SENDER",
                                                   "TARGET");
  Assert(session.sent.size() == 1, "Expected one sent message");
  Assert(session.received.size() == 1, "Expected one received message");
  Assert(session.sent.front().find("35=A") != std::string::npos,
         "Sent message missing 35=A");
}

TEST(SenderAndReceiverRoundTrip) {
  const std::string tx_msg =
      "8=FIX.4.4|49=SENDER|56=TARGET|34=1|52=20241125-17:00:00.000|35=D|55=IBM|54=1|10=111";
  const std::string rx_msg =
      "8=FIX.4.4|49=TARGET|56=SENDER|34=1|52=20241125-17:00:01.000|35=8|39=0|60=20241125-17:00:01.000|10=222";

  fs::path tmp = fs::temp_directory_path() / "fixtestharness" / "integration";
  fs::create_directories(tmp);
  fs::path tx_file = WriteTempFile(tmp, "orig_tx.fix", {tx_msg});
  fs::path rx_file = WriteTempFile(tmp, "orig_rx.fix", {rx_msg});

  uint16_t port = ReservePort();
  Assert(port != 0, "Failed to reserve port");

  fix::TestReceiver receiver(rx_file.string(), port);
  std::thread server([&receiver]() { receiver.Run(); });
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  fix::TestSender sender(tx_file.string(), rx_file.string(), "127.0.0.1", port);
  bool ok = sender.Run();
  if (!ok) {
    for (const auto &d : sender.differences()) {
      std::cerr << d << "\n";
    }
  }
  Assert(ok, "Sender did not validate responses");

  server.join();
}

}  // namespace

int main() {
  int failed = 0;
  for (const auto &t : Registry()) {
    try {
      t.fn();
      std::cout << "[PASS] " << t.name << "\n";
    } catch (const std::exception &ex) {
      std::cout << "[FAIL] " << t.name << ": " << ex.what() << "\n";
      ++failed;
    }
  }
  if (failed) {
    std::cout << failed << " test(s) failed\n";
    return 1;
  }
  return 0;
}
