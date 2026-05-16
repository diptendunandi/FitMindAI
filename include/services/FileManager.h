#pragma once

#include <string>
#include <vector>

namespace fitmind {

class FileManager {
public:
  [[nodiscard]] std::string readTextFile(const std::string& path) const;
  [[nodiscard]] bool fileExists(const std::string& path) const;
  void writeTextFile(const std::string& path, const std::string& content) const;

private:
  static void ensureParentDir(const std::string& path);
};

} // namespace fitmind

