#pragma once

#include <string>
#include <vector>
#include <optional>

namespace fitmind {

// Minimal JSON utilities for the project's schema.
// Uses a lightweight approach (not a full JSON parser),
// designed specifically for the expected key/value structure.

struct JsonValue {
  std::string raw;

  [[nodiscard]] bool isString() const;
  [[nodiscard]] std::string stringValue() const; // removes quotes
  [[nodiscard]] std::optional<double> numberValue() const;
  [[nodiscard]] std::optional<bool> boolValue() const;
};

// Read a JSON object file into a raw map of keys -> raw values (string/number/bool/objects/arrays as raw).
// Returns empty map if file missing/empty.
std::vector<std::pair<std::string, JsonValue>> parseTopLevelObjectMembers(const std::string& json);

std::optional<std::string> getStringMember(const std::vector<std::pair<std::string, JsonValue>>& members,
                                           const std::string& key);

std::optional<double> getNumberMember(const std::vector<std::pair<std::string, JsonValue>>& members,
                                       const std::string& key);

std::optional<bool> getBoolMember(const std::vector<std::pair<std::string, JsonValue>>& members,
                                   const std::string& key);

std::string stringifyProgressJson(const std::string& name,
                                   double latestWeightKg,
                                   const std::vector<double>& weightHistoryKg,
                                   const std::vector<int>& workoutConsistencyDays,
                                   const std::vector<double>& weeklyCaloriesAvg);

// Reads the schema used by ProgressTracker.
struct ProgressData {
  std::string userName;
  double latestWeightKg = 0.0;
  std::vector<double> weightHistoryKg;
  std::vector<int> workoutConsistencyDays;
  std::vector<double> weeklyCaloriesAvg;
};

std::optional<ProgressData> parseProgressData(const std::string& json);

} // namespace fitmind

