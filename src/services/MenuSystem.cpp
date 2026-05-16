#include "services/MenuSystem.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "utils/InputUtils.h"
#include "utils/StringUtils.h"

namespace fitmind {

MenuSystem::MenuSystem(std::string progressStoragePath)
  : tracker_(std::move(progressStoragePath)) {}

void MenuSystem::showHeader() const {
  std::cout << "========================================\n";
  std::cout << "            FitMindAI CLI\n";
  std::cout << "========================================\n";
}

int MenuSystem::run() {
  menuLoop();
  return 0;
}

void MenuSystem::menuLoop() {
  showHeader();

  while (true) {
    std::cout << "\n\n";
    std::cout << "1) Create/Load Profile\n";
    std::cout << "2) Get Smart Recommendations\n";
    std::cout << "3) Add Weekly Progress\n";
    std::cout << "4) View Progress History\n";
    std::cout << "0) Exit\n";

    const int choice = readIntInRange("Select option: ", 0, 4);

    try {
      switch (choice) {
        case 0:
          std::cout << "Exiting.\n";
          return;
        case 1:
          handleCreateOrLoadProfile();
          break;
        case 2:
          handleRecommendations();
          break;
        case 3:
          handleAddWeeklyProgress();
          break;
        case 4:
          handleViewProgress();
          break;
      }
    } catch (const std::exception& ex) {
      std::cout << "Error: " << ex.what() << "\n";
    }
  }
}

void MenuSystem::handleCreateOrLoadProfile() {
  std::cout << "\nProfile input:\n";
  std::cout << "1) Enter manually\n";
  std::cout << "2) Load sample profile (data/sample_profile.json)\n";
  const int c = readIntInRange("Choose: ", 1, 2);

  if (c == 1) readProfileFromUser();
  else loadProfileFromSample();

  std::cout << "\nProfile ready:\n" << profile_.toString() << "\n";
  hasProfile_ = true;
}

Gender MenuSystem::parseGender(const std::string& s) {
  const std::string v = toLower(s);
  if (v == "female" || v == "f") return Gender::Female;
  if (v == "male" || v == "m") return Gender::Male;
  if (v == "other" || v == "o") return Gender::Other;
  return Gender::Other;
}

Goal MenuSystem::parseGoal(const std::string& s) {
  const std::string v = toLower(s);
  if (v == "fat_loss" || v == "fat loss" || v == "loss" || v == "cut") return Goal::FatLoss;
  if (v == "muscle_gain" || v == "muscle gain" || v == "gain") return Goal::MuscleGain;
  if (v == "maintenance" || v == "maintain" || v == "maint") return Goal::Maintenance;
  return Goal::Maintenance;
}

DietPreference MenuSystem::parseDiet(const std::string& s) {
  const std::string v = toLower(s);
  if (v == "vegan") return DietPreference::Vegan;
  if (v == "vegetarian" || v == "veg") return DietPreference::Vegetarian;
  return DietPreference::Omnivore;
}

void MenuSystem::readProfileFromUser() {
  profile_.name = readNonEmptyLine("Name: ");
  profile_.age = readIntInRange("Age (10..100): ", 10, 100);

  while (true) {
    std::string g = readNonEmptyLine("Gender (female/male/other): ");
    profile_.gender = parseGender(g);
    const std::string v = toLower(g);
    if (v == "female" || v == "male" || v == "other" || v == "f" || v == "m" || v == "o") break;
    std::cout << "Invalid gender. Try again.\n";
  }

  profile_.heightCm = readDoubleInRange("Height in cm (120..220): ", 120.0, 220.0);
  profile_.weightKg = readDoubleInRange("Weight in kg (30..300): ", 30.0, 300.0);

  while (true) {
    std::string gl = readNonEmptyLine("Goal (fat_loss/muscle_gain/maintenance): ");
    profile_.goal = parseGoal(gl);
    const std::string v = toLower(gl);
    if (v == "fat_loss" || v == "fat loss" || v == "muscle_gain" || v == "muscle gain" || v == "maintenance") break;
    std::cout << "Invalid goal. Try again.\n";
  }

  profile_.workoutFrequencyPerWeek = readIntInRange("Workout frequency per week (1..14): ", 1, 14);
  profile_.activityLevel = readIntInRange(
    "Activity level (1 sedentary, 2 light, 3 moderate, 4 active, 5 very active): ", 1, 5);
  profile_.sleepHoursPerNight = readDoubleInRange("Sleep hours per night (4..10): ", 4.0, 10.0);

  while (true) {
    std::string d = readNonEmptyLine("Diet preference (omnivore/vegetarian/vegan): ");
    const std::string v = toLower(d);
    if (v == "omnivore" || v == "vegetarian" || v == "vegan") {
      profile_.dietPreference = parseDiet(v);
      break;
    }
    std::cout << "Invalid diet preference. Try again.\n";
  }
}

void MenuSystem::loadProfileFromSample() {
  const std::string path = "data/sample_profile.json";
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("Sample profile file not found: " + path);
  }

  std::ifstream ifs(path, std::ios::binary);
  if (!ifs) throw std::runtime_error("Failed to open: " + path);

  std::string content;
  ifs.seekg(0, std::ios::end);
  auto size = ifs.tellg();
  if (size > 0) {
    content.resize(static_cast<size_t>(size));
    ifs.seekg(0, std::ios::beg);
    ifs.read(content.data(), static_cast<std::streamsize>(content.size()));
  }

