#include "services/ProgressTracker.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "utils/JsonUtils.h"

namespace fitmind {

ProgressTracker::ProgressTracker(std::string storagePath)
  : storagePath_(std::move(storagePath)) {}

static std::vector<double> toDoubleHistory(const std::vector<WeeklyProgressEntry>& entries) {
  std::vector<double> out;
  out.reserve(entries.size());
  for (const auto& e : entries) out.push_back(e.weightKg);
  return out;
}

static std::vector<int> toIntConsistency(const std::vector<WeeklyProgressEntry>& entries) {
  std::vector<int> out;
  out.reserve(entries.size());
  for (const auto& e : entries) out.push_back(e.workoutConsistencyDays);
  return out;
}

static std::vector<double> toDoubleCalories(const std::vector<WeeklyProgressEntry>& entries) {
  std::vector<double> out;
  out.reserve(entries.size());
  for (const auto& e : entries) out.push_back(e.avgCalories);
  return out;
}

std::vector<WeeklyProgressEntry> ProgressTracker::load() const {
  if (!fileManager_.fileExists(storagePath_)) return {};
  const std::string content = fileManager_.readTextFile(storagePath_);
  if (content.empty()) return {};

  auto pdOpt = parseProgressData(content);
  if (!pdOpt.has_value()) return {};
  ProgressData pd = *pdOpt;

  // Reconstruct entries. Week labels are not stored in schema; use simple sequence labels.
  std::vector<WeeklyProgressEntry> entries;
  const size_t n = std::min({pd.weightHistoryKg.size(), pd.workoutConsistencyDays.size(), pd.weeklyCaloriesAvg.size()});
  entries.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    WeeklyProgressEntry e;
    e.weekLabel = "Week-" + std::to_string(i + 1);
    e.weightKg = pd.weightHistoryKg[i];
    e.workoutConsistencyDays = pd.workoutConsistencyDays[i];
    e.avgCalories = pd.weeklyCaloriesAvg[i];
    entries.push_back(e);
  }

  return entries;
}

void ProgressTracker::save(const std::vector<WeeklyProgressEntry>& entries) const {
  if (entries.empty()) {
    fileManager_.writeTextFile(storagePath_, "{}\n");
    return;
  }

  const auto last = entries.back();
  const auto weightHistory = toDoubleHistory(entries);
  const auto workoutConsistency = toIntConsistency(entries);
  const auto weeklyCalories = toDoubleCalories(entries);

  const std::string name = "User"; // schema stores userName; we keep it stable if unknown.
  // Better: store from first entry if available; keep simple.
  const std::string json = stringifyProgressJson(name, last.weightKg, weightHistory, workoutConsistency, weeklyCalories);
  fileManager_.writeTextFile(storagePath_, json);
}

std::string ProgressTracker::currentWeekLabel() {
  using clock = std::chrono::system_clock;
  const auto now = clock::now();
  std::time_t t = clock::to_time_t(now);
  std::tm local{};

  #ifdef _WIN32
  if (localtime_s(&local, &t) != 0) {
    std::tm* tmp = std::localtime(&t);
    if (tmp) local = *tmp;
  }
  #else
  {
    std::tm* tmp = std::localtime(&t);
    if (tmp) local = *tmp;
  }
  #endif

  // ISO-ish week label (approx). Keep it robust without <chrono> week algorithms.
  const int year = local.tm_year + 1900;
  const int yday = local.tm_yday; // 0..365
  const int week = (yday / 7) + 1;

  std::ostringstream oss;
  oss << year << "-W" << std::setw(2) << std::setfill('0') << week;
  return oss.str();
}

void ProgressTracker::addEntry(const UserProfile& profile, const WeeklyProgressEntry& entry) {
  auto entries = load();

  WeeklyProgressEntry e = entry;
  if (e.weekLabel.empty()) e.weekLabel = currentWeekLabel();

  // Clamp to sensible ranges
  e.weightKg = std::clamp(e.weightKg, 25.0, 400.0);
  e.workoutConsistencyDays = std::clamp(e.workoutConsistencyDays, 0, 7);
  e.avgCalories = std::clamp(e.avgCalories, 1000.0, 6000.0);

  entries.push_back(e);

  // Store name by updating file directly with correct schema.
  const auto last = entries.back();
  const auto weightHistory = toDoubleHistory(entries);
  const auto workoutConsistency = toIntConsistency(entries);
  const auto weeklyCalories = toDoubleCalories(entries);

  const std::string json = stringifyProgressJson(profile.name, last.weightKg, weightHistory, workoutConsistency, weeklyCalories);
  fileManager_.writeTextFile(storagePath_, json);
}

} // namespace fitmind

