#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

enum class LogLevel { INFO, WARN, ERROR };

class FileLogger {
 public:
  FileLogger(std::string file_path);

  void log(std::string);
  void log(LogLevel level, std::string);

 private:
  std::shared_ptr<std::ofstream> file;
  std::shared_ptr<std::mutex> lock;

  std::string get_level_str(LogLevel);
};