  auto extractString = [&](const std::string& key) -> std::string {
    const std::string needle = "\"" + key + "\"";
    const auto pos = content.find(needle);
    if (pos == std::string::npos) return {};

    const auto colon = content.find(':', pos);
    if (colon == std::string::npos) return {};

    const auto q1 = content.find('"', colon);
    if (q1 == std::string::npos) return {};

    const auto q2 = content.find('"', q1 + 1);
    if (q2 == std::string::npos) return {};

    return content.substr(q1 + 1, q2 - (q1 + 1));
  };

  auto extractNumber = [&](const std::string& key) -> double {
    const std::string needle = "\"" + key + "\"";
    const auto pos = content.find(needle);
    if (pos == std::string::npos) return 0.0;

    const auto colon = content.find(':', pos);
    if (colon == std::string::npos) return 0.0;

    size_t start = colon + 1;
    while (start < content.size() && std::isspace(static_cast<unsigned char>(content[start]))) ++start;

    size_t end = start;
    while (end < content.size() && (std::isdigit(static_cast<unsigned char>(content[end])) ||
                                    content[end] == '.' || content[end] == '-' || content[end] == '+')) {
      ++end;
    }

    const std::string token = content.substr(start, end - start);
    try {
      return std::stod(token);
    } catch (...) {
      return 0.0;
    }
  };

  profile_.name = extractString("name");
  profile_.age = static_cast<int>(extractNumber("age"));
  profile_.heightCm = extractNumber("heightCm");
  profile_.weightKg = extractNumber("weightKg");
  profile_.workoutFrequencyPerWeek = static_cast<int>(extractNumber("workoutFrequencyPerWeek"));
  profile_.activityLevel = static_cast<int>(extractNumber("activityLevel"));
  profile_.sleepHoursPerNight = extractNumber("sleepHoursPerNight");

  const std::string genderS = extractString("gender");
  const std::string goalS = extractString("goal");
  const std::string dietS = extractString("dietPreference");

  profile_.gender = parseGender(genderS);
  profile_.goal = parseGoal(goalS);
  profile_.dietPreference = parseDiet(dietS);

  if (profile_.name.empty() || profile_.age <= 0 || profile_.heightCm <= 0.0 || profile_.weightKg <= 0.0) {
    throw std::runtime_error("Sample profile JSON is missing required fields or is malformed.");
  }
}

void MenuSystem::handleRecommendations() {
  if (!hasProfile_) {
    std::cout << "Create/load a profile first.\n";
    return;
  }

  const RecommendationBundle b = engine_.generate(profile_);

  std::cout << "\n===== Smart Recommendations =====\n";
  std::cout << "BMI: " << b.macros.bmi << "\n";
  std::cout << "Daily Calories: " << static_cast<int>(std::round(b.macros.dailyCalories)) << " kcal\n\n";

  std::cout << "Macros (g/day):\n";
  std::cout << "  Protein: " << static_cast<int>(std::round(b.macros.proteinG)) << " g\n";
  std::cout << "  Carbs:   " << static_cast<int>(std::round(b.macros.carbsG)) << " g\n";
  std::cout << "  Fats:    " << static_cast<int>(std::round(b.macros.fatsG)) << " g\n\n";

  std::cout << "Workout Split Recommendation:\n" << b.workoutSplit << "\n\n";
  std::cout << "Recovery Advice:\n" << b.recoveryAdvice << "\n\n";
  std::cout << "Hydration Advice:\n" << b.hydrationAdvice << "\n\n";
  std::cout << "Sleep Advice:\n" << b.sleepAdvice << "\n\n";
  std::cout << "Diet Advice:\n" << b.dietAdvice << "\n";

  std::cout << "=================================\n";
}

void MenuSystem::handleAddWeeklyProgress() {
  if (!hasProfile_) {
    std::cout << "Create/load a profile first.\n";
    return;
  }

  WeeklyProgressEntry e;
  e.weightKg = readDoubleInRange("This week weight (kg): ", 25.0, 400.0);
  e.workoutConsistencyDays = readIntInRange("Workout consistency days (0..7): ", 0, 7);

  const RecommendationBundle b = engine_.generate(profile_);
  const double defaultCal = b.macros.dailyCalories;

  std::cout << "Avg calories: press Enter to use recommended (" << static_cast<int>(std::round(defaultCal))
            << ")\n";
  std::cout << "If you type a value, it will override: ";

  std::string line;
  std::getline(std::cin, line);

  if (line.empty()) {
    e.avgCalories = defaultCal;
  } else {
    try {
      e.avgCalories = std::stod(line);
      e.avgCalories = std::clamp(e.avgCalories, 1000.0, 6000.0);
    } catch (...) {
      std::cout << "Invalid entry, using recommended calories.\n";
      e.avgCalories = defaultCal;
    }
  }

  tracker_.addEntry(profile_, e);
  std::cout << "Saved weekly progress to data/progress.json\n";
}

void MenuSystem::handleViewProgress() {
  const auto entries = tracker_.load();

  if (entries.empty()) {
    std::cout << "No progress history yet (data/progress.json missing or empty).\n";
    return;
  }

  std::cout << "\n===== Progress History =====\n";
  for (size_t i = 0; i < entries.size(); ++i) {
    const auto& e = entries[i];
    std::cout << "#" << (i + 1) << " " << e.weekLabel << "\n";
    std::cout << "  Weight: " << e.weightKg << " kg\n";
    std::cout << "  Workout consistency: " << e.workoutConsistencyDays << "/7 days\n";
    std::cout << "  Avg calories: " << static_cast<int>(std::round(e.avgCalories)) << " kcal\n";
  }
  std::cout << "============================\n";
}

} // namespace fitmind

