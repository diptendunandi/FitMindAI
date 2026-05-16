#pragma once

#include <string>
#include "models/UserProfile.h"
#include "services/FitnessCalculator.h"

namespace fitmind {

struct RecommendationBundle {
  CalorieMacroRecommendation macros;
  std::string workoutSplit;
  std::string recoveryAdvice;
  std::string hydrationAdvice;
  std::string sleepAdvice;
  std::string dietAdvice;
};

class RecommendationEngine {
public:
  [[nodiscard]] RecommendationBundle generate(const UserProfile& profile) const;

private:
  [[nodiscard]] std::string workoutSplitFor(const UserProfile& profile) const;
  [[nodiscard]] std::string recoveryFor(const UserProfile& profile) const;
  [[nodiscard]] std::string hydrationFor(const UserProfile& profile) const;
  [[nodiscard]] std::string sleepFor(const UserProfile& profile) const;
  [[nodiscard]] std::string dietFor(const UserProfile& profile, const CalorieMacroRecommendation& m) const;

  [[nodiscard]] std::string bmiAdvice(double bmi) const;

  FitnessCalculator calculator_;
};

} // namespace fitmind

