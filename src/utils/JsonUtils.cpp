#include "utils/JsonUtils.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

namespace fitmind {

static void skipWs(const std::string& s, size_t& i) {
  while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
}

static bool isQuotedStringAt(const std::string& s, size_t i) {
  return i < s.size() && s[i] == '"';
}

bool JsonValue::isString() const {
  std::string r = raw;
  if (r.size() < 2) return false;
  return r.front() == '"' && r.back() == '"';
}

std::string JsonValue::stringValue() const {
  if (!isString()) return {};
  if (raw.size() < 2) return {};
  return raw.substr(1, raw.size() - 2);
}

std::optional<double> JsonValue::numberValue() const {
  try {
    if (raw.empty()) return std::nullopt;
    char* end = nullptr;
    double v = std::strtod(raw.c_str(), &end);
    if (end == raw.c_str()) return std::nullopt;
    return v;
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<bool> JsonValue::boolValue() const {
  if (raw == "true") return true;
  if (raw == "false") return false;
  return std::nullopt;
}

// Parse minimal top-level object: { "k": v, "k2": v2 }
std::vector<std::pair<std::string, JsonValue>> parseTopLevelObjectMembers(const std::string& json) {
  std::vector<std::pair<std::string, JsonValue>> out;

  size_t i = 0;
  skipWs(json, i);
  if (i >= json.size() || json[i] != '{') return out;
  ++i;

  while (true) {
    skipWs(json, i);
    if (i >= json.size()) break;
    if (json[i] == '}') break;

    skipWs(json, i);
    if (!isQuotedStringAt(json, i)) return {};

    // key
    ++i; // skip opening quote
    size_t keyStart = i;
    while (i < json.size() && json[i] != '"') ++i;
    if (i >= json.size()) return {};
    std::string key = json.substr(keyStart, i - keyStart);
    ++i; // closing quote

    skipWs(json, i);
    if (i >= json.size() || json[i] != ':') return {};
    ++i;

    skipWs(json, i);
    // value: string/number/bool/object/array (raw)
    size_t valueStart = i;
    int depth = 0;
    bool inString = false;
    while (i < json.size()) {
      char c = json[i];
      if (inString) {
        if (c == '"' && (i == 0 || json[i - 1] != '\\')) {
          inString = false;
        }
        ++i;
        continue;
      }

      if (c == '"') {
        inString = true;
        ++i;
        continue;
      }

      if (c == '{' || c == '[') depth++;
      if (c == '}' || c == ']') depth--;

      if (depth == 0 && (c == ',' || c == '}')) break;
      ++i;
    }

    std::string rawVal = json.substr(valueStart, i - valueStart);
    // Trim
    size_t a = 0;
    while (a < rawVal.size() && std::isspace(static_cast<unsigned char>(rawVal[a]))) ++a;
    size_t b = rawVal.size();
    while (b > a && std::isspace(static_cast<unsigned char>(rawVal[b - 1]))) --b;
    rawVal = rawVal.substr(a, b - a);

    out.push_back({key, JsonValue{rawVal}});

    skipWs(json, i);
    if (i < json.size() && json[i] == ',') {
      ++i;
      continue;
    }
    if (i < json.size() && json[i] == '}') break;
  }

  return out;
}

std::optional<std::string> getStringMember(const std::vector<std::pair<std::string, JsonValue>>& members,
                                           const std::string& key) {
  for (const auto& kv : members) {
    if (kv.first == key) {
      if (auto v = kv.second.stringValue(); !v.empty()) return v;
    }
  }
  return std::nullopt;
}

std::optional<double> getNumberMember(const std::vector<std::pair<std::string, JsonValue>>& members,
                                       const std::string& key) {
  for (const auto& kv : members) {
    if (kv.first == key) {
      return kv.second.numberValue();
    }
  }
  return std::nullopt;
}

std::optional<bool> getBoolMember(const std::vector<std::pair<std::string, JsonValue>>& members,
                                   const std::string& key) {
  for (const auto& kv : members) {
    if (kv.first == key) {
      return kv.second.boolValue();
    }
  }
  return std::nullopt;
}

static std::string joinDoubleArray(const std::vector<double>& v) {
  std::ostringstream oss;
  oss << '[';
  for (size_t i = 0; i < v.size(); ++i) {
    if (i) oss << ',';
    oss << v[i];
  }
  oss << ']';
  return oss.str();
}

static std::string joinIntArray(const std::vector<int>& v) {
  std::ostringstream oss;
  oss << '[';
  for (size_t i = 0; i < v.size(); ++i) {
    if (i) oss << ',';
    oss << v[i];
  }
  oss << ']';
  return oss.str();
}

std::string stringifyProgressJson(const std::string& name,
                                   double latestWeightKg,
                                   const std::vector<double>& weightHistoryKg,
                                   const std::vector<int>& workoutConsistencyDays,
                                   const std::vector<double>& weeklyCaloriesAvg) {
  std::ostringstream oss;
  oss << "{\n";
  oss << "  \"userName\": \"" << name << "\",\n";
  oss << "  \"latestWeightKg\": " << latestWeightKg << ",\n";
  oss << "  \"weightHistoryKg\": " << joinDoubleArray(weightHistoryKg) << ",\n";
  oss << "  \"workoutConsistencyDays\": " << joinIntArray(workoutConsistencyDays) << ",\n";
  oss << "  \"weeklyCaloriesAvg\": " << joinDoubleArray(weeklyCaloriesAvg) << "\n";
  oss << "}\n";
  return oss.str();
}

static std::optional<std::string> extractMemberRaw(const std::string& json, const std::string& key) {
  auto members = parseTopLevelObjectMembers(json);
  for (const auto& kv : members) {
    if (kv.first == key) {
      return kv.second.raw;
    }
  }
  return std::nullopt;
}

static std::vector<double> parseDoubleArray(const std::string& raw) {
  std::vector<double> out;
  size_t i = 0;
  while (i < raw.size() && std::isspace(static_cast<unsigned char>(raw[i]))) ++i;
  if (i >= raw.size() || raw[i] != '[') return out;
  ++i;
  while (true) {
    while (i < raw.size() && std::isspace(static_cast<unsigned char>(raw[i]))) ++i;
    if (i < raw.size() && raw[i] == ']') break;

    size_t start = i;
    while (i < raw.size() && raw[i] != ',' && raw[i] != ']') ++i;
    std::string token = raw.substr(start, i - start);
    // trim
    size_t a = 0;
    while (a < token.size() && std::isspace(static_cast<unsigned char>(token[a]))) ++a;
    size_t b = token.size();
    while (b > a && std::isspace(static_cast<unsigned char>(token[b - 1]))) --b;
    token = token.substr(a, b - a);

    if (!token.empty()) {
      char* end = nullptr;
      double v = std::strtod(token.c_str(), &end);
      if (end != token.c_str()) out.push_back(v);
    }

    if (i < raw.size() && raw[i] == ',') ++i;
  }
  return out;
}

static std::vector<int> parseIntArray(const std::string& raw) {
  std::vector<int> out;
  size_t i = 0;
  while (i < raw.size() && std::isspace(static_cast<unsigned char>(raw[i]))) ++i;
  if (i >= raw.size() || raw[i] != '[') return out;
  ++i;
  while (true) {
    while (i < raw.size() && std::isspace(static_cast<unsigned char>(raw[i]))) ++i;
    if (i < raw.size() && raw[i] == ']') break;

    size_t start = i;
    while (i < raw.size() && raw[i] != ',' && raw[i] != ']') ++i;
    std::string token = raw.substr(start, i - start);

    size_t a = 0;
    while (a < token.size() && std::isspace(static_cast<unsigned char>(token[a]))) ++a;
    size_t b = token.size();
    while (b > a && std::isspace(static_cast<unsigned char>(token[b - 1]))) --b;
    token = token.substr(a, b - a);

    if (!token.empty()) {
      char* end = nullptr;
      long v = std::strtol(token.c_str(), &end, 10);
      if (end != token.c_str()) out.push_back(static_cast<int>(v));
    }

    if (i < raw.size() && raw[i] == ',') ++i;
  }
  return out;
}

std::optional<ProgressData> parseProgressData(const std::string& json) {
  auto members = parseTopLevelObjectMembers(json);
  if (members.empty()) return std::nullopt;

  ProgressData pd;
  auto itName = getStringMember(members, "userName");
  auto itLatest = getNumberMember(members, "latestWeightKg");

  if (!itName.has_value() || !itLatest.has_value()) return std::nullopt;
  pd.userName = *itName;
  pd.latestWeightKg = *itLatest;

  std::optional<std::string> rawWH = extractMemberRaw(json, "weightHistoryKg");
  std::optional<std::string> rawWC = extractMemberRaw(json, "workoutConsistencyDays");
  std::optional<std::string> rawCal = extractMemberRaw(json, "weeklyCaloriesAvg");
  if (!rawWH || !rawWC || !rawCal) return std::nullopt;

  pd.weightHistoryKg = parseDoubleArray(*rawWH);
  pd.workoutConsistencyDays = parseIntArray(*rawWC);
  pd.weeklyCaloriesAvg = parseDoubleArray(*rawCal);
  return pd;
}

} // namespace fitmind

