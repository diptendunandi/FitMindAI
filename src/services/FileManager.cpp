#include "services/FileManager.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace fitmind {

bool FileManager::fileExists(const std::string& path) const {
  std::error_code ec;
  return fs::exists(path, ec);
}

void FileManager::ensureParentDir(const std::string& path) {
  fs::path p(path);
  auto parent = p.parent_path();
  if (parent.empty()) return;
  std::error_code ec;
  fs::create_directories(parent, ec);
}

std::string FileManager::readTextFile(const std::string& path) const {
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs) return {};
  std::string content;
  ifs.seekg(0, std::ios::end);
  std::streampos size = ifs.tellg();
  if (size > 0) {
    content.resize(static_cast<size_t>(size));
    ifs.seekg(0, std::ios::beg);
    ifs.read(&content[0], static_cast<std::streamsize>(content.size()));
  }
  return content;
}

void FileManager::writeTextFile(const std::string& path, const std::string& content) const {
  ensureParentDir(path);
  std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
  if (!ofs) {
    throw std::runtime_error("Failed to open output file: " + path);
  }
  ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
}

} // namespace fitmind

