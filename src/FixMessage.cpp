#include "fix/FixMessage.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>

namespace fix {

namespace {

std::string TrimTrailingNewlines(const std::string &input) {
  std::string out = input;
  while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) {
    out.pop_back();
  }
  return out;
}

std::string Trim(const std::string &s) {
  size_t start = 0;
  while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
    ++start;
  }
  size_t end = s.size();
  while (end > start &&
         std::isspace(static_cast<unsigned char>(s[end - 1]))) {
    --end;
  }
  return s.substr(start, end - start);
}

char DetectDelimiter(const std::string &message) {
  if (message.find('\x01') != std::string::npos) {
    return '\x01';
  }
  if (message.find('|') != std::string::npos) {
    return '|';
  }
  if (message.find(';') != std::string::npos) {
    return ';';
  }
  return ' ';
}

std::unordered_map<std::string, std::multiset<std::string>> BuildFieldMap(
    const FixMessage &msg,
    const std::unordered_set<std::string> &ignored_tags) {
  std::unordered_map<std::string, std::multiset<std::string>> result;
  for (const auto &f : msg.fields()) {
    if (ignored_tags.find(f.tag) != ignored_tags.end()) {
      continue;
    }
    result[f.tag].insert(f.value);
  }
  return result;
}

}  // namespace

FixMessage FixMessage::Parse(const std::string &raw) {
  FixMessage msg;
  msg.raw_ = TrimTrailingNewlines(raw);
  if (msg.raw_.empty()) {
    return msg;
  }

  char delim = DetectDelimiter(msg.raw_);
  size_t start = 0;
  while (start <= msg.raw_.size()) {
    size_t end = msg.raw_.find(delim, start);
    std::string field =
        msg.raw_.substr(start,
                        end == std::string::npos ? std::string::npos
                                                 : end - start);
    field = Trim(field);
    if (!field.empty()) {
      size_t eq = field.find('=');
      if (eq != std::string::npos && eq + 1 < field.size()) {
        std::string tag = field.substr(0, eq);
        std::string value = field.substr(eq + 1);
        msg.fields_.push_back({tag, value});
      }
    }
    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }

  return msg;
}

std::string FixMessage::Get(const std::string &tag) const {
  for (const auto &f : fields_) {
    if (f.tag == tag) {
      return f.value;
    }
  }
  return "";
}

bool Equivalent(const FixMessage &a, const FixMessage &b,
                const std::unordered_set<std::string> &ignored_tags) {
  auto map_a = BuildFieldMap(a, ignored_tags);
  auto map_b = BuildFieldMap(b, ignored_tags);
  return map_a == map_b;
}

std::vector<std::string> ReadMessagesFromFile(const std::string &path) {
  std::ifstream in(path);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (!Trim(line).empty()) {
      lines.push_back(line);
    }
  }
  return lines;
}

void WriteMessagesToFile(const std::vector<std::string> &messages,
                         const std::string &path) {
  std::ofstream out(path, std::ios::trunc);
  for (const auto &m : messages) {
    out << m << '\n';
  }
}

}  // namespace fix
