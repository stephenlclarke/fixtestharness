/**
 * Reads a FIX log, extracts session messages, and writes per-direction files.
 */
#pragma once

#include <string>
#include <vector>

namespace fix {

struct SessionMessages {
  std::vector<std::string> sent;
  std::vector<std::string> received;
};

class FixLogProcessor {
 public:
  static SessionMessages ExtractSessionMessages(const std::string &log_file,
                                                const std::string &sender_comp_id,
                                                const std::string &target_comp_id);

  static void WriteSessionFiles(const SessionMessages &session,
                                const std::string &output_dir);
};

}  // namespace fix
