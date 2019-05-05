#include <fstream>
#include <memory>
#include <string>

#include "file-logger.h"
#include "utils.h"

FileLogger::FileLogger(std::string file_path)
    : lock{new std::mutex},
      file{new std::ofstream(file_path.c_str(),
                             std::ios::out | std::ios::app)} {}

void FileLogger::log(std::string record) { log(LogLevel::INFO, record); }

void FileLogger::log(LogLevel level, std::string record) {
  lock->lock();

  (*file) << get_now_datetime_iso() << " - " << get_level_str(level) << " - "
          << record << std::endl;

  file->flush();

  lock->unlock();
}

std::string FileLogger::get_level_str(LogLevel level) {
  switch (level) {
    case LogLevel::INFO:
      return "INFO ";
    case LogLevel::WARN:
      return "WARN ";
    case LogLevel::ERROR:
      return "ERROR";
    default:
      throw std::runtime_error("Unknown log level");
  }
}