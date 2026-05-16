#pragma once

#include <string>

#include "models/UserProfile.h"
#include "services/RecommendationEngine.h"
#include "services/ProgressTracker.h"

namespace fitmind {

class MenuSystem {
public:
  explicit MenuSystem(std::string progressStoragePath = "data/progress.json");

  int run();

private:
  UserProfile profile_;
  bool hasProfile_ = false;

  RecommendationEngine engine_;
  ProgressTracker tracker_;

  void showHeader() const;
  void menuLoop();

  void handleCreateOrLoadProfile();
  void handleRecommendations();
  void handleAddWeeklyProgress();
  void handleViewProgress();

  void readProfileFromUser();
  void loadProfileFromSample();

  [[nodiscard]] static Gender parseGender(const std::string& s);
  [[nodiscard]] static Goal parseGoal(const std::string& s);
  [[nodiscard]] static DietPreference parseDiet(const std::string& s);
};

} // namespace fitmind

