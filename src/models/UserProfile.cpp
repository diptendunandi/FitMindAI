#include "models/UserProfile.h"

#include <sstream>

namespace fitmind {

static std::string to_string(Gender g) {
  switch (g) {
    case Gender::Female: return "female";
    case Gender::Male: return "male";
    default: return "other";
  }
}

static std::string to_string(Goal g) {
  switch (g) {
    case Goal::FatLoss: return "fat_loss";
    case Goal::MuscleGain: return "muscle_gain";
    default: return "maintenance";
  }
}

static std::string to_string(DietPreference d) {
  switch (d) {
    case DietPreference::Vegan: return "vegan";
    case DietPreference::Vegetarian: return "vegetarian";
    default: return "omnivore";
  }
}

std::string UserProfile::toString() const {
  std::ostringstream oss;
  oss << "Name: " << name << "\n"
      << "Age: " << age << "\n"
      << "Gender: " << to_string(gender) << "\n"
      << "Height(cm): " << heightCm << "\n"
      << "Weight(kg): " << weightKg << "\n"
      << "Goal: " << to_string(goal) << "\n"
      << "Workout frequency/week: " << workoutFrequencyPerWeek << "\n"
      << "Activity level: " << activityLevel << "\n"
      << "Sleep hours/night: " << sleepHoursPerNight << "\n"
      << "Diet: " << to_string(dietPreference) << "\n";
  return oss.str();
}

} // namespace fitmind

