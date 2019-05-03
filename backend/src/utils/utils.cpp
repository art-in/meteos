#include <cpprest/http_listener.h>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using utility::conversions::to_string_t;
using utility::conversions::to_utf8string;
using web::http::http_request;

void freeze_thread_until_cin_closed() {
  std::string s;
  std::getline(std::cin, s);
}

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

std::string query(http_request req, std::string key) {
  auto k = to_string_t(key);
  return to_utf8string(web::uri::split_query(req.relative_uri().query())[k]);
}