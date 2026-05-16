#include "utils/StringUtils.h"

#include <algorithm>
#include <cctype>

namespace fitmind {

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

bool equalsIgnoreCase(const std::string& a, const std::string& b) {
  return toLower(a) == toLower(b);
}

} // namespace fitmind

