#include <filesystem>
#include <iostream>

#include "fix/FixLogProcessor.hpp"

int main(int argc, char* argv[]) {
  if (argc < 5) {
    std::cerr << "Usage: fix_harness <log_file> <senderCompID> <targetCompID> <output_dir>\n";
    return 1;
  }

  const std::string log_file = argv[1];
  const std::string sender_comp = argv[2];
  const std::string target_comp = argv[3];
  const std::string output_dir = argv[4];

  try {
    fix::SessionMessages session =
        fix::FixLogProcessor::ExtractSessionMessages(log_file, sender_comp,
                                                     target_comp);
    fix::FixLogProcessor::WriteSessionFiles(session, output_dir);
    std::cout << "Wrote " << session.sent.size() << " TX messages and "
              << session.received.size() << " RX messages to " << output_dir
              << "\n";
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
