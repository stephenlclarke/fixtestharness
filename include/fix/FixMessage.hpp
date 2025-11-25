/**
 * Basic FIX message parsing and comparison utilities.
 */
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fix {

struct FixField {
  std::string tag;
  std::string value;
};

class FixMessage {
 public:
  static FixMessage Parse(const std::string &raw);

  const std::vector<FixField> &fields() const { return fields_; }
  const std::string &raw() const { return raw_; }

  std::string Get(const std::string &tag) const;
  bool empty() const { return fields_.empty(); }

 private:
  std::string raw_;
  std::vector<FixField> fields_;
};

bool Equivalent(const FixMessage &a, const FixMessage &b,
                const std::unordered_set<std::string> &ignored_tags);

std::vector<std::string> ReadMessagesFromFile(const std::string &path);
void WriteMessagesToFile(const std::vector<std::string> &messages,
                         const std::string &path);

}  // namespace fix
