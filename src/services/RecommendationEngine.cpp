#include "services/RecommendationEngine.h"

#include <algorithm>
#include <cmath>

namespace fitmind {

static std::string goalToStr(Goal g) {
  switch (g) {
    case Goal::FatLoss: return "Fat Loss";
    case Goal::MuscleGain: return "Muscle Gain";
    default: return "Maintenance";
  }
}

std::string RecommendationEngine::bmiAdvice(double bmi) const {
  if (bmi <= 0.0) return "BMI unavailable (check height/weight inputs).";
  if (bmi < 18.5) return "BMI suggests underweight—aim for nutrient-dense calories and steady strength training.";
  if (bmi < 25.0) return "BMI is in the healthy range—focus on consistency and progressive overload.";
  if (bmi < 30.0) return "BMI suggests overweight—prioritize sustainable deficit, step count, and joint-friendly training.";
  return "BMI suggests obesity—use conservative intensity, recovery-first programming, and consult a professional if needed.";
}

std::string RecommendationEngine::workoutSplitFor(const UserProfile& profile) const {
  const int f = std::clamp(profile.workoutFrequencyPerWeek, 1, 14);

  if (f <= 2) {
    return "2-day full-body: (1) Squat/hinge + Push, (2) Hinge/leg accessory + Pull + Core. Emphasize compound lifts.";
  }
  if (f == 3) {
    return "3-day Upper/Lower split: Day 1 Upper (push focus), Day 2 Lower (squat+hinge), Day 3 Upper/Lower blend with pull/core.";
  }
  if (f == 4) {
    if (profile.goal == Goal::MuscleGain) {
      return "4-day Push/Pull/Legs/Arms (hybrid): Higher volume on compounds + dedicated arms once/week.";
    }
    return "4-day Upper/Lower + Arms: Keep deficit or maintenance recovery-friendly; moderate volume and strict technique.";
  }
  if (f <= 6) {
    return "5-6 day: PPL + Upper/Arms (or PPLR). Prioritize progression; add cardio/steps on recovery days.";
  }
  return "7+ day: Use intensity cycling (high/medium/low) and keep 1-2 true recovery days to avoid overuse.";
}

std::string RecommendationEngine::recoveryFor(const UserProfile& profile) const {
  const double sleep = profile.sleepHoursPerNight;
  const bool aggressiveBlocked = (profile.goal == Goal::FatLoss && sleep < 6.0);
  const bool muscleBlocked = (profile.goal == Goal::MuscleGain && sleep < 6.5);

  if (aggressiveBlocked) {
    return "Recovery-first fat-loss logic: sleep is under 6h, so avoid an aggressive deficit. Choose a smaller calorie gap, keep training RPE moderate, and add 1 extra rest day.";
  }

  if (muscleBlocked) {
    return "Muscle-gain recovery guardrail: sleep is under 6.5h, so prioritize technique + adequate rest over max-intensity sets. Add light accessory work instead.";
  }

  if (sleep < 7.0) {
    return "Recovery: target a consistent bedtime, aim to raise sleep toward ~7.5h. For training, limit failure sets and keep sessions efficient.";
  }

  return "Recovery: you’re in a strong sleep range—use progressive overload while keeping at least 1 lighter day each week.";
}

std::string RecommendationEngine::hydrationFor(const UserProfile& profile) const {
  // Rough hydration: 30-35 ml/kg/day; higher if active.
  double base = profile.weightKg * (profile.activityLevel >= 4 ? 0.035 : 0.03);
  base = std::clamp(base, 1500.0, 4500.0);

  int workouts = std::clamp(profile.workoutFrequencyPerWeek, 1, 14);
  double sweatBonus = workouts >= 5 ? 250.0 : 150.0;
  const double liters = (base + sweatBonus) / 1000.0;

  return "Hydration: aim for about " + std::to_string(static_cast<int>(std::round(liters * 10.0)) / 10.0) + " L/day. Split across the day; add extra around workouts.";
}

std::string RecommendationEngine::sleepFor(const UserProfile& profile) const {
  const double sleep = profile.sleepHoursPerNight;

  if (sleep < 6.0) {
    return "Sleep quality priority: below 6h. Focus on a wind-down routine (screens off 30-60m), consistent wake time, and keep evening caffeine minimal.";
  }
  if (sleep < 7.0) {
    return "Sleep: aim to push toward ~7-8h. If you can’t increase duration, improve consistency and reduce late-night intensity.";
  }
  return "Sleep looks good—protect it. Avoid late-night high-intensity workouts and maintain a predictable bedtime window.";
}

std::string RecommendationEngine::dietFor(const UserProfile& profile, const CalorieMacroRecommendation& m) const {
  const double protein = m.proteinG;

  std::string dietaryType;
  switch (profile.dietPreference) {
    case DietPreference::Vegan: dietaryType = "Plant-forward (vegan)"; break;
    case DietPreference::Vegetarian: dietaryType = "Vegetarian"; break;
    default: dietaryType = "Omnivore"; break;
  }

  std::string proteinLine;
  if (protein < 110.0) {
    proteinLine = "Your protein target is on the low side for a performance phase—consider adding an extra high-protein meal and choosing leaner protein sources.";
  } else {
    proteinLine = "Protein target is set for lean-mass retention/gain. Distribute it across 3-4 meals for better utilization.";
  }

  std::string goalLine;
  switch (profile.goal) {
    case Goal::FatLoss:
      goalLine = "For fat loss, keep carbs around training and prefer fiber-rich foods to improve satiety.";
      break;
    case Goal::MuscleGain:
      goalLine = "For muscle gain, carbs support training performance—prioritize carbohydrates earlier in the day or pre-workout.";
      break;
    default:
      goalLine = "For maintenance, keep calories near target and focus on consistency in training volume and sleep.";
      break;
  }

  return "Diet advice (" + dietaryType + "): " + proteinLine + " " + goalLine;
}

RecommendationBundle RecommendationEngine::generate(const UserProfile& profile) const {
  RecommendationBundle b;
  b.macros = calculator_.recommendCaloriesAndMacros(profile);

  b.workoutSplit = workoutSplitFor(profile);

  b.recoveryAdvice = recoveryFor(profile);

  b.hydrationAdvice = hydrationFor(profile);
  b.sleepAdvice = sleepFor(profile);
  b.dietAdvice = dietFor(profile, b.macros);

  // Append BMI advice for completeness
  b.recoveryAdvice += "\nBMI context: " + bmiAdvice(b.macros.bmi);

  return b;
}

} // namespace fitmind

