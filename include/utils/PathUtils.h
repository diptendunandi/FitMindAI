 #pragma once

#include <string>

namespace fitmind {

// Resolve a relative data path like "data/progress.json" against the executable
// directory so the app works no matter where it’s launched from.
[[nodiscard]] std::string resolvePathFromExecutableDir(const std::string& path);

} // namespace fitmind

