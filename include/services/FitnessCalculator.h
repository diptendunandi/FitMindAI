#pragma once

#include <tuple>
#include "models/UserProfile.h"

namespace fitmind {

struct CalorieMacroRecommendation {
  double bmi = 0.0;
  double dailyCalories = 0.0;
  // grams per day
  double proteinG = 0.0;
  double carbsG = 0.0;
  double fatsG = 0.0;
};

class FitnessCalculator {
public:
  [[nodiscard]] double calculateBMI(double heightCm, double weightKg) const;

  [[nodiscard]] CalorieMacroRecommendation recommendCaloriesAndMacros(const UserProfile& profile) const;

private:
  [[nodiscard]] double activityMultiplier(int activityLevel) const;
  [[nodiscard]] double goalCalorieAdjustment(Goal goal) const;
  [[nodiscard]] double proteinMultiplier(Goal goal, const UserProfile& profile) const;
  [[nodiscard]] double fatsRatio(Goal goal) const;
};

} // namespace fitmind

