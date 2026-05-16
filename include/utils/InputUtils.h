#pragma once

#include <string>
#include <optional>

namespace fitmind {

// Thread-safe-ish and deterministic parsing helpers.
// All functions throw std::invalid_argument on invalid input.

int readIntInRange(const std::string& prompt, int minInclusive, int maxInclusive);

double readDoubleInRange(const std::string& prompt, double minInclusive, double maxInclusive);

std::string readNonEmptyLine(const std::string& prompt);

} // namespace fitmind

