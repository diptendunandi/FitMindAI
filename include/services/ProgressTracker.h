#pragma once

#include <vector>
#include <string>
#include "models/UserProfile.h"
#include "services/FileManager.h"

namespace fitmind {

struct WeeklyProgressEntry {
  std::string weekLabel; // e.g. 2026-W20
  double weightKg = 0.0;
  int workoutConsistencyDays = 0; // 0..7
  double avgCalories = 0.0;
};

class ProgressTracker {
public:
  explicit ProgressTracker(std::string storagePath = "data/progress.json");

  [[nodiscard]] std::vector<WeeklyProgressEntry> load() const;
  void save(const std::vector<WeeklyProgressEntry>& entries) const;

  void addEntry(const UserProfile& profile,
                 const WeeklyProgressEntry& entry);

private:
  std::string storagePath_;
  FileManager fileManager_;

  [[nodiscard]] static std::string currentWeekLabel();
};

} // namespace fitmind

