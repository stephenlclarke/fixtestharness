#include "fix/FixLogProcessor.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "fix/FixMessage.hpp"

namespace fix {

namespace {

std::string ExtractFixPayload(const std::string &line) {
  auto pos = line.find("8=FIX");
  if (pos == std::string::npos) {
    return "";
  }
  return line.substr(pos);
}

}  // namespace

SessionMessages FixLogProcessor::ExtractSessionMessages(
    const std::string &log_file, const std::string &sender_comp_id,
    const std::string &target_comp_id) {
  SessionMessages session;
  std::ifstream in(log_file);
  if (!in.is_open()) {
    throw std::runtime_error("Unable to open log file: " + log_file);
  }

  std::string line;
  while (std::getline(in, line)) {
    const std::string payload = ExtractFixPayload(line);
    if (payload.empty()) {
      continue;
    }

    FixMessage msg = FixMessage::Parse(payload);
    const std::string sender = msg.Get("49");
    const std::string target = msg.Get("56");
    if (sender.empty() || target.empty()) {
      continue;
    }

    if (sender == sender_comp_id && target == target_comp_id) {
      session.sent.push_back(payload);
    } else if (sender == target_comp_id && target == sender_comp_id) {
      session.received.push_back(payload);
    }
  }

  return session;
}

void FixLogProcessor::WriteSessionFiles(const SessionMessages &session,
                                        const std::string &output_dir) {
  namespace fs = std::filesystem;
  fs::create_directories(output_dir);
  const fs::path base(output_dir);
  WriteMessagesToFile(session.received, (base / "orig_rx.fix").string());
  WriteMessagesToFile(session.sent, (base / "orig_tx.fix").string());
}

}  // namespace fix
