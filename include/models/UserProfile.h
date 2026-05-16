#pragma once

#include <string>

namespace fitmind {

enum class Gender { Female, Male, Other };

enum class Goal { FatLoss, MuscleGain, Maintenance };

enum class DietPreference { Omnivore, Vegetarian, Vegan };

struct UserProfile {
  std::string name;
  int age = 0;
  Gender gender = Gender::Other;
  double heightCm = 0.0;
  double weightKg = 0.0;
  Goal goal = Goal::Maintenance;

  // Fitness inputs
  int workoutFrequencyPerWeek = 3; // 1..14 recommended
  // 1 = sedentary, 2 = light, 3 = moderate, 4 = active, 5 = very active
  int activityLevel = 3;
  double sleepHoursPerNight = 7.0;
  DietPreference dietPreference = DietPreference::Omnivore;

  // Serialization helpers
  [[nodiscard]] std::string toString() const;
};

} // namespace fitmind

