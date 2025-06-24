/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/
#ifndef __MODERN_STOREABLE_H
#define __MODERN_STOREABLE_H

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string_view>

/**
 * Modern interface for objects that can be stored to and restored from files.
 * Uses C++17 features including std::filesystem, RAII, and std::optional.
 */
class ModernStoreable {
public:
  virtual ~ModernStoreable() = default;

  /** Modern store interface using streams */
  virtual bool store(std::ostream& stream) const = 0;

  /** Modern restore interface using streams */
  virtual bool restore(std::istream& stream) = 0;

  /** Store to file using std::filesystem
   * @param filepath Path to the file (supports std::string_view)
   * @param binary Whether to open in binary mode (default: true)
   * @return true on success, false on failure
   */
  bool storeToFile(std::string_view filepath, bool binary = true) const {
    namespace fs = std::filesystem;
    try {
      fs::path path(filepath);

      // Create parent directory if it doesn't exist
      if (auto parent = path.parent_path(); !parent.empty()) {
        fs::create_directories(parent);
      }

      auto mode = std::ios::out | (binary ? std::ios::binary : std::ios::openmode{});
      std::ofstream file(path, mode);

      if (!file) {
        return false;
      }

      return store(file);
    } catch (const fs::filesystem_error&) {
      return false;
    }
  }

  /** Restore from file using std::filesystem
   * @param filepath Path to the file (supports std::string_view)
   * @param binary Whether to open in binary mode (default: true)
   * @return true on success, false on failure
   */
  bool restoreFromFile(std::string_view filepath, bool binary = true) {
    namespace fs = std::filesystem;
    try {
      fs::path path(filepath);

      // Check if file exists
      if (!fs::exists(path)) {
        return false;
      }

      auto mode = std::ios::in | (binary ? std::ios::binary : std::ios::openmode{});
      std::ifstream file(path, mode);

      if (!file) {
        return false;
      }

      return restore(file);
    } catch (const fs::filesystem_error&) {
      return false;
    }
  }

  /** Check if a file exists using std::filesystem */
  static bool fileExists(std::string_view filepath) {
    namespace fs = std::filesystem;
    try {
      return fs::exists(fs::path(filepath));
    } catch (const fs::filesystem_error&) {
      return false;
    }
  }

  /** Get file size using std::filesystem */
  static std::optional<std::uintmax_t> getFileSize(std::string_view filepath) {
    namespace fs = std::filesystem;
    try {
      fs::path path(filepath);
      if (fs::exists(path)) {
        return fs::file_size(path);
      }
    } catch (const fs::filesystem_error&) {
      // Fall through
    }
    return std::nullopt;
  }

  /** Get last modification time using std::filesystem */
  static std::optional<std::filesystem::file_time_type> getLastModified(std::string_view filepath) {
    namespace fs = std::filesystem;
    try {
      fs::path path(filepath);
      if (fs::exists(path)) {
        return fs::last_write_time(path);
      }
    } catch (const fs::filesystem_error&) {
      // Fall through
    }
    return std::nullopt;
  }
};

// RAII wrapper for FILE* (for legacy code compatibility)
class FileWrapper {
public:
  explicit FileWrapper(FILE* file)
    : file_(file) {}

  FileWrapper(std::string_view filepath, std::string_view mode) {
    file_ = std::fopen(std::string(filepath).c_str(), std::string(mode).c_str());
  }

  ~FileWrapper() {
    if (file_) {
      std::fclose(file_);
    }
  }

  // Delete copy operations
  FileWrapper(const FileWrapper&) = delete;
  FileWrapper& operator=(const FileWrapper&) = delete;

  // Allow move operations
  FileWrapper(FileWrapper&& other) noexcept
    : file_(other.file_) {
    other.file_ = nullptr;
  }

  FileWrapper& operator=(FileWrapper&& other) noexcept {
    if (this != &other) {
      if (file_) {
        std::fclose(file_);
      }
      file_ = other.file_;
      other.file_ = nullptr;
    }
    return *this;
  }

  FILE* get() const {
    return file_;
  }

  explicit operator bool() const {
    return file_ != nullptr;
  }

  FILE* release() {
    FILE* temp = file_;
    file_ = nullptr;
    return temp;
  }

private:
  FILE* file_ = nullptr;
};

#endif // __MODERN_STOREABLE_H