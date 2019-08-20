#include <cpprest/http_listener.h>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "file-logger.h"
#include "sample.h"
#include "utils.h"

using utility::conversions::to_string_t;
using utility::conversions::to_utf8string;
using web::http::http_request;

void log_service_start(FileLogger logger, web::uri url, std::string db_path,
                       std::string log_path) {
  logger.log("Service started. DB file: " + db_path + "; Log file: " +
             log_path + "; Listening at: " + to_utf8string(url.to_string()));
}

void log_service_stop(FileLogger logger) { logger.log("Service stopped."); }

void log_service_start_failed(FileLogger logger, std::exception& e) {
  logger.log(LogLevel::ERROR,
             std::string{"Failed to start service: "} + e.what());
}

// Freezes main thread until cin is closed.
// Other solutions does not work:
// - while(1), promise/wait - main function is not finished, resource classes
//                            not cleaned up, so network port is not released.
// - signal, sigaction - do not catch termination signals, so unable to cleanup
//                        resource classes.
void freeze_thread_until_cin_closed() {
  std::string s;
  std::getline(std::cin, s);
}

// Returns current time in ISO8601 format and UTC Zulu timezone.
std::string get_now_datetime_iso() {
  using namespace std::chrono;

  // get current time
  auto now = system_clock::now();

  // get number of milliseconds for the current second
  // (remainder after division into seconds)
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  // convert to std::time_t in order to convert to std::tm (broken time)
  auto timer = system_clock::to_time_t(now);

  // convert to broken time
  std::tm bt = *std::localtime(&timer);

  std::ostringstream oss;

  oss << std::put_time(&bt, "%Y-%m-%dT%H:%M:%S");  // HH:MM:SS
  oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';

  return oss.str();
}

std::string ms_since(std::chrono::time_point<std::chrono::steady_clock> start) {
  using namespace std::chrono;

  auto now = steady_clock::now();
  auto ms = duration_cast<milliseconds>(now - start).count();

  return std::to_string(ms) + "ms";
}

std::string query(http_request req, std::string key) {
  auto k = to_string_t(key);
  auto query = web::uri::decode(req.relative_uri().query());
  auto value = web::uri::split_query(query)[k];
  return to_utf8string(value);
}

int query(web::http::http_request req, std::string key, int default_val) {
  std::string str = query(req, key);
  return str == "" ? default_val : std::stoi(str);
}

// Visits N items from target array.
// - ensures visited items are evenly distributed along the target
// - ensures first and last items visited
// - ensures sequence of visited items preserved
template <class T>
void distribute_evenly(int n, std::vector<T> target,
                       std::function<void(T&)> visit) {
  if (target.empty()) {
    return;
  }

  if (n <= 0 || n >= target.size()) {
    for (auto item : target) {
      visit(item);
    }
    return;
  }

  if (n == 1) {
    visit(target[target.size() - 1]);
    return;
  }

  if (n == 2) {
    visit(target[0]);
    visit(target[target.size() - 1]);
    return;
  }

  if (n >= 3) {
    visit(target[0]);

    int divider = std::ceil(static_cast<float>(target.size() - 1) / (n - 1));

    int visited = 1;
    for (int i = 1; i < target.size() - 1; i++) {
      if ((i % divider == 0) || ((target.size() - i) == (n - visited))) {
        visit(target[i]);
        visited++;
      }
    }

    visit(target[target.size() - 1]);
  }
}

template void distribute_evenly<Sample>(int n, std::vector<Sample> target,
                                        std::function<void(Sample&)> visit);