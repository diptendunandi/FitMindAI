#include "services/FitnessCalculator.h"

#include <algorithm>
#include <cmath>

namespace fitmind {

double FitnessCalculator::calculateBMI(double heightCm, double weightKg) const {
  if (heightCm <= 0.0) return 0.0;
  const double h = heightCm / 100.0;
  return weightKg / (h * h);
}

double FitnessCalculator::activityMultiplier(int activityLevel) const {
  switch (activityLevel) {
    case 1: return 1.2; // sedentary
    case 2: return 1.375; // light
    case 3: return 1.55; // moderate
    case 4: return 1.725; // active
    case 5: return 1.9; // very active
    default: return 1.55;
  }
}

double FitnessCalculator::goalCalorieAdjustment(Goal goal) const {
  switch (goal) {
    case Goal::FatLoss: return -0.15; // ~15% deficit
    case Goal::MuscleGain: return 0.12; // ~12% surplus
    case Goal::Maintenance: return 0.0;
  }
  return 0.0;
}

double FitnessCalculator::proteinMultiplier(Goal goal, const UserProfile& profile) const {
  // grams per kg bodyweight
  (void)profile;
  switch (goal) {
    case Goal::FatLoss: return 2.0;
    case Goal::MuscleGain: return 2.2;
    case Goal::Maintenance: return 1.8;
  }
  return 1.8;
}

double FitnessCalculator::fatsRatio(Goal goal) const {
  // Fats as % of calories.
  switch (goal) {
    case Goal::FatLoss: return 0.25;
    case Goal::MuscleGain: return 0.28;
    case Goal::Maintenance: return 0.27;
  }
  return 0.27;
}

CalorieMacroRecommendation FitnessCalculator::recommendCaloriesAndMacros(const UserProfile& profile) const {
  CalorieMacroRecommendation rec;
  rec.bmi = calculateBMI(profile.heightCm, profile.weightKg);

  // Mifflin-St Jeor BMR
  const double weight = profile.weightKg;
  const double height = profile.heightCm;
  const int age = profile.age;

  const double genderOffset = (profile.gender == Gender::Male) ? 5.0 : (profile.gender == Gender::Female ? -161.0 : -78.0);

  double bmr = 10.0 * weight + 6.25 * height - 5.0 * age + genderOffset;
  if (bmr < 1200.0) bmr = 1200.0;

  double tdee = bmr * activityMultiplier(profile.activityLevel);
  const double adj = goalCalorieAdjustment(profile.goal);
  double targetCalories = tdee * (1.0 + adj);

  // Clamp to avoid extreme values from bad input.
  targetCalories = std::clamp(targetCalories, 1200.0, 4500.0);
  rec.dailyCalories = targetCalories;

  const double proteinG = proteinMultiplier(profile.goal, profile) * profile.weightKg;
  rec.proteinG = std::clamp(proteinG, 80.0, 240.0);

  // Fats
  const double fatsKcal = targetCalories * fatsRatio(profile.goal);
  rec.fatsG = std::clamp(fatsKcal / 9.0, 45.0, 150.0);

  // Carbs: remainder
  const double proteinKcal = rec.proteinG * 4.0;
  const double carbsKcal = targetCalories - proteinKcal - (rec.fatsG * 9.0);
  const double carbsG = carbsKcal / 4.0;
  rec.carbsG = std::clamp(carbsG, 80.0, 500.0);

  return rec;
}

} // namespace fitmind

