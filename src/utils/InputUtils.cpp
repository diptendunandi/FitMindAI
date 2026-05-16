#include "utils/InputUtils.h"

#include <iostream>
#include <limits>
#include <stdexcept>

namespace fitmind {

static void clearBadInput() {
  std::cin.clear();
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int readIntInRange(const std::string& prompt, int minInclusive, int maxInclusive) {
  while (true) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
      throw std::runtime_error("Input stream closed.");
    }

    try {
      size_t idx = 0;
      int value = std::stoi(line, &idx);
      if (idx != line.size()) {
        throw std::invalid_argument("Trailing characters.");
      }
      if (value < minInclusive || value > maxInclusive) {
        std::cout << "Please enter a value between " << minInclusive << " and " << maxInclusive << ".\n";
        continue;
      }
      return value;
    } catch (const std::exception&) {
      std::cout << "Invalid integer. Try again.\n";
    }
  }
}

double readDoubleInRange(const std::string& prompt, double minInclusive, double maxInclusive) {
  while (true) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
      throw std::runtime_error("Input stream closed.");
    }

    try {
      size_t idx = 0;
      double value = std::stod(line, &idx);
      if (idx != line.size()) {
        throw std::invalid_argument("Trailing characters.");
      }
      if (value < minInclusive || value > maxInclusive) {
        std::cout << "Please enter a value between " << minInclusive << " and " << maxInclusive << ".\n";
        continue;
      }
      return value;
    } catch (const std::exception&) {
      std::cout << "Invalid number. Try again.\n";
    }
  }
}

std::string readNonEmptyLine(const std::string& prompt) {
  while (true) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
      throw std::runtime_error("Input stream closed.");
    }
    if (line.empty()) {
      std::cout << "Value cannot be empty. Try again.\n";
      continue;
    }
    return line;
  }
}

} // namespace fitmind

